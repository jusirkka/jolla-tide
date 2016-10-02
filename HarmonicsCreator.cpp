#include <cmath>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QFile>
#include <QMapIterator>
#include <QStringList>

#include "Speed.h"
#include "Timestamp.h"
#include "Complex.h"
#include "HarmonicsCreator.h"
#include "Database.h"
#include "Amplitude.h"

using namespace Tide;

static const double daysPerJulianCentury_double    (36525.);
static const double hoursPerJulianCentury_double   (876600.);
static const double secondsPerJulianCentury_double (3155760000.);

static double centuries(const Timestamp& t) {
  static const Timestamp epoch = Timestamp::fromPosixTime(-2209032000ll); // 1899-12-31 12:00 GMT
  return (t - epoch).seconds / secondsPerJulianCentury_double;
}

class P3 {
public:
    P3(double t0, double t1, double t2, double t3);
    double v(double t) const;
    P3 d() const;
private:
    QVector<double> values;
};


P3::P3(double t0, double t1, double t2, double t3) {
    values.append(t0);
    values.append(t1);
    values.append(t2);
    values.append(t3);
}

double P3::v(double t) const {
    double sum = 0;
    double p = 1;
    foreach (double x, values) {
        sum += p * x;
        p *= t;
    }
    return sum;
}

P3 P3::d() const {
    return P3(values[1], 2 * values[2], 3 * values[3], 0);
}

class T {
public:
    T(const Timestamp& t);
    double speed(const QStringList& parts) const;
    double speed(int n1, int n2, int n3, int n4, int n5) const;
private:
    double v1, v2, v3, v4, v5;

};

T::T(const Timestamp& t) {
    double c = centuries(t);
    P3 m1(0, daysPerJulianCentury_double*360, 0, 0);
    v1 = m1.d().v(c);
    P3 m2(270 + 26./60 + 14.72/3600, 1336*360 + 1108411.2/3600, 9.09/3600, .0068/3600);
    v2 = m2.d().v(c);
    P3 m3(279 + 41./60 + 48.04/3600, 129602768.13/3600, 1.089/3600, 0);
    v3 = m3.d().v(c);
    P3 m4(334 + 19./60 + 40.87/3600, 11*360 + 392515.94/3600, -37.24/3600, -.045/3600);
    v4 = m4.d().v(c);
    P3 m5(281 + 13./60 + 15./3600, 6189.03/3600, 1.63/3600, .012/3600);
    v5 = m5.d().v(c);
}

double T::speed(const QStringList& parts) const {
    double k = 0;
    if (parts[0] == "M1") {
        // qDebug() << "Fixing M1";
        k = v4 / hoursPerJulianCentury_double;
    }
    return speed(parts[2].toInt(), parts[3].toInt(), parts[4].toInt(), parts[5].toInt(), parts[6].toInt()) + k;
}

double T::speed(int n1, int n2, int n3, int n4, int n5) const {
    return (v1*n1 + v2*n2 + v3*n3 + v4*n4 + v5*n5) / hoursPerJulianCentury_double;
}

class BC {
public:
    BC(int n1, int n2, int n3, int n4, int n5);
    Speed speed() const;
private:
    int x1, x2, x3, x4, x5;
    T t;
};

BC::BC(int n1, int n2, int n3, int n4, int n5): x1(n1), x2(n2), x3(n3), x4(n4), x5(n5), t(Timestamp::now()){}

Speed BC::speed() const {
    return Speed::fromDegreesPerHour(t.speed(x1, x2, x3, x4, x5));
}

static Speed parseConstituent(const QStringList& parts, const QList<BC>& base) {
    static T t(Timestamp::now());
    if (parts[1] == "Compound") {
        Speed sum = Speed::fromRadiansPerSecond(0);
        for (int i = 2; i < parts.size(); ++i) {
            int mul = parts[i].toInt();
            sum += mul * base[i-2].speed();
        }
        return sum;
    }
    return Speed::fromDegreesPerHour(t.speed(parts));
}

