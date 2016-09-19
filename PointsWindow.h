#ifndef POINTSWINDOW_H
#define POINTSWINDOW_H

#include <QStackedWidget>
#include <qwt_plot_curve.h>
#include <qwt_scale_map.h>

namespace Tide {

class Station;


class GraphFrame: public QFrame {
public:

    GraphFrame(const QString& name);

protected:

    void paintEvent(QPaintEvent *);
    void drawContents(QPainter *p);

    QString m_Name;
    QwtPlotCurve m_Orig, m_Gen;
    QwtScaleMap m_Xmap;
    QwtScaleMap m_Ymap;
};

class TimeDomain: public GraphFrame {
public:

    TimeDomain(const QVector<int>& stamps, const QVector<double>& orig, const QVector<double>& gen);

};


class FrequencyDomain: public GraphFrame {
public:

    FrequencyDomain(const QVector<int>& stamps, const QVector<double>& orig, const QVector<double>& gen);

};

class PointsWindow : public QStackedWidget {
public:

    PointsWindow(const QString& fuid, const QString& suid, const Station& station);

protected:

    void mousePressEvent(QMouseEvent* event);

private:


};

}

#endif // POINTSWINDOW_H
