#include <cmath>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QFile>
#include <QMapIterator>
#include <QStringList>
#include <Eigen/Dense>

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
    m_I(0, 1),
    m_Data(0),
    m_AmplitudeCut(0.005), // meters
    m_SlowCut(0.2),
    m_ResolutionCut(0.9),
    m_AmplitudeDiffLowerCut(0.01),  // meters
    m_AmplitudeDiffUpperCut(1.0),  // meters
    m_MaxSampleSize(365*6*24)
{

    Database::Control("create table if not exists constituents ("
                      "id       integer primary key, "
                      "epoch_id integer not null, "
                      "mode_id  integer not null, "
                      "rea real not null, "
                      "ima real not null)");

    Database::Control("create table if not exists modes ("
                      "id    integer primary key, "
                      "name  text not null, "
                      "omega real not null)");


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
            // qDebug().noquote() << "skipping" << parts[0] << "~>" << modes[mode] << mode.dph();
            continue;
        }
        modes[mode] = parts[0];
    }

    m_KnownModes["Z0"] = Speed::fromRadiansPerSecond(0);
    m_KnownNames[m_KnownModes["Z0"]] = "Z0";
    QMapIterator<Speed, QString> m(modes);
    while (m.hasNext()) {
        m.next();
        // qDebug() << m.value() << m.key().dph();
        m_KnownModes[m.value()] = m.key();
        m_KnownNames[m.key()] = m.value();
    }

    checkDBIntegrity();
}

HarmonicsCreator* HarmonicsCreator::instance() {
    static HarmonicsCreator* hc = new HarmonicsCreator();
    return hc;
}


