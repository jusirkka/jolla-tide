#include <QDir>
#include <QStandardPaths>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlError>
#include <QDebug>

#include "Database.h"

using namespace Tide;

Database::Database() {

    m_DB = QSqlDatabase::addDatabase("QSQLITE");
    QString dbfile = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "tides.db");
    if (dbfile.isEmpty()) {
        QString loc = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        if (!QDir(loc).exists()) {
            QDir().mkpath(loc);
        }
        dbfile = QString("%1/tides.db").arg(loc);
    }
    m_DB.setDatabaseName(dbfile);
    m_DB.open();
    QSqlQuery query;
    query.exec("create table if not exists stations ("
               "id integer primary key autoincrement, "
               "fuid text not null, "
               "suid text not null, "
               "xmlinfo text not null)");
    query.exec("create table if not exists actives ("
               "id integer primary key autoincrement, "
               "station_id integer not null)");
    m_DB.close();
}

Database* Database::instance() {
    static Database* db = new Database();
    return db;
}

QSqlQuery& Database::exec(const QString& sql) {
    if (!m_DB.isOpen()) m_DB.open();
    m_Query = QSqlQuery(m_DB);
    // qDebug() << sql;
    m_Query.exec(sql);
    if (m_Query.lastError().isValid()) qDebug() << m_Query.lastError();
    return m_Query;
}


QSqlQuery& Database::prepare(const QString& sql) {
    if (!m_DB.isOpen()) m_DB.open();
    m_Query = QSqlQuery(m_DB);
    // qDebug() << sql;
    m_Query.prepare(sql);
    if (m_Query.lastError().isValid()) qDebug() << m_Query.lastError();
    return m_Query;
}

void Database::close() {
    m_DB.close();
}

static void exec_and_trace(QSqlQuery& r) {
    r.exec();
    if (r.lastError().isValid()) qDebug() << r.lastError();
}

QStringList Database::ActiveStations(const QString& provider) {
    QStringList s;
    QSqlQuery r;
    if (provider.isEmpty()) {
         r = instance()->exec("select s.fuid, s.suid from stations s join actives a on s.id=a.station_id");
    } else {
        r = instance()->prepare("select s.fuid, s.suid from stations s join actives a on s.id=a.station_id where s.fuid=?");
        r.bindValue(0, provider);
        exec_and_trace(r);
    }
    while (r.next()) {
        s << QString("%1%2%3").arg(r.value(0).toString()).arg(QChar(30)).arg(r.value(1).toString());
    }
    instance()->close();
    return s;
}

QHash<QString, QString> Database::AllStations(const QString& provider) {
    QHash<QString, QString> s;
    QSqlQuery r;
    if (provider.isEmpty()) {
        r = instance()->exec("select fuid, suid, xmlinfo from stations");
    } else {
        r = instance()->prepare("select fuid, suid, xmlinfo from stations where fuid=?");
        r.bindValue(0, provider);
        exec_and_trace(r);
    }
    while (r.next()) {
        QString key = QString("%1%2%3").arg(r.value(0).toString()).arg(QChar(30)).arg(r.value(1).toString());
        s[key] = r.value(2).toString();
    }
    instance()->close();
    return s;
}

void Database::Activate(const QString& station) {
    QSqlQuery r;
    QStringList parts = station.split(QChar::fromLatin1(30));
    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, parts[0]);
    r.bindValue(1, parts[1]);
    exec_and_trace(r);
    if (!r.next()) {
        return;
    }
    int station_id = r.value(0).toInt();
    r = instance()->prepare("select id from actives where station_id=?");
    r.bindValue(0, station_id);
    exec_and_trace(r);
    if (r.next()) {
        return; // already active
    }
    r = instance()->prepare("insert into actives (station_id) values (?)");
    r.bindValue(0, station_id);
    exec_and_trace(r);
}

void Database::Deactivate(const QString& station) {
    QSqlQuery r;
    QStringList parts = station.split(QChar::fromLatin1(30));
    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, parts[0]);
    r.bindValue(1, parts[1]);
    exec_and_trace(r);
    if (!r.next()) {
        return;
    }
    int station_id = r.value(0).toInt();
    r = instance()->prepare("delete from actives where station_id=?");
    r.bindValue(0, station_id);
    exec_and_trace(r);
}


void Database::AddStation(const QString& provider, const QString& station, const QString& xmlinfo) {
    QSqlQuery r;

    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, provider);
    r.bindValue(1, station);
    exec_and_trace(r);
    if (r.next()) {
        return;
    }

    r = instance()->prepare("insert into stations (fuid, suid, xmlinfo) values (?, ?, ?)");
    r.bindValue(0, provider);
    r.bindValue(1, station);
    r.bindValue(2, xmlinfo);
    exec_and_trace(r);
}

//query.exec("create table if not exists epochs ("
//           "id integer primary key, "
//           "station_id integer not null, "
//           "start integer not null, "
//           "timedelta integer not null)");
//query.exec("create table if not exists readings ("
//           "id integer primary key, "
//           "epoch_id integer not null, "
//           "reading real not null)");
//query.exec("create table if not exists constituents ("
//           "id integer primary key, "
//           "epoch_id integer not null, "
//           "mode_id integer not null, "
//           "rea real not null, "
//           "ima real not null)");
//query.exec("create table if not exists modes ("
//           "id integer primary key, "
//           "name text not null, "
//           "frequency real not null)");
