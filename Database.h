#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QHash>
#include <QVariantList>

namespace Tide {

class Database {

public:

    class Active {
    public:
        Active(const QString& s = QString(), const QString& m = QString()): station(s), mark(m) {}
        QString station;
        QString mark;
    };

    typedef QList<Active> ActiveList;

    // table actives
    static ActiveList ActiveStations();
    static void OrderActives(const QStringList& ordering);
    static void SetMark(const QString& station, const QString& mark);

    // table stations
    static QHash<QString, QString> AllStations(const QString& provider = QString());
    static int StationID(const QString& station);
    static QString StationInfo(const QString& station, const QString& attr);
    static void UpdateStationInfo(const QString& provider, const QString& station, const QString& xmlinfo);

    static void Control(const QString& sql, const QVariantList& vars = QVariantList());
    static QList<QVector<QVariant>> Query(const QString& sql, const QVariantList& vars = QVariantList());


    static bool Transaction();
    static bool Commit();

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