void HarmonicsCreator::config(const QString& key, const QVariant& value) {
    bool ok;
    qDebug().noquote() << "configuring " << key << "= " << value.toDouble();
    if (key.toLower() == "resolutioncut") {
        double v = value.toDouble(&ok);
        if (ok) {
            m_ResolutionCut = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    if (key.toLower() == "amplitudecut") {
        double v = value.toDouble(&ok);
        if (ok) {
            m_AmplitudeCut = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    if (key.toLower() == "slowcut") {
        double v = value.toDouble(&ok);
        if (ok) {
            m_SlowCut = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    if (key.toLower() == "amplitudedifflowercut") {
        double v = value.toDouble(&ok);
        if (ok) {
            m_AmplitudeDiffLowerCut = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    if (key.toLower() == "amplitudediffuppercut") {
        double v = value.toDouble(&ok);
        if (ok) {
            m_AmplitudeDiffUpperCut = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    if (key.toLower() == "maxsamplesize") {
        db_int_t v = value.toLongLong(&ok);
        if (ok) {
            m_MaxSampleSize = v;
        } else {
            qDebug().noquote() << key << ": invalid value";
        }
        return;
    }
    qDebug().noquote() << key << ": not found";
}


void HarmonicsCreator::reset(db_int_t station_id) {
    m_Averages.clear();

    delete m_Data;
    m_Data = new PatchIterator(station_id);

    if (!m_Data->lastPatch()) return;
    m_Patch = m_Data->data();


    Timestamp start = m_Patch.start();
    Coefficients sums;
    Speeds modes = m_KnownNames.keys().toVector();
    while (m_Data->next()) {
        double a = m_Data->reading();
        double t = (m_Data->stamp() - start).seconds;
        foreach (Speed q, modes) {
            double w = q.radiansPerSecond;
            sums[q] += a * exp(Complex(0, - w * t));
        }
    }


    CoefficientsIterator it(sums);
    while (it.hasNext()) {
        it.next();
        m_Averages[it.key()] = it.value() / m_Patch.size();
    }

}


void HarmonicsCreator::average(Coefficients& coeffs, Timestamp& epoch) {

    if (m_Averages.isEmpty()) return;

    coeffs = solve();

    Speed z = Speed::fromRadiansPerSecond(0);
    if (!coeffs.contains(z)) {
        coeffs.clear();
        return;
    }

    epoch = m_Patch.start();
}


HarmonicsCreator::Coefficients HarmonicsCreator::solve() {

    Speeds modes = selectModes();
    Coefficients coeffs;
    int maxLoops = 5;


    coeffs = fitModes(modes);
    qDebug() << "number of modes" << modes.length();
    qDebug() << "error estimate = " << errorEstimate(coeffs);
    bool zeros = true;
    int loopCount = 0;
    while (zeros && loopCount++ < maxLoops) {
        zeros = false;
        modes.clear();
        foreach (Speed q, coeffs.keys()) {
            if (coeffs[q].mod() < m_AmplitudeCut) {
                zeros = true;
                continue;
            }
            modes.append(q);
        }
        if (zeros) {
            coeffs = fitModes(modes);
            qDebug() << "number of modes" << modes.length();
            qDebug() << "error estimate = " << errorEstimate(coeffs);
        }
    }
    return coeffs;
}

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> M_T;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> V_T;

HarmonicsCreator::Coefficients HarmonicsCreator::fitModes(Speeds& selected) {
    Speed z = Speed::fromRadiansPerSecond(0);
    selected.removeAll(z);
    Coefficients coeffs;
    coeffs[z] = m_Averages[z];
    int rows = m_Patch.size();
    if (rows > m_MaxSampleSize) {
        rows = m_MaxSampleSize;
    }
    int firstReading = m_Patch.size() - rows;

    int cols = 2 * selected.size();
    double datum = coeffs[z].x;
    M_T A(rows, cols);
    V_T B(rows);
    m_Data->lastPatch();
    int row = 0;
    while (m_Data->next()) {
        if (row < firstReading) {
            row++;
            continue;
        }
        B(row - firstReading) = m_Data->reading() - datum;
        for (int col = 0; col < cols / 2; col++) {
            double x = selected[col].radiansPerSecond * m_Patch.step().seconds * row;
            A(row - firstReading, 2 * col) = std::cos(x);
            A(row - firstReading, 2 * col + 1) = - std::sin(x);
        }
        row++;
    }
    V_T X = A.fullPivHouseholderQr().solve(B);
    for (int col = 0; col < cols / 2; col++) {
        Speed q = selected[col];
        Complex coeff(X(2*col), X(2*col+1));
        coeffs[q] = coeff;
        qDebug().noquote() << "coeff" << m_KnownNames[q] << q.dph() << coeffs[q].mod();
    }

    return coeffs;
}

static bool Diag = true;


HarmonicsCreator::Speeds HarmonicsCreator::selectModes() {
    Speeds modes = m_KnownNames.keys().toVector();
    Speed z = Speed::fromRadiansPerSecond(0);

    Speeds pass_1;
    pass_1.append(z);
    foreach (Speed q, modes) {
        if (!m_Averages.contains(q) || m_Averages[q].mod() < m_AmplitudeCut / 2 || q == z) {
            if (m_Averages.contains(q) && q != z) {
                qDebug().noquote() << "Skipping minor mode" << m_KnownNames[q] << q.dph() << m_Averages[q].mod();
            }
            continue;
        }
        pass_1.append(q);
    }

    qDebug() << "number of modes after pass 1" << pass_1.length();


    Speeds pass_2;
    foreach (Speed q, pass_1) {
        Complex elem  = computeElement(z, q, Diag);
        if (elem.mod() > m_SlowCut) {
            if (q != z) {
                qDebug().noquote() << "Skipping long wave"  << m_KnownNames[q] << q.dph() << elem.mod();
            }
            continue;
        }
        pass_2.append(q);
    }

    qDebug() << "number of modes after pass 2" << pass_2.length();

    QVector<Speeds> grouping;
    grouping.append(Speeds());
    grouping.last().append(z);
    foreach (Speed q, pass_2) {
        Complex elem  = computeElement(grouping.last().last(), q, Diag);
        if (elem.mod() > m_ResolutionCut) {
            grouping.last().append(q);
        } else {
            grouping.append(Speeds());
            grouping.last().append(q);
        }
    }

    QVector<Speeds> filtered;
    foreach (Speeds group, grouping) {
        bool done = false;
        Speeds modes = group;
        while (!done && modes.length() > 1) {
            done = checkModes(modes);
        }
        filtered.append(modes);
    }

    qDebug() << "number of groups" << filtered.length();

    // Just debug logging in this loop
    foreach (Speeds group, filtered) {
        Speed q = group.first();
        Speed p = group.last();
        qDebug().noquote() << "group"  << m_KnownNames[q] << q.dph() << " -> " << m_KnownNames[p] << p.dph() << ", length" << group.length();
        QString avs = "";
        foreach (Speed w, group) {
            avs += QString("%1 ").arg(m_Averages[w].mod(), 6, 'f', 4);
            if (group.length() == 1) continue;
            QString corrs = "";
            foreach (Speed w2, group) {
                if (w2 > w) {
                    Complex r  = computeElement(w, w2, Diag);
                    Complex dB = (m_Averages[w2] - m_Averages[w]);
                    Complex B0 = (m_Averages[w2] + m_Averages[w]);
                    corrs += QString("%1 ").arg((dB/B0 + r.y * m_I).mod() / (1-r.x), 6, 'g', 4);
                }
            }
            if (corrs.isEmpty()) continue;
            qDebug().noquote() << corrs;
        }
        qDebug().noquote() << avs;
    }



    Speeds selected;

    foreach (Speeds group, filtered) {
        foreach (Speed q, group) {
            selected.append(q);
        }
    }


    return selected;
}

bool HarmonicsCreator::checkModes(Speeds& modes) {
    Speed q = modes.first();
    Speed p = modes.last();
    qDebug().noquote() << "checkModes"  << m_KnownNames[q] << q.dph() << " -> " << m_KnownNames[p] << p.dph() << ", length" << modes.length();
    foreach (Speed w, modes) {
        foreach (Speed w2, modes) {
            if (w2 > w) {
                Complex r  = computeElement(w, w2, Diag);
                Complex dB = (m_Averages[w2] - m_Averages[w]);
                Complex B0 = (m_Averages[w2] + m_Averages[w]);
                double corr = (dB/B0 + r.y * m_I).mod() / (1-r.x);
                if (corr < m_AmplitudeDiffLowerCut || corr > m_AmplitudeDiffUpperCut) {
                    if (m_Averages[w2].mod() < m_Averages[w].mod()) {
                        qDebug().noquote() << "removing"  << m_KnownNames[w2] << w2.dph() << m_Averages[w2].mod();
                        modes.removeOne(w2);
                    } else {
                        qDebug().noquote() << "removing"  << m_KnownNames[w] << w.dph() << m_Averages[w].mod();
                        modes.removeOne(w);
                    }
                    return false;
                }
            }
        }
    }
    return true;
}

double HarmonicsCreator::errorEstimate(const Coefficients& coeffs) {

    Speed z = Speed::fromRadiansPerSecond(0);

    if (!coeffs.contains(z)) {
        return ::nan("");
    }

    Amplitude datum = Amplitude::fromDottedMeters(coeffs[z].x, 0);
    RunningSet rset(m_Patch.start(), datum);

    CoefficientsIterator m(coeffs);
    while (m.hasNext()) {
        m.next();
        if (m.key() == z) continue;
        rset.append(m.value(), m.key());
    }

    double squareSum = 0;

    m_Data->lastPatch();
    while (m_Data->next()) {
        double d = m_Data->reading() - (rset.datum() + rset.tideDerivative(m_Data->stamp(), 0)).value;
        squareSum += d * d;
    }

    return ::sqrt(squareSum) / m_Patch.size();
}



void HarmonicsCreator::printMatrix(const ModeMatrix& D) {
    QString art("\n");

    QString triangle("");

    ModeMatrixIterator m(D);
    while (m.hasNext()) {
        m.next();
        art += triangle;
        triangle += '_';
        ModeVectorIterator n(m.value());
        while (n.hasNext()) {
            n.next();
            if (n.value().mod() < 0.01) {
                art += ".";
            } else {
                unsigned v = unsigned(10*n.value().mod());
                art += QString("%1").arg(v, 0, 16).toUpper();
            }
        }
        art += "\n";
    }
    qDebug().noquote() << art;
}



Complex HarmonicsCreator::computeElement(const Speed& q, const Speed& p, bool diag) const {
    double mul = diag ? -1 : 1;
    return coeff(q.radiansPerSecond + mul * p.radiansPerSecond);
}

void HarmonicsCreator::computeMatrix(ModeMatrix &m, bool diag) const {
    double mul = diag ? -1 : 1;
    QList<Speed> modes = m_KnownNames.keys();

    foreach (Speed q, modes) {
        foreach (Speed p, modes) {
            if (p < q) continue;
            m[q][p] = coeff(q.radiansPerSecond + mul * p.radiansPerSecond);
        }
    }
}

Complex HarmonicsCreator::coeff(double omega) const {
    double x = m_Patch.step().seconds * omega;
    double beta = double(m_Patch.offset().seconds) / m_Patch.step().seconds + 0.5 * (m_Patch.size() - 1);
    return exp(Complex(0, - beta * x)) * factor(x, m_Patch.size());
}

double HarmonicsCreator::factor(double x, unsigned n) const {
    if (x < 0) x = -x;
    double r = x / (2*M_PI);
    double eps = 0.001 / n;
    int lo = int(r);
    if (r - lo < eps) {
        if ((lo * (n - 1)) % 2 == 0) return 1;
        return -1;
    }
    int up = int(r+1);
    if (up - r < eps) {
        if ((up * (n - 1)) % 2 == 0) return 1;
        return -1;
    }
    return sin(0.5 * n * x) / sin(0.5 * x) / n;
}

void HarmonicsCreator::checkDBIntegrity() {
    QList<QVector<QVariant>> r;
    QVariantList vars;

    // Modes
    ModeSpeedIterator m(m_KnownModes);
    Database::Transaction();
    while (m.hasNext()) {
        m.next();
        vars.clear();
        vars << QVariant::fromValue(m.key());
        r = Database::Query("select id from modes where name=?", vars);
        if (r.isEmpty()) {
            vars.clear();
            vars << QVariant::fromValue(m.value().radiansPerSecond) << QVariant::fromValue(m.key());
            Database::Control("insert into modes (omega, name) values (?, ?)", vars);
        } else {
            vars.clear();
            vars << QVariant::fromValue(m.value().radiansPerSecond) << r.first()[0];
            Database::Control("update modes set omega=? where id=?", vars);
        }
    }
    Database::Commit();
}

void HarmonicsCreator::select(db_int_t station_id, Coefficients& coeffs, Timestamp& epoch) {
    QVariantList vars;
    vars << station_id;
    QList<QVector<QVariant>> r = Database::Query("select e.id, e.start, m.omega, c.rea, c.ima from constituents c "
                                                 "join modes m on m.id=c.mode_id join epochs e on e.id=c.epoch_id "
                                                 "where e.station_id=?", vars);

    // just in case there are several sets per station
    QMap<Timestamp, db_int_t> epochs;
    QMap<db_int_t, Coefficients> c;
    foreach (QVector<QVariant> row, r) {
        db_int_t epoch_id = row[0].toInt();
        Timestamp start = Timestamp::fromPosixTime(row[1].toInt());
        epochs[start] = epoch_id;
        Speed mode = Speed::fromRadiansPerSecond(row[2].toDouble());
        double x = row[3].toDouble();
        double y = row[4].toDouble();
        c[epoch_id][mode] = Complex(x, y);
    }
    if (!epochs.isEmpty()) {
        db_int_t last_epoch_id = epochs.last();
        epoch = epochs.key(last_epoch_id);
        coeffs = c[last_epoch_id];
    }
}

void HarmonicsCreator::insert(db_int_t station_id, const Coefficients& coeffs, const Timestamp& epoch) {
    QVariantList vars;
    vars << station_id << epoch.posix();
    QList<QVector<QVariant>> r = Database::Query("select id from epochs where station_id=? and start=?", vars);
    if (r.length() != 1) {
        qDebug() << "FATAL: unique epoch for" << station_id << "and" << epoch.print() << "not found, aborting insert";
        return;
    }
    db_int_t epoch_id = r.first().first().toInt();
    vars.clear();

    QMap<db_int_t, Complex> values;

    r = Database::Query("select id, name from modes");
    foreach (QVector<QVariant> row, r) {
        db_int_t mode_id = row[0].toLongLong();
        Speed q = m_KnownModes[row[1].toString()];
        if (coeffs.contains(q)) {
            values[mode_id] = coeffs[q];
        }
    }

    if (values.size() != coeffs.size()) {
        qDebug() << "FATAL: all modes not found, aborting insert";
        return;
    }

    Database::Transaction();
    QMapIterator<db_int_t, Complex> it(values);
    while (it.hasNext()) {
        it.next();
        vars.clear();
        vars << epoch_id << it.key() << it.value().x << it.value().y;
        Database::Control("insert into constituents (epoch_id, mode_id, rea, ima) values (?, ?, ?, ?)", vars);
    }
    Database::Commit();
}

Tide::RunningSet* HarmonicsCreator::CreateConstituents(int station_id) {


    Coefficients coeffs;
    Timestamp epoch;

    Speed z = Speed::fromRadiansPerSecond(0);

    instance()->select(station_id, coeffs, epoch);

    if (coeffs.isEmpty()) {
        instance()->reset(station_id);
        instance()->average(coeffs, epoch);
        if (coeffs.contains(z)) {
            instance()->insert(station_id, coeffs, epoch);
        }
    }

    if (!coeffs.contains(z)) {
        return 0;
    }

    Amplitude datum = Amplitude::fromDottedMeters(coeffs[z].x, 0);
    RunningSet* rset = new RunningSet(epoch, datum);

    CoefficientsIterator m(coeffs);
    while (m.hasNext()) {
        m.next();
        if (m.key() == z) continue;
        rset->append(m.value(), m.key());
    }

    return rset;
}

void HarmonicsCreator::Config(const QString& key, const QVariant& value) {
    instance()->config(key, value);
}

void HarmonicsCreator::Delete(db_int_t station_id) {
    QVariantList vars;
    vars << station_id;
    Database::Control("delete from constituents where epoch_id in (select id from epochs where station_id=?)", vars);
}

