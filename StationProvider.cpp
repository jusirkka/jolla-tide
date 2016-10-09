#include "StationProvider.h"
#include <QDebug>


Tide::StationUpdateHandler::StationUpdateHandler(StationProvider* parent, const QString& key):
    m_Parent(parent),
    m_Key(key)
{}

void Tide::StationUpdateHandler::whenFinished(const Status& s) {
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

Tide::ClientProxy* Tide::StationUpdateHandler::clone() {
    return new StationUpdateHandler(m_Parent, m_Key);
}



Tide::StationProvider::~StationProvider() {}


Tide::StationProvider::StationProvider(Factories* factories, QObject* parent):
    QAbstractListModel(parent),
    m_Factories(factories),
    m_Invalid()
{
    connect(m_Factories, SIGNAL(availableChanged(const QString&)), this, SLOT(resetVisible(const QString&)));

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


void Tide::StationProvider::resetVisible(const QString&) {
    QString filter = m_Filter;
    m_Filter = ""; // enforce reset
    setFilter(filter);
}

int Tide::StationProvider::rowCount(const QModelIndex&) const {
    return m_Visible.size();
}


QVariant Tide::StationProvider::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    QString key = m_Visible[index.row()];

    if (role == KeyRole) {
        return QVariant::fromValue(key);
    }

    QStringList parts = key.split(QChar::fromLatin1(30));
    StationFactory* factory = m_Factories->instance(parts[0]);
    QDomElement elem = factory->available()[parts[1]].info.documentElement();

    if (role == NameRole || role == Qt::DecorationRole) {
        return QVariant::fromValue(elem.attribute("name"));
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
        return QVariant::fromValue(details.join(" / "));
    }

    if (role == LocationRole) {
        // Mount Everest +27.5916+086.5640+8850CRSWGS_84/
        return QVariant::fromValue(Coordinates::parseISO6709(elem.attribute("location")).print());
    }

    if (role == TypeRole) {
        return QVariant::fromValue(elem.attribute("type"));
    }


    return QVariant();
}

QHash<int, QByteArray> Tide::StationProvider::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DetailRole] = "detail";
    roles[LocationRole] = "location";
    roles[TypeRole] = "kind";
    roles[KeyRole] = "key";
    return roles;
}

QDomElement Tide::StationProvider::info(const QString& key) {
    QStringList parts = key.split(QChar::fromLatin1(30));
    StationFactory* factory = m_Factories->instance(parts[0]);
    return factory->available()[parts[1]].info.documentElement();
}

const Tide::Station& Tide::StationProvider::station(const QString& key) {
    QStringList parts = key.split(QChar::fromLatin1(30));
    StationFactory* factory = m_Factories->instance(parts[0]);
    if (!factory) return m_Invalid;
    const Station& s = factory->instance(parts[1]);
    if (factory->updateNeeded(parts[1])) {
        factory->update(parts[1], new StationUpdateHandler(this, key));
    }
    return s;
}


void Tide::StationProvider::setFilter(const QString& fter) {
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
                        QString key = QString("%1%2%3").arg(f->info().key).arg(QChar(30)).arg(st.key());
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
        QStringList parts = key.split(QChar::fromLatin1(30));
        StationFactory* factory = m_Factories->instance(parts[0]);
        factory->updateStationInfo("location", parts[1], new StationUpdateHandler(this, key));
    }

    endResetModel();
    emit filterChanged(m_Filter);
}

QString Tide::StationProvider::filter() const {return m_Filter;}

