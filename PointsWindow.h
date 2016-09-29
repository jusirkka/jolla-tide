#ifndef POINTSWINDOW_H
#define POINTSWINDOW_H

#include <QStackedWidget>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_scaleitem.h>

#include "Timestamp.h"

namespace Tide {

class Station;

class GraphFrame: public QwtPlot {
public:
    GraphFrame(const QString& name);

protected:

    QwtPlotCurve m_Orig, m_Gen;
    QwtPlotScaleItem m_Bottom, m_Left;

};

class TimeDomain: public GraphFrame {
public:

    TimeDomain(const QString& station, const QVector<Timestamp>& stamps, const QVector<double>& orig, const QVector<double>& gen);

};


class FrequencyDomain: public GraphFrame {
public:

    FrequencyDomain(const QString& station, const QVector<Timestamp>& stamps, const QVector<double>& orig, const QVector<double>& gen);

};

class PointsWindow : public QStackedWidget {
public:

    PointsWindow(const QString& key, const Station& station);

protected:

    void mousePressEvent(QMouseEvent* event);

private:


};

}

#endif // POINTSWINDOW_H
