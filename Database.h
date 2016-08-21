#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QHash>

namespace Tide {

class Database {

public:

    static QStringList ActiveStations(const QString& provider = QString());
    static QHash<QString, QString> AllStations(const QString& provider = QString());
    static void Activate(const QString& station);
    static void Deactivate(const QString& station);
    static void AddStation(const QString& provider, const QString& station, const QString& xmlinfo);

private:

    static Database* instance();
    Database();
    Database(const Database&);
    Database& operator=(const Database&);

    QSqlQuery& exec(const QString& query);
    QSqlQuery& prepare(const QString& query);
    void close();

private:

    QSqlDatabase m_DB;
    QSqlQuery m_Query;

};

}

#endif // DATABASE_H
