#include <QQmlApplicationEngine>
#include <QtPlugin>
#include <QGuiApplication>
#include <QPluginLoader>
#include <QQmlContext>
#include <QtQml>
#include <QDebug>
#include <QQuickView>

#include <sailfishapp.h>

#include "StationProvider.h"
#include "TideForecast.h"
#include "ActiveStations.h"
#include "Events.h"
#include "CoverModel.h"

Q_IMPORT_PLUGIN(TideForecast)


int main(int argc, char *argv[])
{

    // Set up QML engine.
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));


    QString translations = QString("%1/../share/%2/translations").arg(QCoreApplication::applicationDirPath()).arg(app->applicationName());
    qDebug() << translations;

    if (QDir(translations).exists()) {
        QTranslator *translator = new QTranslator();

        // Locale-based translation
        bool ok = translator->load(QLocale::system(), app->applicationName(), "-", translations);
        if (!ok) {
            translator->load(QDir(translations).filePath(app->applicationName() + "-en"));
        }

        app->installTranslator(translator);
    }


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

    QScopedPointer<QQuickView> view(SailfishApp::createView());


    Tide::Factories factoryModel(factories);
    view->rootContext()->setContextProperty("factoryModel", &factoryModel);

    Tide::StationProvider stations(&factoryModel);
    view->rootContext()->setContextProperty("stationModel", &stations);

    Tide::ActiveStations activeStations(&stations);
    view->rootContext()->setContextProperty("activeStationsModel", &activeStations);


    Tide::CoverModel coverModel(&stations, &activeStations);
    view->rootContext()->setContextProperty("coverModel", &coverModel);

    Tide::Events events(&stations);
    view->rootContext()->setContextProperty("eventsModel", &events);


    view->setSource(SailfishApp::pathTo("qml/tide.qml"));
    view->show();

    return app->exec();
}
