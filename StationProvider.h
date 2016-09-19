#ifndef STATION_PROVIDER_H
#define STATION_PROVIDER_H

#include <QAbstractListModel>
#include <QtXml/QDomDocument>
#include "Factories.h"



namespace Tide {

class StationProvider;

class StationUpdateHandler: public ClientProxy {
public:
    StationUpdateHandler(StationProvider* parent, const QString& key);
    void whenFinished(const Status&);
    ClientProxy* clone();

private:

    StationProvider* m_Parent;
    QString m_Key;
};




class StationProvider: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

public:

    enum StationRoles {
        NameRole = Qt::UserRole + 1,
        DetailRole, // County, Country, Region
        LocationRole,
        TypeRole, // current/master/slave/...
        KeyRole
    };


public:

    // create new provider model
    StationProvider(Factories* factories, QObject* parent = 0);

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QHash<int, QByteArray> roleNames() const;

    ~StationProvider();

    QDomNode info(const QString& key);
    const Station& station(const QString& key);


    QString filter() const;
    void setFilter(const QString& s);

public slots:

    void updateAvailable(const QString&);

signals:

    void filterChanged(const QString& filter);
    void stationChanged(const QString& key);


private:

    QHash<QString, QDomDocument> m_AvailableStations;
    QList<QString> m_Visible;
    Factories* m_Factories;
    QString m_Filter;
    Station m_Invalid;

};



}
#endif
