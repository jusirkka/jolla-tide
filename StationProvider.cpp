#include "StationProvider.h"
#include <QDebug>

Tide::StationProvider::~StationProvider() {}


Tide::StationProvider::StationProvider(QList<StationFactory*>& factories, QObject* parent)
    :QAbstractListModel(parent)
{
    QString errMsg;
    int erow;
    int ecol;
    foreach(StationFactory* factory, factories) {
        m_Factories[factory->info().key] = factory;
        QString fkey = factory->info().key;
        const QHash<QString, StationInfo>& stations = factory->available();
        QHash<QString, StationInfo>::const_iterator s = stations.constBegin();
        for (; s != stations.constEnd(); ++s) {
            QDomDocument doc(s.key());
            doc.setContent(s.value().xmlDetail, &errMsg, &erow, &ecol);
            m_AvailableStations[QString("%1%2%3").arg(fkey).arg(QChar(30)).arg(s.key())] = doc;
        }
    }
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

    QDomNode stationNode = m_AvailableStations[key].firstChild();

    if (role == NameRole || role == Qt::DecorationRole) {
        QString name = stationNode.attributes().namedItem("name").nodeValue();
        return QVariant::fromValue(name);
    }

    if (role == DetailRole) {
        QStringList attrs;
        attrs << "county" << "country" << "region";
        QStringList details;
        foreach (QString attr, attrs) {
            QDomNode detail = stationNode.attributes().namedItem(attr);
            if (detail.isNull()) {
                continue;
            }
            details << detail.nodeValue();
        }
        return QVariant::fromValue(details.join(" / "));
    }

    if (role == LocationRole) {
        // Mount Everest +27.5916+086.5640+8850CRSWGS_84/
        QString location = stationNode.attributes().namedItem("location").nodeValue();
        // TODO: transform
        return QVariant::fromValue(location);
    }

    if (role == TypeRole) {
        QString kind = stationNode.namedItem("kind").nodeValue();
        return QVariant::fromValue(kind);
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

QDomNode Tide::StationProvider::info(const QString& key) {
    return m_AvailableStations[key].firstChild();
}

const Tide::Station& Tide::StationProvider::station(const QString& key) {
    QStringList parts = key.split(QChar::fromLatin1(30));
    return m_Factories[parts[0]]->instance(parts[1]);
}


void Tide::StationProvider::setFilter(const QString& fter) {
    if (m_Filter == fter) return;
    m_Filter = fter;

    beginResetModel();

    m_Visible.clear();

    if (m_Filter.length() > 2) {
        QStringList attrs;
        attrs << "name" << "county" << "country" << "region" << "location";

        QHashIterator<QString, QDomDocument> st(m_AvailableStations);
        while (st.hasNext()) {
            st.next();
            QDomNode stationNode = st.value().firstChild();
            foreach (QString attr, attrs) {
                QDomNode detail = stationNode.attributes().namedItem(attr);
                if (detail.isNull()) {
                    continue;
                }
                if (detail.nodeValue().contains(m_Filter, Qt::CaseInsensitive)) {
                    m_Visible.append(st.key());
                    break;
                }
            }
        }
    }
    endResetModel();
    emit filterChanged(m_Filter);
}

QString Tide::StationProvider::filter() const {return m_Filter;}

