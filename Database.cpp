#include <QDir>
#include <QStandardPaths>
#include <QSqlRecord>
#include <QVariant>
#include <QSqlError>
#include <QDebug>
#include <QtXml/QDomDocument>


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
    m_DB.commit();
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
        while (r.next()) {
            QString key = QString("%1%2%3").arg(r.value(0).toString()).arg(QChar(30)).arg(r.value(1).toString());
            s[key] = r.value(2).toString();
        }
    } else {
        r = instance()->prepare("select suid, xmlinfo from stations where fuid=?");
        r.bindValue(0, provider);
        exec_and_trace(r);
        while (r.next()) {
            s[r.value(0).toString()] = r.value(1).toString();
        }
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
    instance()->close();
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
    instance()->close();
}

int Database::StationID(const QString& station) {
    QSqlQuery r;
    QStringList parts = station.split(QChar::fromLatin1(30));
    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, parts[0]);
    r.bindValue(1, parts[1]);
    exec_and_trace(r);
    if (!r.next()) {
        return 0;
    }
    return r.value(0).toInt();
}

QString Database::StationInfo(const QString& station, const QString& attr) {
    QSqlQuery r;
    QStringList parts = station.split(QChar::fromLatin1(30));
    r = instance()->prepare("select xmlinfo from stations where fuid=? and suid=?");
    r.bindValue(0, parts[0]);
    r.bindValue(1, parts[1]);
    exec_and_trace(r);
    if (!r.next()) {
        return QString();
    }
    QString errMsg;
    int erow;
    int ecol;
    QDomDocument doc(station);
    doc.setContent(r.value(0).toString(), &errMsg, &erow, &ecol);
    if (!errMsg.isEmpty()) {
        qDebug() << errMsg << erow << ecol;
        return QString();
    }
    return doc.firstChild().attributes().namedItem(attr).nodeValue();
}

void Database::UpdateStationInfo(const QString& provider, const QString& station, const QString& xmlinfo) {
    QSqlQuery r;

    r = instance()->prepare("select id from stations where fuid=? and suid=?");
    r.bindValue(0, provider);
    r.bindValue(1, station);
    exec_and_trace(r);
    if (r.next()) {
        int station_id = r.value(0).toInt();
        r = instance()->prepare("update stations set xmlinfo=? where id=?");
        r.bindValue(0, xmlinfo);
        r.bindValue(1, station_id);
    } else {
        r = instance()->prepare("insert into stations (fuid, suid, xmlinfo) values (?, ?, ?)");
        r.bindValue(0, provider);
        r.bindValue(1, station);
        r.bindValue(2, xmlinfo);
    }
    exec_and_trace(r);
}


void Database::Control(const QString& sql, const QVariantList& vars) {
    QSqlQuery r;
    if (vars.isEmpty()) {
        r = instance()->exec(sql);
    } else {
        r = instance()->prepare(sql);
        for (int i = 0; i < vars.size(); ++i) {
            r.bindValue(i, vars[i]);
        }
        exec_and_trace(r);
    }
}

QList<QVector<QVariant>> Database::Query(const QString& sql, const QVariantList& vars) {
    QSqlQuery r;
    if (vars.isEmpty()) {
        r = instance()->exec(sql);
    } else {
        r = instance()->prepare(sql);
        for (int i = 0; i < vars.size(); ++i) {
            r.bindValue(i, vars[i]);
        }
        exec_and_trace(r);
    }
    QList<QVector<QVariant>> s;
    while (r.next()) {
        QVector<QVariant> row(r.record().count());
        for (int i = 0; i < r.record().count(); ++i) {
            // qDebug() << r.value(i);
            row[i] = r.value(i);
        }
        s << row;
    }
    return s;
}

bool Database::Transaction() {
    if (!instance()->m_DB.isOpen()) instance()->m_DB.open();
    return instance()->m_DB.transaction();
}

bool Database::Commit() {
    return instance()->m_DB.commit();
}
