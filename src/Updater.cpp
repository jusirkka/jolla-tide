#include "Updater.h"
#include "Database.h"
#include <QDebug>

using namespace Tide;


UpdaterProxy::UpdaterProxy(Updater* parent, const Address& addr):
    m_Parent(parent),
    m_Addr(addr)
{}

void UpdaterProxy::whenFinished(const Status& s) {
    qDebug() << "Tide::UpdaterProxy::whenFinished" << s.code << s.detail;
    m_Parent->updated(m_Addr, s);
}

ClientProxy* UpdaterProxy::clone() {
    return new UpdaterProxy(m_Parent, m_Addr);
}



Updater::~Updater() {}


Updater::Updater(const QList<StationFactory*>& factories, QObject* parent):
    QObject(parent),
    m_State(IDLE),
    m_QueuedRequest(false),
    m_EmitReady(false)
{
    foreach (StationFactory* factory, factories) {
        QString fkey = factory->info().key;
        m_Factories[fkey] = factory;
        if (factory->available().isEmpty()) {
            factory->updateAvailable(new UpdaterProxy(this, Address(fkey)));
        }
    }
    m_Long = new QTimer(this);
    m_Long->setInterval(1000*3600*6);
    connect(m_Long, SIGNAL(timeout()), this, SLOT(sync()));
    m_Long->start();
    m_Short = new QTimer(this);
    m_Short->setInterval(100);
    m_Short->setSingleShot(true);
    connect(m_Short, SIGNAL(timeout()), this, SLOT(sync()));
}

void Updater::sync() {
    qDebug() << "Tide::Updater::sync";
    if (m_State == UPDATING) {
        m_QueuedRequest = true;
        return;
    }
    // IDLE -> UPDATING
    m_State = UPDATING;
    m_EmitReady = false;
    m_QueuedRequest = false;
    m_Pending.clear();
    Database::ActiveList actives = Database::ActiveStations();
    foreach (Database::Active ac, actives) {
        if (m_Factories[ac.address.factory]->updateNeeded(ac.address.station)) {
            m_Pending.append(ac.address);
        }
    }

    if (m_Pending.isEmpty()) {
        // all uptodate
        // UPDATING -> IDLE
        m_State = IDLE;
        return;
    }

    foreach (Address addr, m_Pending) {
        m_Factories[addr.factory]->update(addr.station, new UpdaterProxy(this, addr));
    }
}

void Updater::updated(const Address &address, const Status &status) {
    if (!m_Pending.contains(address)) return;
    m_Pending.removeAll(address);
    if (status.code == Status::SUCCESS) {
        m_EmitReady = true;
        // this computes new constituentset (webfactory)
        const Station& st = m_Factories[address.factory]->instance(address.station);
        if (st.isvalid()) {
            qDebug() << st.name() << "is ready";
        }
    }

    if (m_Pending.isEmpty()) {
        // UPDATING -> IDLE
        m_State = IDLE;
        if (m_EmitReady) {
            emit ready();
        }
        if (m_QueuedRequest) {
            m_Short->start();
        }
    }

}

