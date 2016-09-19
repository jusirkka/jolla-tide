#ifndef ACTIVE_STATIONS_H
#define ACTIVE_STATIONS_H

#include <QAbstractListModel>
#include <QtXml/QDomDocument>
#include <QTimer>

#include "TideEvent.h"

namespace Tide {

class StationProvider;

class ActiveStations: public QAbstractListModel
{
    Q_OBJECT

public:

    enum ActiveStationRoles {
        NameRole = Qt::UserRole + 1,
        KeyRole,
        NextEventIconRole,
        NextEventDescRole,
        NextEventTimestampRole
    };


public:

    // create new provider model
    ActiveStations(StationProvider* parent);

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QHash<int, QByteArray> roleNames() const;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) Q_DECL_OVERRIDE;

    Q_INVOKABLE void append(const QString& station);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void movetotop(int row);
    Q_INVOKABLE void showpoints(int row);

    ~ActiveStations();

private slots:

    void computeNextEvent();
    void stationChanged(const QString&);

private:

    class Data {
    public:
        TideEvent next;
        QTimer* recompute;
    };

    QList<QString> m_Stations;
    QHash<QString, Data> m_Events;
    StationProvider* m_Parent;
};

}
#endif
