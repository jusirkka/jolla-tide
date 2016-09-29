#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtPlugin>
#include <QPluginLoader>
#include <QQmlContext>
#include <QtQml>
#include <QDebug>

#include "StationProvider.h"
#include "TideForecast.h"
#include "ActiveStations.h"
#include "Events.h"

Q_IMPORT_PLUGIN(TideForecast)


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    QList<Tide::StationFactory*> factories;
    foreach (QObject* plugin, QPluginLoader::staticInstances()) {
        Tide::StationFactory* factory = qobject_cast<Tide::StationFactory*>(plugin);
        if (factory) {
            factories.append(factory);
        }
    }

    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        Tide::StationFactory* factory = qobject_cast<Tide::StationFactory*>(loader.instance());
        if (factory) {
            factories.append(factory);
        }
    }

    qmlRegisterSingletonType(QUrl("file:///home/jusirkka/src/jolla-tide/Theme.qml"), "net.kvanttiapina.tide.theme", 1, 0, "Theme");

    QQmlApplicationEngine engine;
    QQmlContext *ctxt = engine.rootContext();

    Tide::Factories factoryModel(factories);
    ctxt->setContextProperty("factoryModel", &factoryModel);

    Tide::StationProvider stations(&factoryModel);
    ctxt->setContextProperty("stationModel", &stations);

    Tide::ActiveStations activeStations(&stations);
    ctxt->setContextProperty("activeStationsModel", &activeStations);

    Tide::Events events(&stations);
    ctxt->setContextProperty("eventsModel", &events);

    engine.load(QUrl(QStringLiteral("tide.qml")));

    return app.exec();
}
