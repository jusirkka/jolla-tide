#include "PointsWindow.h"
#include "Complex.h"
#include "Database.h"
#include "Amplitude.h"
#include "Station.h"
#include "PatchIterator.h"
#include "HarmonicsCreator.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_scaleitem.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_math.h>
#include <qwt_plot_grid.h>
#include <qwt_date.h>
#include <qwt_date_scale_engine.h>
#include <qwt_date_scale_draw.h>

#include <QMouseEvent>
#include <QApplication>
#include <fftw3.h>

using namespace Tide;



PointsWindow::PointsWindow(const Address& addr, const Station& station):
    QStackedWidget()
{

    QVector<double> orig;
    QVector<double> gen;
    QVector<Timestamp> stamps;

    int station_id = Database::StationID(addr);
    PatchIterator points(station_id);

    if (points.lastPatch()) {
        while (points.next()) {
            stamps.append(points.stamp());
            orig.append(points.reading());
        }
    }

    foreach (Timestamp t, stamps) {
        Amplitude v = station.predictTideLevel(t);
        gen.append(v.value);
    }

    QString stationName = Database::StationInfo(addr, "name");

    addWidget(new TimeDomain(stationName, stamps, orig, gen));
    addWidget(new FrequencyDomain(stationName, stamps, orig, gen));
}


PointsWindow::PointsWindow(int station_id):
    QStackedWidget()
{

    QVector<double> orig;
    QVector<double> gen;
    QVector<Timestamp> stamps;

    PatchIterator points(station_id);

    if (points.lastPatch()) {
        while (points.next()) {
            stamps.append(points.stamp());
            orig.append(points.reading());
        }
    }

    Tide::RunningSet* rset = Tide::HarmonicsCreator::CreateConstituents(station_id);


    foreach (Timestamp t, stamps) {
        Amplitude v = rset->datum() + rset->tideDerivative(t, 0);
        gen.append(v.value);
    }

    QString stationName = QString("Station %1").arg(station_id);

    addWidget(new TimeDomain(stationName, stamps, orig, gen));
    addWidget(new FrequencyDomain(stationName, stamps, orig, gen));
}


void PointsWindow::mousePressEvent(QMouseEvent*) {
    int curr = currentIndex();
    int c = count();
    setCurrentIndex((curr + 1) % c);
}

void PointsWindow::keyPressEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Escape) {
       close();
    }
}

GraphFrame::GraphFrame(const QString &name): QwtPlot(QwtText(name)) {

    canvas()->setContentsMargins(0, 0, 0, 0);
    setStyleSheet("QwtPlot{ border: 0; }");
    canvas()->setStyleSheet("QwtPlotCanvas {border: none; margin: 1; background-color: white;}");
    plotLayout()->setCanvasMargin(0);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach(this);

    enableAxis(QwtPlot::yLeft);
    enableAxis(QwtPlot::xBottom);

    axisWidget(QwtPlot::yLeft)->setMargin(0);
    axisWidget(QwtPlot::yLeft)->setSpacing(0);
    axisWidget(QwtPlot::xBottom)->setMargin(0);


    updateCanvasMargins();

    m_Orig.setTitle("Original");
    m_Orig.setPen(Qt::darkGreen);
    m_Orig.setStyle(QwtPlotCurve::Lines);
    m_Orig.attach(this);

    m_Gen.setTitle("Generated");
    m_Gen.setPen(Qt::red);
    m_Gen.setStyle(QwtPlotCurve::Lines);
    m_Gen.attach(this);
}

TimeDomain::TimeDomain(const QString& station, const QVector<Timestamp>& stamps, const QVector<double>& orig, const QVector<double>& gen):
    GraphFrame(QString("%1: Time domain").arg(station))
{

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

    setAxisScale(QwtPlot::yLeft, lmin * (lmin < 0 ? 1.05 : 0.95), lmax * (lmax < 0 ? 0.95 : 1.05));

    Timestamp start = stamps[0];
    Interval timescale = stamps[stamps.size() - 1] - start;
    qDebug() << "days = " << timescale.days();


    QwtDateScaleDraw* scaleDraw = new QwtDateScaleDraw(Qt::UTC);
    QwtDateScaleEngine* scaleEngine = new QwtDateScaleEngine(Qt::UTC);

    scaleDraw->setDateFormat(QwtDate::Day, "MM-dd");

    setAxisScaleDraw(QwtPlot::xBottom, scaleDraw);
    setAxisScaleEngine(QwtPlot::xBottom, scaleEngine);

    QVector<double> msecs;
    foreach (Timestamp t, stamps) {
        msecs.append(t.posix()*1000);
    }

    m_Orig.setSamples(msecs, orig);
    m_Gen.setSamples(msecs, gen);

    replot();
}




FrequencyDomain::FrequencyDomain(const QString& station, const QVector<Timestamp>& stamps, const QVector<double>& orig, const QVector<double>& gen):
    GraphFrame(QString("%1: Frequency domain").arg(station))
{
    setAxisScale(QwtPlot::yLeft, 0, 4);
    setAxisScale(QwtPlot::xBottom, 0, 200);

    int count = 2 * (stamps.size() / 2);

    double *dp = (double *)malloc(sizeof(double) * (count));
    fftw_complex *cp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * count);

    fftw_plan plan = fftw_plan_dft_r2c_1d(count, dp, cp, FFTW_MEASURE);
    for (int i = 0; i < count; i++) {
        dp[i] = orig[i];
    }
    fftw_execute(plan);

    QVector<double> korig;
    for (int k = 0; k < count / 2; k++) {
        korig.append(std::log10(1 + Complex(cp[k][0], cp[k][1]).mod()));
    }

    for(int i = 0; i < count; i++) {
        dp[i] = gen[i];
    }

    fftw_execute(plan);

    QVector<double> kgen;
    for (int k = 0; k < count / 2; k++) {
        kgen.append(std::log10(1 + Complex(cp[k][0], cp[k][1]).mod()));
    }

    fftw_destroy_plan(plan);


    //
    //  define curve styles
    //
    m_Orig.setSymbol(new QwtSymbol(QwtSymbol::Cross, Qt::NoBrush, QPen(Qt::darkGreen), QSize(5, 5)));
    m_Gen.setSymbol(new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush, QPen(Qt::red), QSize(5, 5)));


    QVector<double> degrees_per_hour;
    double step = (stamps[1] - stamps[0]).seconds;
    for (int k = 0; k < count; ++k) {
        degrees_per_hour.append(Speed::fromRadiansPerSecond(2 * M_PI * k / count / step).dph());
    }

    m_Orig.setSamples(degrees_per_hour, korig);
    m_Gen.setSamples(degrees_per_hour, kgen);

    replot();
}


