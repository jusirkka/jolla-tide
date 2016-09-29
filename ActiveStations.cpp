#include "ActiveStations.h"
#include "StationProvider.h"
#include "Database.h"
#include "PointsWindow.h"
#include <QDebug>
#include <QDateTime>

Tide::ActiveStations::~ActiveStations() {}


Tide::ActiveStations::ActiveStations(StationProvider* parent):
    QAbstractListModel(parent),
    m_Parent(parent)
{
    QStringList actives = Database::ActiveStations();
    foreach (QString saved, actives) {
        append(saved);
    }
    connect(m_Parent, SIGNAL(stationChanged(const QString&)), this, SLOT(stationChanged(const QString&)));

}



int Tide::ActiveStations::rowCount(const QModelIndex&) const {
    return m_Stations.size();
}


QVariant Tide::ActiveStations::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    QString key = m_Stations[index.row()];


    if (role == KeyRole) {
        return key;
    }

    if (role == NameRole) {
        QDomNode stationInfo = m_Parent->info(key);
        return stationInfo.attributes().namedItem("name").nodeValue();
    }

    TideEvent ev = m_Events[key].next;
    if (role == NextEventDescRole) {
        return ev.description();
    }
    if (role == NextEventTimestampRole) {
        return ev.time.posix();
    }

    if (role == NextEventIconRole) {
        return QString("tide-%1").arg(ev.shortname());
    }
    return QVariant();
}

QHash<int, QByteArray> Tide::ActiveStations::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[KeyRole] = "key";
    roles[NextEventDescRole] = "description";
    roles[NextEventIconRole] = "icon";
    roles[NextEventTimestampRole] = "timestamp";
    return roles;
}


void Tide::ActiveStations::append(const QString& station) {
    Database::Activate(station);
    int row = m_Stations.size();
    beginInsertRows(QModelIndex(), row, row);
    m_Stations.append(station);
    Data d;
    d.recompute = new QTimer(this);
    d.recompute->setSingleShot(true);
    connect(d.recompute, SIGNAL(timeout()), this, SLOT(computeNextEvent()));
    m_Events[station] = d;
    computeNextEvent();
    endInsertRows();
}

void Tide::ActiveStations::computeNextEvent() {
    QHashIterator<QString, Data> ev(m_Events);
    while (ev.hasNext()) {
        ev.next();
        QTimer* r = ev.value().recompute;
        if (r->isActive() && ev.value().next.type != TideEvent::invalid) {
            continue;
        }
        const Station& s = m_Parent->station(ev.key());
        TideEvent::Organizer org;
        Timestamp now = Timestamp::now();
        Timestamp start = now;
        while (org.isEmpty()) {
            Timestamp then = start + Interval::fromSeconds(3600*6);
            s.predictTideEvents(start, then, org);
            start = then;
        }
        TideEvent event = org.first();
        m_Events[ev.key()].next = event;
        Interval tmout = event.time - now;
        r->start(tmout.seconds * 1000);
        QModelIndex c = index(m_Stations.indexOf(ev.key()));
        emit dataChanged(c, c);
    }
}

void Tide::ActiveStations::stationChanged(const QString& key) {
    if (!m_Events.contains(key)) return;

    QTimer* r = m_Events[key].recompute;
    const Station& s = m_Parent->station(key);
    TideEvent::Organizer org;
    Timestamp now = Timestamp::now();
    Timestamp start = now;
    while (org.isEmpty()) {
        Timestamp then = start + Interval::fromSeconds(3600*6);
        s.predictTideEvents(start, then, org);
        start = then;
    }
    TideEvent event = org.first();
    m_Events[key].next = event;
    Interval tmout = event.time - now;
    r->start(tmout.seconds * 1000);
    QModelIndex c = index(m_Stations.indexOf(key));
    emit dataChanged(c, c);
}


bool Tide::ActiveStations::removeRows(int row, int count, const QModelIndex& parent) {
    if (count != 1) return false;
    if (row < 0 || row > m_Stations.size() - 1) return false;
    beginRemoveRows(parent, row, row);
    QString station = m_Stations[row];
    m_Stations.removeAt(row);
    delete m_Events[station].recompute;
    m_Events.remove(station);
    endRemoveRows();
    return true;
}

void Tide::ActiveStations::remove(int row) {
    Database::Deactivate(m_Stations[row]);
    removeRows(row, 1);
}

void Tide::ActiveStations::movetotop(int row) {
    if (row < 1 || row > m_Stations.size() - 1) return;
    QString station = m_Stations[row];
    removeRows(row, 1);
    beginInsertRows(QModelIndex(), 0, 0);
    m_Stations.push_front(station);
    Data d;
    d.recompute = new QTimer(this);
    d.recompute->setSingleShot(true);
    connect(d.recompute, SIGNAL(timeout()), this, SLOT(computeNextEvent()));
    m_Events[station] = d;
    computeNextEvent();
    endInsertRows();
    emit dataChanged(index(0), index(m_Stations.size() - 1));
}


void Tide::ActiveStations::showpoints(int row) {
    QString key = m_Stations[row];
    const Station& s = m_Parent->station(key);
    PointsWindow* w = new PointsWindow(key, s);
    w->resize(800, 200);
    w->show();
}

