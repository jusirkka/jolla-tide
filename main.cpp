#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtPlugin>
#include <QPluginLoader>
#include <QDir>
#include <QQmlContext>
#include <QtQml>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDebug>

#include "StationProvider.h"
#include "Dummy.h"
#include "ActiveStations.h"
#include "Events.h"

Q_IMPORT_PLUGIN(DummyStations)

int create_tidebase() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbfile = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "tides.db");
    if (dbfile.isEmpty()) {
        QString loc = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (loc.isEmpty()) return -1;
        // TODO: create datalocation dir if not exists
        dbfile = QString("%1/tides.db").arg(loc);
    }
    if (dbfile.isEmpty()) return -1;
    db.setDatabaseName(dbfile);
    if (!db.open()) return -1;
    QSqlQuery query;
    query.exec("create table if not exists stations ("
               "id integer primary key, "
               "suid text not null, "
               "fuid text not null, "
               "xmlinfo text not null)");
    query.exec("create table if not exists actives ("
               "id integer primary key, "
               "station_id integer not null)");
    query.exec("create table if not exists epochs ("
               "id integer primary key, "
               "station_id integer not null, "
               "start integer not null, "
               "timedelta integer not null)");
    query.exec("create table if not exists readings ("
               "id integer primary key, "
               "epoch_id integer not null, "
               "reading real not null)");
    query.exec("create table if not exists constituents ("
               "id integer primary key, "
               "epoch_id integer not null, "
               "mode_id integer not null, "
               "rea real not null, "
               "ima real not null)");
    query.exec("create table if not exists modes ("
               "id integer primary key, "
               "name text not null, "
               "frequency real not null)");
    db.close();
    return 1;

}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

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

    if (create_tidebase() < 0) {
        return 1;
    }

    qmlRegisterSingletonType(QUrl("file:///home/jusirkka/src/untitled/Theme.qml"), "net.kvanttiapina.tide.theme", 1, 0, "Theme");

    Tide::StationProvider stations(factories);
    QQmlApplicationEngine engine;
    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("stationModel", &stations);
    Tide::ActiveStations activeStations(&stations);
    ctxt->setContextProperty("activeStationsModel", &activeStations);
    Tide::Events events(&stations);
    ctxt->setContextProperty("eventsModel", &events);
    engine.load(QUrl(QStringLiteral("tide.qml")));
    return app.exec();
}
