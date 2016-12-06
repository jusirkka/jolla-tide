#include <QDebug>
#include "Address.h"

using namespace Tide;


bool Address::operator== (const Address& other) const {
    return factory == other.factory && station == other.station;
}

QString Address::key() const {
    return QString("%1%2%3").arg(factory).arg(separator()).arg(station);
}

QChar Address::separator() {
    return QChar::fromLatin1(30);
}

Address Address::fromKey(const QString &key) {
    QStringList parts = key.split(separator());
    if (parts.length() != 2) {
        return Address();
    }
    return Address(parts[0], parts[1]);
}

Address::AddressList Address::fromKeys(const QStringList& keys) {
    AddressList addresses;
    foreach (QString key, keys) {
        addresses.append(fromKey(key));
    }
    return addresses;
}
