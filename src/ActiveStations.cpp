#include "ActiveStations.h"
#include "StationProvider.h"
#include "Database.h"
#include <QDebug>
#include <QDateTime>

#ifndef NO_POINTSWINDOW
#include "PointsWindow.h"
#endif

Tide::ActiveStations::~ActiveStations() {}


Tide::ActiveStations::ActiveStations(StationProvider* parent):
    QAbstractListModel(parent),
    m_Parent(parent)
{
    Database::ActiveList acs = Database::ActiveStations();
    foreach (Database::Active ac, acs) {
        QString key = ac.address.key();
        m_Marks[key] = Amplitude::parseDottedMeters(ac.mark);
        m_Stations.append(key);
        Data d;
        d.recompute = new QTimer(this);
        d.recompute->setSingleShot(true);
        connect(d.recompute, SIGNAL(timeout()), this, SLOT(computeNextEvent()));
        m_Events[key] = d;
    }
    computeNextEvent();
    connect(m_Parent, SIGNAL(stationChanged(const QString&)), this, SLOT(stationChanged(const QString&)));
    connect(m_Parent, SIGNAL(stationReset()), this, SLOT(computeNextEvent()));
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
        return m_Parent->info(key).attribute("name");
    }

    if (role == LevelRole) {
        const Station& s = m_Parent->station(key);
        return s.predictTideLevel(Timestamp::now()).print();
    }

    if (role == MarkRole) {
        return m_Marks[key].print();
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
    roles[LevelRole] = "level";
    roles[MarkRole] = "mark";
    roles[NextEventDescRole] = "description";
    roles[NextEventIconRole] = "icon";
    roles[NextEventTimestampRole] = "timestamp";
    return roles;
}


void Tide::ActiveStations::append(const QString& station) {
    int row = m_Stations.size();
    beginInsertRows(QModelIndex(), row, row);
    m_Stations.append(station);
    Database::OrderActives(Address::fromKeys(m_Stations));
    Data d;
    d.recompute = new QTimer(this);
    d.recompute->setSingleShot(true);
    connect(d.recompute, SIGNAL(timeout()), this, SLOT(computeNextEvent()));
    m_Events[station] = d;
    computeNextEvent();
    endInsertRows();
    // request update from parent
    m_Parent->stationUpdate();
}

void Tide::ActiveStations::computeNextEvent(bool reset) {
    QHashIterator<QString, Data> ev(m_Events);
    while (ev.hasNext()) {
        ev.next();
        QTimer* r = ev.value().recompute;
        if (r->remainingTime() > 0 && ev.value().next.type != TideEvent::invalid && !reset) {
            continue;
        }
        const Station& s = m_Parent->station(ev.key());
        TideEvent::Organizer org;
        Timestamp now = Timestamp::now();
        Timestamp start = now;
        while (org.isEmpty()) {
            Timestamp then = start + Interval::fromSeconds(3600*6);
            s.predictTideEvents(start, then, org, m_Marks[ev.key()]);
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
        s.predictTideEvents(start, then, org, m_Marks[key]);
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
    m_Marks.remove(station);
    endRemoveRows();
    return true;
}

void Tide::ActiveStations::remove(int row) {
    removeRows(row, 1);
    emit dataChanged(index(0), index(m_Stations.size() - 1));
    Database::OrderActives(Address::fromKeys(m_Stations));
}

void Tide::ActiveStations::movetotop(int row) {
    if (row < 1 || row > m_Stations.size() - 1) return;
    QString station = m_Stations[row];
    Amplitude mark = m_Marks.value(station);
    removeRows(row, 1);
    beginInsertRows(QModelIndex(), 0, 0);
    m_Stations.push_front(station);
    Data d;
    d.recompute = new QTimer(this);
    d.recompute->setSingleShot(true);
    connect(d.recompute, SIGNAL(timeout()), this, SLOT(computeNextEvent()));
    m_Events[station] = d;
    m_Marks[station] = mark;
    computeNextEvent();
    endInsertRows();
    emit dataChanged(index(0), index(m_Stations.size() - 1));
    Database::OrderActives(Address::fromKeys(m_Stations));
}


#ifndef NO_POINTSWINDOW
void Tide::ActiveStations::showpoints(int row) {
    QString key = m_Stations[row];
    const Station& s = m_Parent->station(key);
    PointsWindow* w = new PointsWindow(Address::fromKey(key), s);
    w->resize(1600, 800);
    w->show();
}
#endif

void Tide::ActiveStations::setmark(int row, const QString& mark) {
    QString key = m_Stations[row];
    m_Marks[key] = Amplitude::parseDottedMeters(mark);
    Database::SetMark(Address::fromKey(key), m_Marks[key].print());
    stationChanged(key);
}
