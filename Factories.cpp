#include "Factories.h"
#include <QDebug>

using namespace Tide;


FactoryProxy::FactoryProxy(Factories* parent, const QString& key):
    m_Parent(parent), m_Key(key) {}

void FactoryProxy::whenFinished(const Status& s) {
    qDebug() << "FactoryProxy::whenFinished " << s.code << s.xmlDetail;
    if (s.code == Status::SUCCESS) {
        emit m_Parent->availableChanged(m_Key);
    }
}

ClientProxy* FactoryProxy::clone() {
    return new FactoryProxy(m_Parent, m_Key);
}


Factories::~Factories() {}


Factories::Factories(const QList<StationFactory*>& factories, QObject* parent):
    QAbstractListModel(parent),
    m_Factories(factories)
{
    QString errMsg;
    int erow;
    int ecol;
    foreach (StationFactory* f, m_Factories) {
        QString key = f->info().key;
        QDomDocument doc(key);
        doc.setContent(f->info().xmlDetail, &errMsg, &erow, &ecol);
        if (!errMsg.isEmpty()) qDebug() << errMsg << erow << ecol;
        m_Info[key] = doc;
    }
}


int Factories::rowCount(const QModelIndex&) const {
    return m_Factories.size();
}


QVariant Factories::data(const QModelIndex& index, int role) const {

    if (!index.isValid()) {
        return QVariant();
    }

    StationFactory* factory = m_Factories[index.row()];

    QDomNode node = m_Info[factory->info().key].firstChild();

    if (role == NameRole || role == Qt::DecorationRole) {
        QString name = node.attributes().namedItem("name").nodeValue();
        return QVariant::fromValue(name);
    }


    if (role == AboutRole) {
        return QVariant::fromValue(node.attributes().namedItem("about").nodeValue());
    }

    if (role == HomePageRole) {
        return QVariant::fromValue(node.attributes().namedItem("home").nodeValue());
    }


    return QVariant();
}

QHash<int, QByteArray> Factories::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[AboutRole] = "about";
    roles[HomePageRole] = "home";
    return roles;
}

StationFactory* Factories::instance(int row) {
    if (row < 0 || row >= m_Factories.size()) return 0;
    return m_Factories[row];
}

StationFactory* Factories::instance(const QString& key) {
    foreach (StationFactory* f, m_Factories) {
        if (f->info().key == key) return f;
    }
    return 0;
}

void Factories::update(int row) {
    if (row < 0 || row >= m_Factories.size()) return;
    QString factory = m_Factories[row]->info().key;
    m_Factories[row]->updateAvailable(new FactoryProxy(this, factory));
}
