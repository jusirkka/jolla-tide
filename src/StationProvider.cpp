#include "StationProvider.h"
#include "Address.h"
#include <QDebug>

using namespace Tide;

StationUpdateHandler::StationUpdateHandler(StationProvider* parent, const QString& key):
    m_Parent(parent),
    m_Key(key)
{}

void StationUpdateHandler::whenFinished(const Status& s) {
    qDebug() << "Tide::StationUpdateHandler::whenFinished" << s.code << s.detail;
    if (s.code == Status::SUCCESS) {
        QModelIndex c = m_Parent->index(m_Parent->m_Visible.indexOf(m_Key));
        if (c.isValid()) {
            qDebug() << "Tide::StationUpdateHandler: data changed";
            emit m_Parent->dataChanged(c, c);
        } else {
            qDebug() << "Tide::StationUpdateHandler: not visible";
        }
        emit m_Parent->stationChanged(m_Key);
    }
}

ClientProxy* StationUpdateHandler::clone() {
    return new StationUpdateHandler(m_Parent, m_Key);
}



StationProvider::~StationProvider() {}


StationProvider::StationProvider(Factories* factories, QObject* parent):
    QAbstractListModel(parent),
    m_Factories(factories),
    m_Invalid()
{
    connect(m_Factories, SIGNAL(availableChanged(const QString&)), this, SLOT(resetVisible(const QString&)));

    m_Updater = new Update::Manager("net.kvanttiapina.tide", "/Updater", QDBusConnection::sessionBus(), this);
    connect(m_Updater, SIGNAL(ready()), this, SLOT(stationUpdateReady()));

    for (int row = 0; row < m_Factories->rowCount(QModelIndex()); ++row) {
        StationFactory* factory = m_Factories->instance(row);
        QString fkey = factory->info().key;
        const QHash<QString, StationInfo>& stations = factory->available();
        if (stations.isEmpty()) {
            m_Factories->update(row);
            continue;
        }
    }
}


void StationProvider::resetVisible(const QString&) {
    QString filter = m_Filter;
    m_Filter = ""; // enforce reset
    setFilter(filter);
}

int StationProvider::rowCount(const QModelIndex&) const {
    return m_Visible.size();
}


QVariant StationProvider::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    QString key = m_Visible[index.row()];

    if (role == KeyRole) {
        return key;
    }

    Address addr = Address::fromKey(key);
    StationFactory* factory = m_Factories->instance(addr.factory);
    QDomElement elem = factory->available()[addr.station].info.documentElement();

    if (role == NameRole || role == Qt::DecorationRole) {
        return elem.attribute("name");
    }

    if (role == DetailRole) {
        QStringList attrs;
        attrs << "county" << "country" << "region";
        QStringList details;
        foreach (QString attr, attrs) {
            QString detail = elem.attribute(attr);
            if (!detail.isEmpty()) {
                details << detail;
            }
        }
        return details.join(" / ");
    }

    if (role == LocationRole) {
        // Mount Everest +27.5916+086.5640+8850CRSWGS_84/
        return Coordinates::parseISO6709(elem.attribute("location")).print();
    }

    if (role == TypeRole) {
        return elem.attribute("type");
    }


    return QVariant();
}

QHash<int, QByteArray> StationProvider::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DetailRole] = "detail";
    roles[LocationRole] = "location";
    roles[TypeRole] = "kind";
    roles[KeyRole] = "key";
    return roles;
}

QDomElement StationProvider::info(const QString& key) {
    Address addr = Address::fromKey(key);
    StationFactory* factory = m_Factories->instance(addr.factory);
    return factory->available()[addr.station].info.documentElement();
}

const Station& StationProvider::station(const QString& key) {
    Address addr = Address::fromKey(key);
    StationFactory* factory = m_Factories->instance(addr.factory);
    if (!factory) return m_Invalid;
    return factory->instance(addr.station);
}


void StationProvider::setFilter(const QString& fter) {
    if (m_Filter == fter) return;
    m_Filter = fter;

    beginResetModel();

    m_Visible.clear();

    QStringList locationUpdate;

    if (m_Filter.length() > 2) {
        QStringList attrs;
        attrs << "name" << "county" << "country" << "region" << "location";


        for (int i = 0; i < m_Factories->rowCount(QModelIndex()); ++i) {
            StationFactory* f = m_Factories->instance(i);
            QHash<QString, StationInfo> stations = f->available();
            QHashIterator<QString, StationInfo> st(stations);

            while (st.hasNext()) {
                st.next();
                QDomElement elem = st.value().info.documentElement();
                foreach (QString attr, attrs) {
                    QString detail = elem.attribute(attr);
                    if (detail.isEmpty()) {
                        continue;
                    }
                    if (detail.contains(m_Filter, Qt::CaseInsensitive)) {
                        QString key = Address(f->info().key, st.key()).key();
                        m_Visible.append(key);
                        QString loc = elem.attribute("location");
                        if (loc.isEmpty()) {
                            locationUpdate.append(key);
                        }
                        break;
                    }
                }
            }
        }
    }

    // check that we have locations
    foreach (QString key, locationUpdate) {
        Address addr = Address::fromKey(key);
        StationFactory* factory = m_Factories->instance(addr.factory);
        factory->updateStationInfo("location", addr.station, new StationUpdateHandler(this, key));
    }

    endResetModel();
    emit filterChanged(m_Filter);
}

QString StationProvider::filter() const {return m_Filter;}


QString StationProvider::name(const QString& key) {
    return info(key).attribute("name");
}

QString StationProvider::location(const QString& key) {
    return Coordinates::parseISO6709(info(key).attribute("location")).print();
}

QString StationProvider::kind(const QString& key) {
    return info(key).attribute("type");
}

QString StationProvider::detail(const QString& key) {
    QStringList attrs;
    attrs << "county" << "country" << "region";
    QStringList details;
    QDomElement elem = info(key);
    foreach (QString attr, attrs) {
        QString detail = elem.attribute(attr);
        if (!detail.isEmpty()) {
            details << detail;
        }
    }
    return details.join(" / ");
}

QString StationProvider::provider(const QString& key) {
    Address addr = Address::fromKey(key);
    StationFactory* factory = m_Factories->instance(addr.factory);
    QDomElement f = factory->info().info.documentElement();
    return f.attribute("name");
}

QString StationProvider::providerlogo(const QString& key) {
    Address addr = Address::fromKey(key);
    StationFactory* factory = m_Factories->instance(addr.factory);
    QDomElement f = factory->info().info.documentElement();
    return f.attribute("logo");
}

void StationProvider::stationUpdate() const {
    m_Updater->sync();
}

void StationProvider::stationUpdateReady() {
    for (int row = 0; row < m_Factories->rowCount(QModelIndex()); ++row) {
        StationFactory* factory = m_Factories->instance(row);
        factory->reset();
    }
    emit stationReset();
}


