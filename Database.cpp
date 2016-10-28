#include <QDir>
#include <QStandardPaths>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlError>
#include <QDebug>
#include <QtXml/QDomDocument>

#include "Database.h"

uint qHash(const Tide::Address& addr) {
    return qHash(addr.key());
}

using namespace Tide;

Database::Database() {

    m_DB = QSqlDatabase::addDatabase("QSQLITE");
    // ~/.local/share
    QString loc = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    loc = QString("%1/jolla-tide").arg(loc);
    if (!QDir(loc).exists()) {
        QDir().mkpath(loc);
    }
    QString dbfile = QString("%1/tides.db").arg(loc);
    m_DB.setDatabaseName(dbfile);
    m_DB.open();
    QSqlQuery query;
    query.exec("create table if not exists stations ("
               "id integer primary key autoincrement, "
               "fuid text not null, "
               "suid text not null, "
               "xmlinfo text not null)");
    query.exec("create table if not exists actives ("
               "id integer primary key autoincrement, "
               "station_id integer not null, "
               "mark text default 'notset', "
               "ordering integer)");
    m_DB.close();
}

Database* Database::instance() {
    static Database* db = new Database();
    return db;
}

QSqlQuery& Database::exec(const QString& sql) {
    if (!m_DB.isOpen()) m_DB.open();
    m_Query = QSqlQuery(m_DB);
    // qDebug() << sql;
    m_Query.exec(sql);
    if (m_Query.lastError().isValid()) qDebug() << m_Query.lastError();
    return m_Query;
}


QSqlQuery& Database::prepare(const QString& sql) {
    if (!m_DB.isOpen()) m_DB.open();
    m_Query = QSqlQuery(m_DB);
    // qDebug() << sql;
    m_Query.prepare(sql);
    if (m_Query.lastError().isValid()) qDebug() << m_Query.lastError();
    return m_Query;
}

void Database::close() {
    m_DB.commit();
    m_DB.close();
}

static void exec_and_trace(QSqlQuery& r) {
    r.exec();
    if (r.lastError().isValid()) qDebug() << r.lastError();
}

Database::ActiveList Database::ActiveStations() {
    QMap<int, Active> s; // ordering, (station, mark)
    QSqlQuery r = instance()->exec("select s.fuid, s.suid, a.mark, a.ordering from stations s join actives a on s.id=a.station_id");
    bool ok;
    int fail_ord = 1000; // some big number
    while (r.next()) {
        int ord = r.value(3).toInt(&ok);
        if (!ok) ord = fail_ord++;
        while (s.contains(ord)) ord = fail_ord++;
        s[ord] = Active(Address(r.value(0).toString(), r.value(1).toString()), r.value(2).toString());
    }
    return s.values();
}

QHash<Address, QString> Database::AllStations(const QString& provider) {
    QHash<Address, QString> s;
    QSqlQuery r;
    if (provider.isEmpty()) {
        r = instance()->exec("select fuid, suid, xmlinfo from stations");
        while (r.next()) {
            Address addr(r.value(0).toString(), r.value(1).toString());
            s[addr] = r.value(2).toString();
        }
    } else {
        r = instance()->prepare("select suid, xmlinfo from stations where fuid=?");
        r.bindValue(0, provider);
        exec_and_trace(r);
        while (r.next()) {
            Address addr(provider, r.value(0).toString());
            s[addr] = r.value(1).toString();
        }
    }
    instance()->close();
    return s;
}

