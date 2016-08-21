#ifndef STATION_FACTORY_H
#define STATION_FACTORY_H

#include <QtPlugin>
#include <QString>
#include <QList>

#include "Station.h"

namespace Tide {

class Status {

public:

    enum Code {NOOP, ERROR, SUCCESS};

    Status(Code c, const QString& d): code(c), xmlDetail(d) {}


    Code code;
    QString xmlDetail;
};


class StationInfo {

public:

    StationInfo(const QString& k, const QString& d): key(k), xmlDetail(d) {}
    StationInfo() {}

    QString key;
    QString xmlDetail;
};


// alias
typedef StationInfo StationFactoryInfo;

class StationFactory {

public:

    StationFactory() {}

    virtual const StationFactoryInfo& info() = 0;
    virtual const QHash<QString, StationInfo>& available() = 0;
    virtual const Station& instance(const QString& key) = 0;
    virtual const Status& update(const QString& key) = 0;
    virtual const Status& updateAvailable() = 0;

    virtual ~StationFactory() {}


};

} // namespace Tide

Q_DECLARE_INTERFACE(Tide::StationFactory, "net.kvanttiapina.tide.StationFactory/1.0")


#endif // STATION_FACTORY_H