HarmonicsCreator::HarmonicsCreator():
    m_I(0, 1)
{
    QMap<Speed, QString> modes;
    QFile congen(":/congen_input.txt");
    QRegExp sep("\\s+");
    QStringList types;
    types << "Basic"  << "Doodson" << "Compound";
    QList<BC> base;
    // O1 K1 P1 M2 S2 N2 L2 K2 Q1 NU2 S1 M1-DUTCH LDA2

    base << BC(1,-2,1,0,0) << BC(1,0,1,0,0) << BC(1,0,-1,0,0) << BC(2,-2,2,0,0)
         << BC(2,0,0,0,0) << BC(2,-3,2,1,0) << BC(2,-1,2,-1,0) << BC(2,0,2,0,0)
         << BC(1,-3,1,1,0) << BC(2,-3,4,-1,0) << BC(1,0,0,0,0) << BC(1,-1,1,1,0)
         << BC(2,-1,0,1,0);

    congen.open(QIODevice::ReadOnly|QIODevice::Text);
    char buf[512];
    qint64 lineLength = congen.readLine(buf, sizeof(buf));
    while (lineLength > 0) {
        QString line(buf);
        lineLength = congen.readLine(buf, sizeof(buf));
        if (line.trimmed()[0] == '#') continue;
        QStringList parts = line.split(sep, QString::SkipEmptyParts);
        if (parts.length() < 2) continue;
        if (!types.contains(parts[1])) continue;
        Speed mode = parseConstituent(parts, base);
        if (modes.contains(mode)) {
            // qDebug() << parts[0] << "is a duplicate of" << modes[mode] << ", skipping";
            continue;
        }
        modes[mode] = parts[0];
    }

    m_W["Z0"] = 0;
    QMapIterator<Speed, QString> m(modes);
    while (m.hasNext()) {
        m.next();
        // qDebug() << m.value() << m.key().dph();
        m_W[m.value()] = m.key().radiansPerSecond;
    }

    checkDBIntegrity();
}

HarmonicsCreator* HarmonicsCreator::instance() {
    static HarmonicsCreator* hc = new HarmonicsCreator();
    return hc;
}

HarmonicsCreator::Coefficients HarmonicsCreator::patch(int step, const LevelData& levels) {
    Coefficients r;
    ModeSpeedIterator it(m_W);
    while (it.hasNext()) {
        it.next();
        QString m = it.key();
        double w = it.value();
        Complex sum = 0;
        for (int n = 0; n < levels.count(); n++) {
            sum += levels[n] * exp(-1 * m_I * w * step * n);
        }

        r[m] = sum;
    }

    return r;
}

void HarmonicsCreator::reset() {
    m_N = 0;
    m_D = 0;
    m_ModeSums.clear();
}


void HarmonicsCreator::append(int delta, int patchsize, int step, const Coefficients& patch) {
    m_N += patchsize;
    m_D += (patchsize - 1) * step;
    CoefficientsIterator it(patch);
    while (it.hasNext()) {
        it.next();
        double w = m_W[it.key()];
        m_ModeSums[it.key()] += it.value() * exp(-1 * m_I * delta * w);
    }
}

HarmonicsCreator::Coefficients HarmonicsCreator::average() const {

    Coefficients r;

    if (!m_N) return r;

    ModeSpeedIterator it(m_W);
    QMap<quint64, Complex> ksum;
    QMap<quint64, int> kcount;
    QMap<quint64, QString> kname;
    while (it.hasNext()) {
        it.next();
        double w = it.value();
        QString m = it.key();

        if (m == "Z0") {
            r[m] = m_ModeSums[m] / m_N;
            continue;
        }

        double kvalue = w / (2*M_PI) * m_D;
        if (kvalue < 2) {
            // qDebug() << "skipping " << m << ", k_c = " << kvalue << "Amplitude = " << a;
            continue;
        }

        quint64 slot(kvalue);
        ksum[slot] += m_ModeSums[m];
        kcount[slot] += 1;
        kname[slot] = m; // represents the average

    }

    QMapIterator<quint64, QString> k(kname);
    while (k.hasNext()) {
        k.next();
        QString m = k.value();
        // double w = Speed::fromRadiansPerSecond(m_W[m]).dph();
        double a = ksum[k.key()].mod() / kcount[k.key()] / m_N * 2;
        if (a < 0.02) {
            // qDebug() << "skipping " << m <<  w << ", Amplitude = " << a;
            continue;
        }
        r[m] = ksum[k.key()] / kcount[k.key()] / m_N * 2;
        // qDebug() << "Mode" << m << w << "Amplitude = " << r[m].mod() << "average of" << kcount[k.key()] << "modes";
    }

    return r;
}

