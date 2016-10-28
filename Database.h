#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariantList>

#include "Address.h"

namespace Tide {

class Database {

public:

    class Active {
    public:
        Active(const Address& a = Address(), const QString& m = QString()): address(a), mark(m) {}
        Address address;
        QString mark;
    };

    typedef QList<Active> ActiveList;

    // table actives
    static ActiveList ActiveStations();
    static void OrderActives(const Address::AddressList& ordering);
    static void SetMark(const Address& station, const QString& mark);

    // table stations
    static QHash<Address, QString> AllStations(const QString& provider = QString());
    static int StationID(const Address& station);
    static QString StationInfo(const Address& station, const QString& attr);
    static void UpdateStationInfo(const Address& provider, const QString& xmlinfo);

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
