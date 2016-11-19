#include <cmath>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QVariantList>
#include <QFile>
#include <QMapIterator>
#include <QStringList>
//#include <Eigen/SparseCore>
//#include <Eigen/SparseLU>
#include <Eigen/Dense>

#include "Speed.h"
#include "Timestamp.h"
#include "Complex.h"
#include "HarmonicsCreator.h"
#include "Database.h"
#include "Amplitude.h"
#include "ConstituentManager.h"

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
    m_I(0, 1), m_Data(0)
{

    Database::Control("create table if not exists constituents ("
                      "id       integer primary key, "
                      "patch_id integer not null, "
                      "mode_id  integer not null, "
                      "rea real not null, "
                      "ima real not null)");

    Database::Control("create table if not exists patches ("
                      "id         integer primary key, "
                      "station_id integer not null, "
                      "start      integer not null, "
                      "timedelta  integer not null, "
                      "patchsize  integer not null)");

    Database::Control("create table if not exists patchepochs ("
                      "id       integer primary key, "
                      "epoch_id integer not null, "
                      "patch_id integer not null)");

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

void HarmonicsCreator::reset(db_int_t station_id) {
    m_Averages.clear();
    m_PatchData.clear();

    ConstituentManager mgr(station_id);
    Coefficients sums;
    while (mgr.next()) {
        Patch patch = mgr.patch();
        Coefficients coeffs = mgr.constituents();

        m_PatchData.append(patch);

        CoefficientsIterator it(coeffs);
        double delta = patch.offset().seconds;
        while (it.hasNext()) {
            it.next();
            if (!m_KnownNames.contains(it.key())) continue;
            double w = it.key().radiansPerSecond;
            sums[it.key()] += it.value() * exp(Complex(0, - delta * w));
        }
    }

    m_N = 0;

    for (int p = 0; p < m_PatchData.size(); p++) {
        m_N += m_PatchData[p].size();
    }

    CoefficientsIterator it(sums);
    while (it.hasNext()) {
        it.next();
        m_Averages[it.key()] = it.value() / m_N;
    }

    delete m_Data;
    m_Data = new PatchIterator(station_id);
}


RunningSet* HarmonicsCreator::average(double b0) {

    if (m_Averages.isEmpty()) return 0;

    Coefficients coeffs = solve(0.9, 0.0025, b0);
    qDebug() << "error estimate = " << errorEstimate(coeffs);

    Speed z = Speed::fromRadiansPerSecond(0);
    if (!coeffs.contains(z)) {
        return 0;
    }

    Amplitude datum = Amplitude::fromDottedMeters(coeffs[z].x, 0);
    RunningSet* rset = new RunningSet(m_Data->epoch(), datum);

    CoefficientsIterator m(coeffs);
    while (m.hasNext()) {
        m.next();
        if (m.key() == z) continue;
        rset->append(m.value(), m.key());
    }

    return rset;
}

//typedef Eigen::SparseMatrix<std::complex<double>> SpMat; // declares a column-major sparse matrix type of complex
//typedef Eigen::Triplet<std::complex<double>> A;

static bool Diag = true;
// static bool Off = false;

typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> MT;
typedef Eigen::Matrix<std::complex<double>, Eigen::Dynamic, 1> VT;

HarmonicsCreator::Coefficients HarmonicsCreator::solve(double cutCeil, double cutFloor, double b0) {

    QVector<QVector<Speed>> modes = selectModes(cutCeil, cutFloor, b0);
    Coefficients coeffs;
    Speed z = Speed::fromRadiansPerSecond(0);
    coeffs[z] = m_Averages[z];

    foreach (QVector<Speed> group, modes) {
        if (group.length() == 1) {
            Speed q = group.first();
            if (q != z) {
                coeffs[q] = 2 * m_Averages[q];
            }
            continue;
        }
        int n = group.length();
        MT A(n, n);
        VT B(n);
        for (int q = 0; q < n; q++) {
            Speed w1 = group[q];
            A(q, q) = 1;
            B(q) = m_Averages[w1].complex();
            for (int p = q + 1; p < n; p++) {
                Speed w2 = group[p];
                A(q, p) = computeElement(w1, w2, Diag).complex();
            }
        }
        A = Eigen::SelfAdjointView<Eigen::MatrixXcd, Eigen::Upper>(A);

        Eigen::VectorXcd X = A.colPivHouseholderQr().solve(B);
        qDebug() << "error estimate of inversion" << (A*X - B).norm() / B.norm();
        for (int q = 0; q < n; q++) {
            Speed w1 = group[q];
            coeffs[w1] = 2 * Complex(B(q));
        }
    }

    return coeffs;
}

QVector<QVector<Speed>> HarmonicsCreator::selectModes(double cutCeil, double cutFloor, double b0) {
    QVector<Speed> modes = m_KnownNames.keys().toVector();
    Speed z = Speed::fromRadiansPerSecond(0);

    QVector<Speed> pass_1;
    pass_1.append(z);
    foreach (Speed q, modes) {
        if (!m_Averages.contains(q) || m_Averages[q].mod() < cutFloor || q == z) {
            if (m_Averages.contains(q) && q != z) {
                qDebug().noquote() << "skipping" << m_KnownNames[q] << q.dph() << m_Averages[q].mod();
            }
            continue;
        }
        pass_1.append(q);
    }

    qDebug() << "number of mode after pass 1" << pass_1.length();


    QVector<Speed> pass_2;
    foreach (Speed q, pass_1) {
        Complex elem  = computeElement(z, q, Diag);
        if (elem.mod() > 0.1) {
            if (q != z) {
                qDebug().noquote() << "TODO: Long wave"  << m_KnownNames[q] << q.dph() << elem.mod();
            }
            continue;
        }
        pass_2.append(q);
    }

    qDebug() << "number of modes after pass 2" << pass_2.length();

    QVector<QVector<Speed>> grouping;
    grouping.append(QVector<Speed>());
    grouping.last().append(z);
    foreach (Speed q, pass_2) {
        Complex elem  = computeElement(grouping.last().last(), q, Diag);
        if (elem.mod() > cutCeil) {
            grouping.last().append(q);
        } else {
            grouping.append(QVector<Speed>());
            grouping.last().append(q);
        }
    }

    QVector<QVector<Speed>> filtered;
    foreach (QVector<Speed> group, grouping) {
        bool done = false;
        QVector<Speed> modes = group;
        while (!done && modes.length() > 1) {
            done = checkModes(modes, b0);
        }
        filtered.append(modes);
    }

    foreach (QVector<Speed> group, filtered) {
        Speed q = group.first();
        Speed p = group.last();
        qDebug().noquote() << "group"  << m_KnownNames[q] << q.dph() << " -> " << m_KnownNames[p] << p.dph() << ", length" << group.length();
        QString avs = "";
        foreach (Speed w, group) {
            avs += QString("%1 ").arg(m_Averages[w].mod(), 6, 'f', 4);
            if (group.length() == 1) continue;
            // QString corrs1 = "";
            QString corrs2 = "";
            foreach (Speed w2, group) {
                if (w2 > w) {
                    Complex r  = computeElement(w, w2, Diag);
                    Complex dB = (m_Averages[w2] - m_Averages[w]);
                    Complex B0 = (m_Averages[w2] + m_Averages[w]);
                    // corrs1 += QString("%1 ").arg(dB.mod() / (1-r.mod()), 6, 'g', 4);
                    corrs2 += QString("%1 ").arg((dB/B0 + r.y * m_I).mod() / (1-r.x), 6, 'g', 4);
                }
            }
            if (corrs2.isEmpty()) continue;
            // qDebug().noquote() << corrs1;
            qDebug().noquote() << corrs2;

        }
        qDebug().noquote() << avs;
    }

    qDebug() << "number of groups" << grouping.length();


    return filtered;
}

bool HarmonicsCreator::checkModes(QVector<Speed>& modes, double b0) {
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
                if (corr > b0) {
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
    RunningSet rset(m_Data->epoch(), datum);

    CoefficientsIterator m(coeffs);
    while (m.hasNext()) {
        m.next();
        if (m.key() == z) continue;
        rset.append(m.value(), m.key());
    }

    double squareSum = 0;

    m_Data->reset();
    while (m_Data->nextPatch()) {
        while (m_Data->next()) {
            double d = m_Data->reading() - (rset.datum() + rset.tideDerivative(m_Data->stamp(), 0)).value;
            squareSum += d * d;
        }
    }

    return ::sqrt(squareSum) / m_N;
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
    Complex sum(0, 0);
    foreach (Patch patch, m_PatchData) {
        sum += patch.size() / m_N * coeff(q.radiansPerSecond + mul * p.radiansPerSecond, patch);
    }
    return sum;
}

void HarmonicsCreator::computeMatrix(ModeMatrix &m, bool diag) const {
    double mul = diag ? -1 : 1;
    QList<Speed> modes = m_KnownNames.keys();

    foreach (Speed q, modes) {
        foreach (Speed p, modes) {
            if (p < q) continue;
            Complex sum(0, 0);
            foreach (Patch patch, m_PatchData) {
                sum += patch.size() / m_N * coeff(q.radiansPerSecond + mul * p.radiansPerSecond, patch);
            }
            m[q][p] = sum;
        }
    }
}

Complex HarmonicsCreator::coeff(double omega, const Patch &patch) const {
    double x = patch.step().seconds * omega;
    double beta = double(patch.offset().seconds) / patch.step().seconds + 0.5 * (patch.size() - 1);
    return exp(Complex(0, - beta * x)) * factor(x, patch.size());
}

double HarmonicsCreator::factor(double x, unsigned n) const {
    if (x < 0) x = -x;
    double r = x / (2*M_PI);
    double eps = 0.001 / n;
    int lo = int(r);
    if (r - lo < eps) {
        if ((lo * (n - 1)) % 2) return 1;
        return -1;
    }
    int up = int(r+1);
    if (up - r < eps) {
        if ((up * (n - 1)) % 2) return 1;
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

Tide::RunningSet* HarmonicsCreator::CreateConstituents(int station_id, double cutOff) {
    instance()->reset(station_id);
    return instance()->average(cutOff);
}



