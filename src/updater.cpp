#include <QCoreApplication>
#include <QtPlugin>
#include <QPluginLoader>
#include <QDebug>
#include <QtDBus/QDBusConnection>

#include "TideForecast.h"
#include "stationupdater_adaptor.h"
#include "Updater.h"

Q_IMPORT_PLUGIN(TideForecast)


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

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

    Tide::Updater* updater = new Tide::Updater(factories);
    new ManagerAdaptor(updater);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerObject("/Updater", updater);
    conn.registerService("net.kvanttiapina.tide");
   
    return app.exec();
}
