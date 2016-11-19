#include <QApplication>
#include <QDebug>

#include "HarmonicsCreator.h"
#include "PointsWindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    bool ok;
    if (argc < 3) return 1;
    int station_id = QString(argv[1]).toInt(&ok);
    if (!ok) return 1;
    double cutOff = QString(argv[2]).toDouble(&ok);
    if (!ok) return 1;
    Tide::PointsWindow w(station_id, cutOff);
    w.resize(1600, 800);
    w.show();
    return app.exec();
}
