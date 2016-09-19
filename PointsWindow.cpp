#include "PointsWindow.h"
#include "Complex.h"
#include "Database.h"
#include "Amplitude.h"
#include "Station.h"

#include <qwt_scale_map.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_math.h>
#include <QMouseEvent>
#include <fftw3.h>

using namespace Tide;

PointsWindow::PointsWindow(const QString& fuid, const QString& suid, const Station& station):
    QStackedWidget()
{

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(fuid) << QVariant::fromValue(suid);
    r = Database::Query("select id from stations where fuid=? and suid=?", vars);
    int station_id = r.first()[0].toInt();
    vars.clear();
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select id, start, timedelta from epochs where station_id=?", vars);
    QVector<double> orig;
    QVector<int> stamps;
    foreach (QVector<QVariant> row, r) {
        int epoch_id = row[0].toInt();
        int start = row[1].toInt();
        int step = row[2].toInt();
        qDebug() << epoch_id << start << step;
        vars.clear();
        vars << QVariant::fromValue(epoch_id);
        QList<QVector<QVariant>> raw = Database::Query("select reading from readings where epoch_id=?", vars);
        int curr = start;
        foreach (QVector<QVariant> v, raw) {
            stamps.append(curr);
            orig.append(v[0].toDouble());
            curr += step;
        }
    }
    QVector<double> gen;
    foreach (int t, stamps) {
        Amplitude v = station.predictTideLevel(Timestamp::fromPosixTime(t));
        gen.append(v.value);
    }


    addWidget(new TimeDomain(stamps, orig, gen));
    addWidget(new FrequencyDomain(stamps, orig, gen));
}


void PointsWindow::mousePressEvent(QMouseEvent*) {
    int curr = currentIndex();
    int c = count();
    setCurrentIndex((curr + 1) % c);
}

GraphFrame::GraphFrame(const QString& name): QFrame(), m_Name(name) {
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);
    setMidLineWidth(3);
}

void GraphFrame::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QPainter painter(this);
    painter.setClipRect(contentsRect());
    drawContents(&painter);
}


void GraphFrame::drawContents(QPainter *painter) {
    QRect r = contentsRect();

    //
    //  draw curves
    //
    m_Xmap.setPaintInterval(r.left(), r.right());
    m_Ymap.setPaintInterval(r.top(), r.bottom());

    painter->setRenderHint(QPainter::Antialiasing, m_Orig.testRenderHint(QwtPlotItem::RenderAntialiased));
    m_Orig.draw(painter, m_Xmap, m_Ymap, r);

    painter->setRenderHint(QPainter::Antialiasing, m_Gen.testRenderHint(QwtPlotItem::RenderAntialiased));
    m_Gen.draw(painter, m_Xmap, m_Ymap, r);


    //
    // draw titles
    //
    painter->setFont(QFont("Helvetica", 12));
    painter->setPen(Qt::black);
    painter->drawText(0, r.top(), r.width(), painter->fontMetrics().height(), Qt::AlignTop | Qt::AlignHCenter, m_Name);
}




TimeDomain::TimeDomain(const QVector<int>& stamps, const QVector<double>& orig, const QVector<double>& gen):
    GraphFrame("Time domain")
{

    double number_of_days = (stamps[stamps.size() - 1] - stamps[0]) / 3600. / 24.;
    qDebug() << "days = " << number_of_days;
    m_Xmap.setScaleInterval(0, number_of_days);

    QVector<double> days;
    double start = stamps[0];

    foreach (int t, stamps) {
        // qDebug() << (t - start) / 3600. / 24.;
        days.append((t - start) / 3600. / 24.);
    }

    double lmax = 0;
    double lmin = 100;
    foreach (double x, orig) {
        if (x < lmin) lmin = x;
        if (x > lmax) lmax = x;
    }
    foreach (double x, gen) {
        if (x < lmin) lmin = x;
        if (x > lmax) lmax = x;
    }

    // qDebug() << lmin << lmax;
    m_Ymap.setScaleInterval(lmax * (lmax < 0 ? 0.95 : 1.05), lmin * (lmin < 0 ? 1.05 : 0.95));

    //
    //  define curve styles
    //
    m_Orig.setPen(Qt::darkGreen);
    m_Orig.setStyle(QwtPlotCurve::Lines);

    m_Gen.setPen(Qt::red);
    m_Gen.setStyle(QwtPlotCurve::Lines);


    //
    // attach data
    //
    m_Orig.setSamples(days, orig);
    m_Gen.setSamples(days, gen);
}




FrequencyDomain::FrequencyDomain(const QVector<int>& stamps, const QVector<double>& orig, const QVector<double>& gen):
    GraphFrame("Frequency domain")
{
    // TODO: handle different step sizes
//     int step = stamps[1] - stamps[0];
    int count = 2 * (stamps.size() / 2);
//    if ((count * step) % 43200 == 0) {
//        // S2 resonance prevention
//        qDebug() << "S2 resonance prevention";
//        count -= 2;
//    }

    QVector<double> ks;

    for (int ki = 0; ki < count / 2; ++ki) {
        ks.append(ki);
    }

    m_Xmap.setScaleInterval(0, count/10);


    double *dp = (double *)malloc(sizeof(double) * (count));
    fftw_complex *cp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * count);

    fftw_plan plan = fftw_plan_dft_r2c_1d(count, dp, cp, FFTW_MEASURE);
    for (int i = 0; i < count; i++) {
        dp[i] = orig[i];
        // qDebug() << dp[i];
    }
    fftw_execute(plan);

    QVector<double> korig;
    for (int k = 0; k < count / 2; k++) {
        // qDebug() << Complex(cp[k][0], cp[k][1]).mod();
        korig.append(Complex(cp[k][0], cp[k][1]).mod());
    }


    for(int i = 0; i < count; i++) {
        dp[i] = gen[i];
    }

    fftw_execute(plan);

    QVector<double> kgen;
    for (int k = 0; k < count / 2; k++) {
        // qDebug() << Complex(cp[k][0], cp[k][1]).mod();
        kgen.append(Complex(cp[k][0], cp[k][1]).mod());
    }

    fftw_destroy_plan(plan);


    double fmax = 60;
//    foreach (double f, korig) {
//        if (f > fmax) fmax = f;
//    }
//    foreach (double f, kgen) {
//        if (f > fmax) fmax = f;
//    }

    m_Ymap.setScaleInterval(fmax, 0);

    //
    //  define curve styles
    //
    m_Orig.setPen(Qt::darkGreen);
    m_Orig.setStyle(QwtPlotCurve::Lines);
    m_Orig.setSymbol(new QwtSymbol(QwtSymbol::Cross, Qt::NoBrush, QPen(Qt::darkGreen), QSize(5, 5)));

    m_Gen.setPen(Qt::red);
    m_Gen.setStyle(QwtPlotCurve::Lines);
    m_Gen.setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush, QPen(Qt::red), QSize(5, 5)));


    //
    // attach data
    //
    m_Orig.setSamples(ks, korig);
    m_Gen.setSamples(ks, kgen);
}


