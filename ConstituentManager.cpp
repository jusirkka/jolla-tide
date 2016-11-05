#include "ConstituentManager.h"
#include "Database.h"
#include <QDebug>

using namespace Tide;


ConstituentManager::ConstituentManager(int station_id):
    m_Station(station_id),
    m_Patches(station_id)
{
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select p.id, pe.epoch_id from patches p "
                        "join patchepochs pe on p.id=pe.patch_id "
                        "where p.station_id=?", vars);
    QMap<db_int_t, Patch::Epochs> epochs;
    foreach (QVector<QVariant> row, r) {
        int patch_id = row[0].toInt();
        int epoch_id = row[1].toInt();
        epochs[patch_id].append(epoch_id);
    }

    r = Database::Query("select id, start, timedelta, patchsize from patches where station_id=? order by start", vars);

    m_PatchIds.append(-1); // removed in next()
    foreach (QVector<QVariant> row, r) {
        int patch_id = row[0].toInt();
        m_PatchIds.append(patch_id);
        Timestamp start = Timestamp::fromPosixTime(row[1].toInt());
        Interval dt = Interval::fromSeconds(row[2].toInt());
        db_int_t size = row[3].toInt();
        m_PatchData[patch_id] = Patch(start, dt, size, epochs[patch_id]);
    }
}

bool ConstituentManager::next() {
    if (!m_PatchIds.isEmpty()) m_PatchIds.removeFirst();

    if (!m_Patches.nextPatch()) {
        deleteConstituents();
        return false;
    }
    if (m_PatchIds.isEmpty()) {
        createConstituent();
    }
    Patch p1 = m_Patches.data();
    Patch p2 = m_PatchData[m_PatchIds.first()];
    if (p1 != p2) {
        deleteConstituents();
        createConstituent();
    }
    return true;
}

void ConstituentManager::deleteConstituents() {
    if (m_PatchIds.isEmpty()) return;

    QVariantList vars;
    Database::Transaction();
    foreach (int patch_id, m_PatchIds){
        vars.clear();
        vars << QVariant::fromValue(patch_id);
        Database::Control("delete from constituents where patch_id=?", vars);
        Database::Control("delete from patchepochs where patch_id=?", vars);
        Database::Control("delete from patches where id=?", vars);
    }
    Database::Commit();
    m_PatchIds.clear();
}

void ConstituentManager::createConstituent() {

    Patch p = m_Patches.data();
    qDebug() << "create patch" << p.epochs();
    QVariantList vars;
    vars << m_Station << p.start().posix() << p.step().seconds << p.size();
    Database::Control("insert into  patches (station_id, start, timedelta, patchsize) values (?, ?, ?, ?)", vars);
    vars.clear();
    vars << m_Station << p.start().posix();
    QList<QVector<QVariant>> r;
    r = Database::Query("select id from patches where station_id=? and start=?", vars);
    Q_ASSERT(r.size() == 1);

    int patch_id = r.first().at(0).toInt();
    m_PatchIds.append(patch_id);
    m_PatchData[patch_id] = p;

    Database::Transaction();
    foreach (int epoch_id, p.epochs()) {
        vars.clear();
        vars << epoch_id << patch_id;
        Database::Control("insert into patchepochs (epoch_id, patch_id) values (?, ?)", vars);
    }
    Database::Commit();



    r = Database::Query("select id, omega from modes");
    QMap<Speed, int> modes;
    foreach (QVector<QVariant> row, r) {
        modes[Speed::fromRadiansPerSecond(row[1].toDouble())] = row[0].toInt();
    }

    QMap<int, Complex> constituents;
    quint64 t = 0;
    while (m_Patches.next()) {
        QMapIterator<Speed, int> it(modes);
        while (it.hasNext()) {
            it.next();
            double w = it.key().radiansPerSecond;
            constituents[it.value()] += m_Patches.reading() * exp(Complex(0, - w * t));
        }
        t += p.step().seconds;
    }
    qDebug() << "consistency:" << t / p.step().seconds  << p.size();

    Database::Transaction();
    QMapIterator<int, Complex> it(constituents);
    while (it.hasNext()) {
        it.next();
        vars.clear();
        vars << patch_id << it.key() << it.value().x << it.value().y;
        Database::Control("insert into constituents (patch_id, mode_id, rea, ima) values (?, ?, ?, ?)", vars);
    }
    Database::Commit();
}

ConstituentManager::Coefficients ConstituentManager::constituents() {
    QVariantList vars;
    vars << m_PatchIds.first();
    QList<QVector<QVariant>> r = Database::Query("select m.omega, c.rea, c.ima from constituents c "
                                                 "join modes m on m.id=c.mode_id "
                                                 "where c.patch_id=?", vars);

    Coefficients c;
    foreach (QVector<QVariant> row, r) {
        Speed mode = Speed::fromRadiansPerSecond(row[0].toDouble());
        double x = row[1].toDouble();
        double y = row[2].toDouble();
        c[mode] = Complex(x, y);
    }

    return c;
}

Patch ConstituentManager::patch() const {
    return m_Patches.data().offsetCopy(m_Patches.epoch());
}

Timestamp ConstituentManager::epoch() const {
    return m_Patches.epoch();
}

