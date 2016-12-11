#include "CoverModel.h"
#include "Address.h"
#include <QDebug>

using namespace Tide;


CoverModel::~CoverModel() {}


CoverModel::CoverModel(StationProvider* parent, ActiveStations* stations):
    QAbstractListModel(parent),
    m_Parent(parent),
    m_Stations(stations),
    m_Size(3)
{
    updateCover();
    connect(m_Stations, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(updateCover()));
}

QString CoverModel::station() const {
    return m_Station;
}


void CoverModel::updateCover() {
    beginResetModel();

    m_Events.clear();

    QModelIndex top = m_Stations->index(0);

    if (top.isValid()) {
        QString key = m_Stations->data(top, ActiveStations::KeyRole).toString();

        QString station = m_Stations->data(top, ActiveStations::NameRole).toString();
        if (station != m_Station) {
            m_Station = station;
            emit stationChanged(m_Station);
        }

        Timestamp start = Timestamp::now();

        QString m = m_Stations->data(top, ActiveStations::MarkRole).toString();
        Amplitude mark = Amplitude::parseDottedMeters(m);

        const Station& s = m_Parent->station(key);
        TideEvent::Organizer org;
        while (org.size() < m_Size) {
            Timestamp then = start + Interval::fromSeconds(3600*24);
            s.predictTideEvents(start, then, org, mark);
            start = then;
        }

        QMapIterator<Timestamp, TideEvent> ev(org);
        int cnt = m_Size;
        while (ev.hasNext() && cnt > 0) {
            ev.next();
            cnt--;
            m_Events.push_back(ev.value());
        }


    } else {
        QString station = "Kokomo";
        if (station != m_Station) {
            m_Station = station;
            emit stationChanged(m_Station);
        }
    }

    endResetModel();

}


int CoverModel::rowCount(const QModelIndex&) const {
    return m_Events.size();
}


QVariant CoverModel::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    TideEvent ev = m_Events[index.row()];

    if (role == EventDescRole) {
        return ev.description();
    }

    if (role == EventTimestampRole) {
        return ev.time.posix();
    }

    if (role == EventIconRole) {
        return QString("tide-%1").arg(ev.shortname());
    }
    return QVariant();
}

QHash<int, QByteArray> CoverModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[EventDescRole] = "description";
    roles[EventIconRole] = "icon";
    roles[EventTimestampRole] = "timestamp";
    return roles;
}



