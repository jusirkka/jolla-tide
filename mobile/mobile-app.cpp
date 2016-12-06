#include <QQmlApplicationEngine>
#include <QtPlugin>
#include <QGuiApplication>
#include <QPluginLoader>
#include <QQmlContext>
#include <QtQml>
#include <QDebug>

#include <sailfishapp.h>

#include "StationProvider.h"
#include "TideForecast.h"
#include "ActiveStations.h"
#include "Events.h"

Q_IMPORT_PLUGIN(TideForecast)


int main(int argc, char *argv[])
{

    // Set up QML engine.
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));


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


    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->setSource(QUrl(":/silica/tide.qml"));
    view->show();

    return app->exec();
}
