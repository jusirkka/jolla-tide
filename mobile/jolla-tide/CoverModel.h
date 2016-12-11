#ifndef COVER_MODEL_H
#define COVER_MODEL_H

#include <QAbstractListModel>
#include "ActiveStations.h"
#include "StationProvider.h"
#include "Timestamp.h"
#include "TideEvent.h"

namespace Tide {


class CoverModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString station READ station NOTIFY stationChanged)


public:

    CoverModel(StationProvider* parent, ActiveStations* stations);

    enum EventRoles {
        EventIconRole = Qt::UserRole + 1,
        EventDescRole,
        EventTimestampRole
    };

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QHash<int, QByteArray> roleNames() const;



    ~CoverModel();

    QString station() const;


signals:

    void stationChanged(const QString& station);

private slots:

    void updateCover();

private:

    StationProvider* m_Parent;
    ActiveStations* m_Stations;

    QString m_Key;
    QString m_Station;
    QList<TideEvent> m_Events;
    int m_Size;
};



}
#endif