void Database::OrderActives(const QList<Address>& ordering) {
    ActiveList actives = ActiveStations();
    QList<Address> stations;
    foreach (Active ac, actives) {
        stations.append(ac.address);
    }

    instance()->exec("delete from actives");

    Transaction();

    for (int ord = 0; ord < ordering.size(); ord++) {
        QString mark("notset");
        Address station = ordering[ord];
        if (stations.contains(station)) mark = actives[stations.indexOf(station)].mark;
        QSqlQuery r = instance()->prepare("select id from stations where fuid=? and suid=?");
        r.bindValue(0, station.factory);
        r.bindValue(1, station.station);
        exec_and_trace(r);
        if (!r.next()) {
            continue;
        }
        int station_id = r.value(0).toInt();
        r = instance()->prepare("insert into actives (station_id, mark, ordering) values (?, ?, ?)");
        r.bindValue(0, station_id);
        r.bindValue(1, mark);
        r.bindValue(2, ord);
        exec_and_trace(r);
    }

    Commit();
}

void Database::SetMark(const Address& addr, const QString& mark) {
    QSqlQuery r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, addr.factory);
    r.bindValue(1, addr.station);
    exec_and_trace(r);
    if (!r.next()) {
        return;
    }
    int station_id = r.value(0).toInt();
    r = instance()->prepare("update actives set mark=? where station_id=?");
    r.bindValue(0, mark);
    r.bindValue(1, station_id);
    exec_and_trace(r);
}


int Database::StationID(const Address& addr) {
    QSqlQuery r;
    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, addr.factory);
    r.bindValue(1, addr.station);
    exec_and_trace(r);
    if (!r.next()) {
        return 0;
    }
    return r.value(0).toInt();
}

QString Database::StationInfo(const Address& addr, const QString& attr) {
    QSqlQuery r;
    r = instance()->prepare("select xmlinfo from stations where fuid=? and suid=?");
    r.bindValue(0, addr.factory);
    r.bindValue(1, addr.station);
    exec_and_trace(r);
    if (!r.next()) {
        return QString();
    }
    QString errMsg;
    int erow;
    int ecol;
    QDomDocument doc(addr.station);
    doc.setContent(r.value(0).toString(), &errMsg, &erow, &ecol);
    if (!errMsg.isEmpty()) {
        qDebug() << errMsg << erow << ecol;
        return QString();
    }
    return doc.documentElement().attribute(attr);
}

void Database::UpdateStationInfo(const Address& addr, const QString& xmlinfo) {
    QSqlQuery r;

    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, addr.factory);
    r.bindValue(1, addr.station);
    exec_and_trace(r);
    if (r.next()) {
        int station_id = r.value(0).toInt();
        r = instance()->prepare("update stations set xmlinfo=? where id=?");
        r.bindValue(0, xmlinfo);
        r.bindValue(1, station_id);
    } else {
        r = instance()->prepare("insert into stations (fuid, suid, xmlinfo) values (?, ?, ?)");
        r.bindValue(0, addr.factory);
        r.bindValue(1, addr.station);
        r.bindValue(2, xmlinfo);
    }
    exec_and_trace(r);
}


void Database::Control(const QString& sql, const QVariantList& vars) {
    QSqlQuery r;
    if (vars.isEmpty()) {
        r = instance()->exec(sql);
    } else {
        r = instance()->prepare(sql);
        for (int i = 0; i < vars.size(); ++i) {
            r.bindValue(i, vars[i]);
        }
        exec_and_trace(r);
    }
}

QList<QVector<QVariant>> Database::Query(const QString& sql, const QVariantList& vars) {
    QSqlQuery r;
    if (vars.isEmpty()) {
        r = instance()->exec(sql);
    } else {
        r = instance()->prepare(sql);
        for (int i = 0; i < vars.size(); ++i) {
            r.bindValue(i, vars[i]);
        }
        exec_and_trace(r);
    }
    QList<QVector<QVariant>> s;
    while (r.next()) {
        QVector<QVariant> row(r.record().count());
        for (int i = 0; i < r.record().count(); ++i) {
            // qDebug() << r.value(i);
            row[i] = r.value(i);
        }
        s << row;
    }
    return s;
}

bool Database::Transaction() {
    if (!instance()->m_DB.isOpen()) instance()->m_DB.open();
    return instance()->m_DB.transaction();
}

bool Database::Commit() {
    return instance()->m_DB.commit();
}