void HarmonicsCreator::checkDBIntegrity() {
    QList<QVector<QVariant>> r;
    QVariantList vars;

    // Modes
    ModeSpeedIterator m(m_W);
    Database::Transaction();
    while (m.hasNext()) {
        m.next();
        vars.clear();
        vars << QVariant::fromValue(m.key());
        r = Database::Query("select id from modes where name=?", vars);
        if (r.isEmpty()) {
            vars.clear();
            vars << QVariant::fromValue(m.value()) << QVariant::fromValue(m.key());
            Database::Control("insert into modes (omega, name) values (?, ?)", vars);
        } else {
            vars.clear();
            vars << QVariant::fromValue(m.value()) << r.first()[0];
            Database::Control("update modes set omega=? where id=?", vars);
        }
    }
    Database::Commit();

    // Epochs
    QMap<qint64, QVector<qint64>> patch_first;
    QMap<qint64, QVector<qint64>> patch_last;
    QMap<qint64, QVector<qint64>> epochs;
    QMap<qint64, qint64> deltas;
    r = Database::Query("select id, station_id, start, timedelta, patchsize from epochs order by station_id, start");
    foreach (QVector<QVariant> row, r) {
        qint64 epoch_id = row[0].toInt();
        qint64 station_id = row[1].toInt();
        qint64 start = row[2].toInt();
        qint64 timedelta = row[3].toInt();
        qint64 patchsize = row[4].toInt();
        patch_first[station_id].append(start);
        patch_last[station_id].append(start + timedelta * (patchsize - 1));
        epochs[station_id].append(epoch_id);
        deltas[epoch_id] = timedelta;
    }

    QMapIterator<qint64, QVector<qint64>> st(epochs);
    while (st.hasNext()) {
        st.next();
        QVector<qint64> first_stamp = patch_first[st.key()];
        QVector<qint64> last_stamp = patch_last[st.key()];
        for (int idx = 1; idx < last_stamp.size(); idx++) {
            qint64 epoch_id = st.value()[idx - 1];
            if (first_stamp[idx] < last_stamp[idx - 1]) {
                qDebug() << "Overlap" << st.key() << epoch_id << idx << last_stamp[idx - 1] - first_stamp[idx];
                qint64 new_last = first_stamp[idx] - 5;
                qint64 delta = deltas[epoch_id];
                vars.clear();
                vars << QVariant::fromValue(epoch_id);
                Database::Control("delete from constituents where epoch_id=?", vars);
                r = Database::Query("select id from readings where epoch_id=? limit 1", vars);
                if (r.isEmpty()) continue;
                vars << QVariant::fromValue(r.first()[0].toInt() + (new_last - first_stamp[idx - 1]) / delta);
                Database::Control("delete from readings where epoch_id=? and id > ?", vars);
                vars.clear();
                vars << QVariant::fromValue(epoch_id);
                r = Database::Query("select count(*) from readings where epoch_id=?", vars);
                vars.clear();
                vars << r.first()[0] << QVariant::fromValue(epoch_id);
                Database::Control("update epochs set patchsize=? where id=?", vars);
            }
        }
    }
}

