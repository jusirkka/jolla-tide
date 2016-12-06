#ifndef TIDE_ADDRESS_H
#define TIDE_ADDRESS_H

#include <QString>
#include <QList>

namespace Tide {


class Address {
public:

    Address(const Address& a): factory(a.factory), station(a.station) {}
    Address& operator=(const Address& a) {factory = a.factory; station = a.station; return *this;}
    Address(const QString& f = QString(), const QString& s = QString()): factory(f), station(s) {}

    QString key() const;

    ~Address() {}

    typedef QList<Address> AddressList;

    static QChar separator();
    static Address fromKey(const QString& key);
    static AddressList fromKeys(const QStringList& keys);

    bool operator==(const Address &other) const;

    QString factory;
    QString station;
};

inline uint qHash(const Address& addr, uint seed = 0) {
    return qHash(addr.key(), seed);
}

}



#endif
