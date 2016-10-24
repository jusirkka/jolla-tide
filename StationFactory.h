#ifndef STATION_FACTORY_H
#define STATION_FACTORY_H

#include <QtPlugin>
#include <QString>
#include <QList>
#include <QDomDocument>

#include "Station.h"

namespace Tide {

class Status {

public:

    enum Code {NOOP, ERROR, SUCCESS, PENDING};

    Status(Code c = NOOP, const QString& d = QString()): code(c), detail(d) {}


    Code code;
    QString detail;
};


class StationInfo {

public:

    StationInfo(const QString& k, const QString& xml): key(k) {
        info.setContent(xml);
    }

    StationInfo(const QString& k, const QDomDocument& doc): key(k), info(doc) {}
    StationInfo() {}

    QString key;
    QDomDocument info;
};


// alias
typedef StationInfo StationFactoryInfo;


class ClientProxy
{

public:

    ClientProxy() {}

    virtual void whenFinished(const Status&) = 0;
    virtual ClientProxy* clone() = 0;

    virtual ~ClientProxy() {}
};


class StationFactory {

public:

    StationFactory() {}

    virtual const StationFactoryInfo& info() = 0;
    virtual const QHash<QString, StationInfo>& available() = 0;
    virtual const Station& instance(const QString& station) = 0;
    virtual void update(const QString& station, ClientProxy* client) = 0;
    virtual bool updateNeeded(const QString& station) = 0;
    virtual void updateAvailable(ClientProxy* client) = 0;
    virtual void updateStationInfo(const QString& attr, const QString& station, ClientProxy* client) = 0;
    virtual void reset() = 0;

    virtual ~StationFactory() {}


};

} // namespace Tide

Q_DECLARE_INTERFACE(Tide::StationFactory, "net.kvanttiapina.tide.StationFactory/1.0")


#endif // STATION_FACTORY_H