Tide::RunningSet* HarmonicsCreator::CreateConstituents(int station_id) {

    // Ensure that all patches have been computed
    UpdateDB(station_id);

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select id, start, timedelta, patchsize from epochs where station_id=? order by start", vars);
    if (r.isEmpty()) {
        return 0;
    }

    Timestamp epoch = Timestamp::fromPosixTime(r.first()[1].toInt());

    instance()->reset();


    foreach (QVector<QVariant> row, r) {
        int epoch_id = row[0].toInt();
        int delta = row[1].toInt() - epoch.posix();
        int step = row[2].toInt();
        int patchsize = row[3].toInt();
        Coefficients c;
        QList<QVector<QVariant>> r2;
        vars.clear();
        vars << QVariant::fromValue(epoch_id);
        r2 = Database::Query("select m.name, c.rea, c.ima from constituents c "
                             "join modes m on m.id=c.mode_id "
                             "where c.epoch_id=?", vars);
        foreach (QVector<QVariant> row2, r2) {
            QString name = row2[0].toString();
            double x = row2[1].toDouble();
            double y = row2[2].toDouble();
            c[name] = Complex(x, y);
        }
        instance()->append(delta, patchsize, step, c);
    }

    Coefficients coeffs = instance()->average();

    if (!coeffs.contains("Z0")) {
        return 0;
    }

    Amplitude datum = Amplitude::fromDottedMeters(coeffs["Z0"].x, 0);
    RunningSet* rset = new RunningSet(epoch, datum);

    ModeSpeed modes = instance()->modes();

    CoefficientsIterator m(coeffs);
    while (m.hasNext()) {
        m.next();
        if (m.key() == "Z0") continue;
        Speed w = Speed::fromRadiansPerSecond(modes[m.key()]);
        rset->append(m.value(), w);
    }

    return rset;
}

Tide::Timestamp HarmonicsCreator::LastDataPoint(int station_id) {

    QList<QVector<QVariant>> r;
    QVariantList vars;

    vars << QVariant::fromValue(station_id);
    r = Database::Query("select timedelta, patchsize, max(start) from epochs "
                        "where station_id=?", vars);
    if (r.isEmpty()) {
        return Timestamp::fromPosixTime(0);
    }

    Interval last_step = Interval::fromSeconds(r.first()[0].toInt());
    int last_number_of_points = r.first()[1].toInt();
    Timestamp last_stamp = Timestamp::fromPosixTime(r.first()[2].toInt());

    return last_stamp + last_step * (last_number_of_points - 1);
}

void HarmonicsCreator::UpdateDB(int station_id) {
    // Fetch patches that are already present
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select distinct e.id from epochs e join constituents c on c.epoch_id=e.id where e.station_id=?", vars);
    QVector<int> constituents;
    foreach (QVector<QVariant> row, r) {
        constituents.append(row[0].toInt());
    }
    // 1. read raw data
    r = Database::Query("select id, timedelta from epochs where station_id=?", vars);
    // qDebug() << r.size() << vars[0].toInt() << station_id;
    foreach (QVector<QVariant> row, r) {
        int epoch_id = row[0].toInt();
        if (constituents.contains(epoch_id)) continue;
        int step = row[1].toInt();
        vars.clear();
        vars << QVariant::fromValue(epoch_id);
        QList<QVector<QVariant>> raw = Database::Query("select reading from readings where epoch_id=?", vars);
        HarmonicsCreator::LevelData levels(raw.size());
        for (int i = 0; i < levels.size(); ++i) {
            levels[i] = raw[i][0].toDouble();
        }
        // 2. Read partial constituents
        Coefficients coeffs = instance()->patch(step, levels);
        // 5. Insert partial constituents
        CoefficientsIterator c(coeffs);
        Database::Transaction();
        while (c.hasNext()) {
            c.next();
            vars.clear();
            vars << QVariant::fromValue(c.key());
            r = Database::Query("select id from modes where name=?", vars);
            vars.clear();
            vars << QVariant::fromValue(epoch_id) << r[0][0] <<
                    QVariant::fromValue(c.value().x) << QVariant::fromValue(c.value().y);
            Database::Control("insert into constituents (epoch_id, mode_id, rea, ima) values (?, ?, ?, ?)", vars);
        }
        Database::Commit();

    }


}



