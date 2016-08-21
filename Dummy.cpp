#include "Dummy.h"
#include "Year.h"
#include "RunningSet.h"
#include "Database.h"

using namespace Tide;

DummyStations::DummyStations():
    QObject(),
    StationFactory(),
    m_Info("dummy", "<factory name=\"Dummy\"/>"),
    m_Status(Status::NOOP, "<status/>")
{
    QStringList keys;
    keys << "10192830" << "09432037" << "1847928";
    QStringList infos;
    infos << "<station name='Mokio' type='Running'/>" << "<station name='Porto' type='Running'/>" << "<station name='Tokio' type='Running'/>";

    for (int i = 0; i < keys.size(); ++i) {
        m_Available[keys[i]] = StationInfo(keys[i], infos[i]);
        Database::AddStation("dummy", keys[i], infos[i]);
    }

    Amplitude datum = Amplitude::fromDottedMeters(1, 0);
    Timestamp epoch = Timestamp::fromUTCYear(Year::fromAD(2016));
    m_Constituents = new RunningSet(epoch, datum);
    m_Constituents->append(Amplitude::fromDottedMeters(1.5, 0), Speed::fromDegreesPerHour(29.9), Angle::fromDegrees(10));
    m_Constituents->append(Amplitude::fromDottedMeters(3.0, 0), Speed::fromDegreesPerHour(60.3), Angle::fromDegrees(90.3));

    m_Instance = new Station("1", Coordinates::fromWGS84LatLong(-8.0, 35.0), m_Constituents);

}

const StationFactoryInfo& DummyStations::info() {
    return m_Info;
}

const QHash<QString, StationInfo>& DummyStations::available() {
    return m_Available;
}

const Station& DummyStations::instance(const QString&) {
    return *m_Instance;
}

const Status& DummyStations::update(const QString&) {
    return m_Status;
}

const Status& DummyStations::updateAvailable() {
    return m_Status;
}

DummyStations::~DummyStations() {
    delete m_Constituents;
    delete m_Instance;
}

