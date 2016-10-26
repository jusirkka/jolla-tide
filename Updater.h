#ifndef TIDE_UPDATER_H
#define TIDE_UPDATER_H

#include <QObject>
#include "StationFactory.h"
#include <QTimer>

namespace Tide {

class Updater;

class Address {
public:
    Address(const QString& key = QString()) {
        QStringList parts = key.split(QChar::fromLatin1(30));
        if (parts.length() == 2) {
            factory = parts[0];
            station = parts[1];
        }
    }
    Address(const QString& f, const QString& s): factory(f), station(s) {}

    QString factory;
    QString station;
};

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


bool operator== (const Address& a, const Address& b);



}
#endif
