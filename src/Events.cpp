#include "Events.h"
#include "StationProvider.h"
#include <QDebug>
#include <QMultiMap>
#include <QDateTime>

Tide::Events::~Events() {}


Tide::Events::Events(StationProvider* parent):
    QAbstractListModel(parent),
    m_Parent(parent)
{}



int Tide::Events::rowCount(const QModelIndex&) const {
    return m_Events.size();
}


QVariant Tide::Events::data(const QModelIndex& index, int role) const {

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

QHash<int, QByteArray> Tide::Events::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[EventDescRole] = "description";
    roles[EventIconRole] = "icon";
    roles[EventTimestampRole] = "timestamp";
    return roles;
}


void Tide::Events::forward() {
    int row = m_Events.size();
    beginInsertRows(QModelIndex(), row, row + 9);
    computeEvents(10);
    endInsertRows();
    m_Today = m_Today * (m_Events.size() - 10) / m_Events.size();
    emit todayChanged(m_Today);
}

void Tide::Events::rewind() {
    beginInsertRows(QModelIndex(), 0, 9);
    computeEvents(-10);
    endInsertRows();
    m_Today = (m_Today * (m_Events.size() - 10) + 10) / m_Events.size();
    emit todayChanged(m_Today);
    m_Delta = 10.0 / m_Events.size();
    emit deltaChanged(m_Delta);
}


void Tide::Events::init(const QString& key, const QString& mark) {
    qDebug() << "init" << key;
    beginResetModel();
    m_Events.clear();
    m_Station = key;
    m_Mark = Amplitude::parseDottedMeters(mark);
    computeEvents(-10);
    computeEvents(10);
    qDebug() << "init" << m_Events.size();
    m_Today = 0.5;
    m_Delta = 0.0;
    endResetModel();
}

void Tide::Events::computeEvents(int cnt) {
    Timestamp start;
    int mul = cnt < 0 ? -1 : 1;
    if (m_Events.isEmpty()) {
        start = Timestamp::now();
    } else if (cnt < 0) {
        start = m_Events.first().time;
    } else {
        start = m_Events.last().time;
    }
    const Station& s = m_Parent->station(m_Station);
    TideEvent::Organizer org;
    while (org.size() < mul*cnt) {
        Timestamp then = start + mul * Interval::fromSeconds(3600*24);
        if (mul < 0) {
            s.predictTideEvents(then, start, org, m_Mark);
        } else {
            s.predictTideEvents(start, then, org, m_Mark);
        }
        start = then;
    }

    QMapIterator<Timestamp, TideEvent> ev(org);
    if (cnt < 0) {
        ev.toBack();
        while (ev.hasPrevious() && cnt < 0) {
            ev.previous();
            cnt++;
            m_Events.push_front(ev.value());
        }
    } else {
        while (ev.hasNext() && cnt > 0) {
            ev.next();
            cnt--;
            m_Events.push_back(ev.value());
        }
    }
}
