#ifndef TIDE_UPDATER_H
#define TIDE_UPDATER_H

#include <QObject>
#include <QTimer>
#include "StationFactory.h"
#include "Address.h"

namespace Tide {

class Updater;


class UpdaterProxy: public ClientProxy {
public:
    UpdaterProxy(Updater* parent, const Address& addr);
    void whenFinished(const Status&);
    ClientProxy* clone();

private:

    Updater* m_Parent;
    Address m_Addr;
};


class Updater: public QObject
{
    Q_OBJECT

public:

    Updater(const QList<StationFactory*>& factories, QObject* parent = 0);

    enum State {IDLE, UPDATING};


    ~Updater();



public slots:

    void sync();


signals:

    void ready();

private:

    void updated(const Address& address, const Status& status);

private:

    QMap<QString, StationFactory*> m_Factories;
    State m_State;
    bool m_QueuedRequest;
    bool m_EmitReady;
    QTimer* m_Long;
    QTimer* m_Short;
    QList<Address> m_Pending;

    friend class UpdaterProxy;

};


}
#endif
