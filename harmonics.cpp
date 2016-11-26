#include <QApplication>
#include <QDebug>

#include "HarmonicsCreator.h"
#include "PointsWindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    bool ok;
    if (argc < 2) return 1;
    int station_id = QString(argv[1]).toInt(&ok);
    if (!ok) return 1;
    QString key;
    double value;
    for (int k = 2; k < argc; k++) {
        if (k % 2 == 0) {
            key = QString(argv[k]);
        } else {
            value = QString(argv[k]).toDouble(&ok);
            if (!ok) return 1;
            Tide::HarmonicsCreator::Config(key, value);
        }
    }
    Tide::HarmonicsCreator::Delete(station_id);
    Tide::PointsWindow w(station_id);
    w.resize(1600, 700);
    w.show();
    return app.exec();
}
