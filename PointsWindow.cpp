#include "PointsWindow.h"
#include "Complex.h"
#include "Database.h"
#include "Amplitude.h"
#include "Station.h"

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
#include <fftw3.h>

using namespace Tide;

class ReadingsIterator {
public:
    ReadingsIterator(const Address& addr, const Station* s);
    bool next();
    Timestamp stamp();
    double reading();

    ~ReadingsIterator() {}

private:

    const Station* m_Station;

    int m_CurrentPatch;
    int m_CurrentStamp;
    int m_Step;

    QVector<qint64> m_Epochs;

    QMap<qint64, qint64> m_PatchFirst;
    QMap<qint64, qint64> m_PatchLast;
    QMap<qint64, qint64> m_Steps;
    QMap<qint64, QVector<double>> m_Readings;


};


ReadingsIterator::ReadingsIterator(const Address &addr, const Station* s):
    m_Station(s),
    m_CurrentPatch(0) {

    int station_id = Database::StationID(addr);

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select id, start, timedelta, patchsize from epochs where station_id=? order by start", vars);


    qint64 max_step = 0;
    foreach (QVector<QVariant> row, r) {
        qint64 epoch_id = row[0].toInt();
        m_Epochs.append(epoch_id);

        qint64 start = row[1].toInt();
        qint64 step = row[2].toInt();

        if (step > max_step) max_step = step;

        qint64 patchsize = row[3].toInt();

        m_PatchFirst[epoch_id] = start;
        m_PatchLast[epoch_id] = start + step * (patchsize - 1);
        m_Steps[epoch_id] = step;

        vars.clear();
        vars << QVariant::fromValue(epoch_id);
        QList<QVector<QVariant>> raw = Database::Query("select reading from readings where epoch_id=?", vars);
        foreach (QVector<QVariant> v, raw) {
            m_Readings[epoch_id].append(v[0].toDouble());
        }
    }

    m_Step = max_step;
    m_CurrentStamp = m_PatchFirst[m_Epochs.first()] - m_Step;
}


bool ReadingsIterator::next() {
    if (m_CurrentPatch >= m_Epochs.size()) return false;
    m_CurrentStamp += m_Step;
    if (m_CurrentStamp > m_PatchLast[m_Epochs[m_CurrentPatch]]) {
        m_CurrentPatch += 1;
    }
    if (m_CurrentPatch >= m_Epochs.size()) return false;
    return true;
}

Timestamp ReadingsIterator::stamp() {
    return Timestamp::fromPosixTime(m_CurrentStamp);
}

double ReadingsIterator::reading() {

    qint64 epoch_id = m_Epochs[m_CurrentPatch];
    if (m_CurrentStamp < m_PatchFirst[epoch_id]) {
        // between patches
        Amplitude v = m_Station->predictTideLevel(Timestamp::fromPosixTime(m_CurrentStamp));
        return v.value;
    }

    qint64 delta = m_CurrentStamp - m_PatchFirst[epoch_id];
    qint64 step = m_Steps[epoch_id];
    qint64 index = delta / step;
    if (delta % step == 0) {
        return m_Readings[epoch_id][index];
    }

    // linear interpolation
    double x = double (delta % step) / step;
    return m_Readings[epoch_id][index] * (1 - x) + m_Readings[epoch_id][index + 1] * x;
}


PointsWindow::PointsWindow(const Address& addr, const Station& station):
    QStackedWidget()
{

    QVector<double> orig;
    QVector<double> gen;
    QVector<Timestamp> stamps;

    ReadingsIterator points(addr, &station);
    while (points.next()) {
        stamps.append(points.stamp());
        orig.append(points.reading());
    }

    foreach (Timestamp t, stamps) {
        Amplitude v = station.predictTideLevel(t);
        gen.append(v.value);
    }

    QString stationName = Database::StationInfo(addr, "name");

    addWidget(new TimeDomain(stationName, stamps, orig, gen));
    addWidget(new FrequencyDomain(stationName, stamps, orig, gen));
}


void PointsWindow::mousePressEvent(QMouseEvent*) {
    int curr = currentIndex();
    int c = count();
    setCurrentIndex((curr + 1) % c);
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
    setAxisScale(QwtPlot::yLeft, 0, 60);
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
        korig.append(Complex(cp[k][0], cp[k][1]).mod());
    }

    for(int i = 0; i < count; i++) {
        dp[i] = gen[i];
    }

    fftw_execute(plan);

    QVector<double> kgen;
    for (int k = 0; k < count / 2; k++) {
        kgen.append(Complex(cp[k][0], cp[k][1]).mod());
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


