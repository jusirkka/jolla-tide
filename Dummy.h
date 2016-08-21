#ifndef DUMMY_H
#define DUMMY_H

#include <QObject>
#include <QtPlugin>

#include "StationFactory.h"
#include "RunningSet.h"

namespace Tide {

class DummyStations : public QObject, public StationFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.tide.StationFactory/1.0")
    Q_INTERFACES(Tide::StationFactory)

public:

    DummyStations();

     const StationFactoryInfo& info();
     const QHash<QString, StationInfo>& available();
     const Station& instance(const QString& key);
     const Status& update(const QString& key);
     const Status& updateAvailable();

    ~DummyStations();

private:

     StationFactoryInfo m_Info;
     QHash<QString, StationInfo> m_Available;
     Station* m_Instance;
     RunningSet* m_Constituents;
     Status m_Status;

};

}
#endif
