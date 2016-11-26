#include "WebFactory.h"
#include "Database.h"
#include "RunningSet.h"
#include "HarmonicsCreator.h"
#include <QVector>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

using namespace Tide;


WebFactory::WebFactory(const QString& key, const QString& name, const QString& logo,
                       const QString& desc, const QString& url):
    QObject(),
    m_Invalid(),
    m_DLManager(new QNetworkAccessManager(this))
{
    Database::Control("create table if not exists epochs ("
                      "id         integer primary key, "
                      "station_id integer not null, "
                      "start      integer not null, "
                      "timedelta  integer not null, "
                      "patchsize  integer not null)");
    Database::Control("create table if not exists readings ("
                      "id       integer primary key, "
                      "epoch_id integer not null, "
                      "reading  real not null)");
    Database::Control("create table if not exists locations ("
                      "id         integer primary key, "
                      "station_id integer not null, "
                      "location   text not null)");

    QDomDocument doc(name);
    doc.setContent(QString("<factory/>"));
    QDomElement f = doc.documentElement();
    f.setAttribute("name", name);
    f.setAttribute("logo", logo);
    f.setAttribute("about", desc);
    f.setAttribute("home", url);
    m_Info = StationFactoryInfo(key, doc);

    connect(m_DLManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadReady(QNetworkReply*)));
}

const StationFactoryInfo& WebFactory::info() {
    return m_Info;
}

const QHash<QString, StationInfo>& WebFactory::available() {
    if (m_Available.isEmpty()) {
        QHashIterator<Address, QString> st(Database::AllStations(m_Info.key));
        while (st.hasNext()) {
            st.next();
            m_Available[st.key().station] = StationInfo(st.key().station, st.value());
        }
    }
    return m_Available;
}

const Station& WebFactory::instance(const QString& key) {
    if (m_Loaded.contains(key)) {
        return *m_Loaded[key];
    }

    if (!m_Available.contains(key)) {
        return m_Invalid;
    }

    QDomDocument doc = m_Available[key].info;

    QString name = doc.documentElement().attribute("name");
    QString loc = doc.documentElement().attribute("location", "N/A");
    qDebug() << name << loc;

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    // qDebug() << key << m_Info.key;
    r = Database::Query("select id from stations where suid=? and fuid=?", vars);
    if (r.isEmpty()) {
        return m_Invalid;
    }

    int station_id = r.first()[0].toInt();

    RunningSet* rset = HarmonicsCreator::CreateConstituents(station_id);
    if (!rset) {
        return m_Invalid;
    }

    m_Loaded[key] = new Station(rset, name, Coordinates::parseISO6709(loc));
    PatchIterator patches(station_id);
    m_LastDataPoint[key] = patches.lastDataPoint();

    return *m_Loaded[key];

}


bool WebFactory::updateNeeded(const QString& key) {
    if (!m_Available.contains(key)) {
        // qDebug() << "updateNeeded false: not available" << key;
        return false;
    }
    if (!instance(key).isvalid()) {
        qDebug() << "updateNeeded true: not active" << key;
        return true;
    }

    if (m_LastDataPoint.contains(key) && m_LastDataPoint[key] > Timestamp::now() + Interval::fromSeconds(2*24*3600)) {
        qDebug() << "updateNeeded false:" << key << "valid until" << m_LastDataPoint[key].print();
        return false;
    }


    QString req = stationUrl(key);
    if (m_Pending.contains(req)) {
        qDebug() << "updateNeeded false: update of" << key << "already pending";
        return false;
    }

    qDebug() << "updateNeeded true: updating" << key;
    return true;
}




void WebFactory::update(const QString& key, ClientProxy* client) {
    if (!m_Available.contains(key)) {
        Status s(Status::ERROR, "<error status='Not available'/>");
        client->whenFinished(s);
        delete client;
        return;
    }

    QString req = stationUrl(key);
    if (m_Pending.contains(req)) {
        Status s(Status::PENDING, "<ok status='started'/>");
        client->whenFinished(s);
        delete client;
        return;
    }

    m_Pending[req] = client;

    m_DLManager->get(QNetworkRequest(req));
}

void WebFactory::updateStationInfo(const QString& attr, const QString& key, ClientProxy* client) {
    if (!m_Available.contains(key)) {
        Status s(Status::ERROR, "<error status='Not available'/>");
        client->whenFinished(s);
        delete client;
        return;
    }
    if (attr != "location") {
        Status s(Status::ERROR, "<error status='Unsupported attribute'/>");
        client->whenFinished(s);
        delete client;
        return;
    }
    // check database
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    r = Database::Query("select l.location from locations l join stations s on l.station_id=s.id where s.suid=? and s.fuid=?", vars);
    if (!r.isEmpty()) {
        QString loc = r.first()[0].toString();
        m_Available[key].info.documentElement().setAttribute("location", loc);
        Status s(Status::SUCCESS, "<ok/>");
        client->whenFinished(s);
        delete client;
        return;
    }
    QString req = locationUrl(key);
    if (m_Pending.contains(req)) {
        Status s(Status::PENDING, "<ok status='started'/>");
        client->whenFinished(s);
        delete client;
        return;
    }
    m_Pending[req] = client;

    m_DLManager->get(QNetworkRequest(req));
}


void WebFactory::reset() {
    QHashIterator<QString, Station*> st(m_Loaded);
    while (st.hasNext()) {
        st.next();
        delete st.value();
    }

    m_Loaded.clear();
    m_LastDataPoint.clear();
}

void WebFactory::updateAvailable(ClientProxy* client) {
    QString req = availUrl();

    if (m_Pending.contains(req)) {
        Status s(Status::PENDING, "<ok status='started'/>");
        client->whenFinished(s);
        delete client;
        return;
    }

    m_Pending[req] = client;

    m_DLManager->get(QNetworkRequest(req));
}

void WebFactory::downloadReady(QNetworkReply* reply) {
    QString req = reply->request().url().toString();
    if (!m_Pending.contains(req)) {
        qDebug() << "FATAL: request " << req << "not found in pending requests!";
        reply->deleteLater();
        return;
    }
    ClientProxy* client = m_Pending[req];
    m_Pending.remove(req);
    if (reply->error()) {
        Status s(Status::ERROR, QString("<error status='%1'/>").arg(reply->errorString()));
        client->whenFinished(s);
        delete client;
        return;
    }
    handleDownloaded(client, reply);
    reply->deleteLater();
}

void WebFactory::storeStation(const QString& key, ClientProxy* client, const QVector<Amplitude>& points, const Timestamp& epoch, const Interval& step) {

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    r = Database::Query("select id from stations where suid=? and fuid=?", vars);
    if (r.isEmpty()) {
        Status s(Status::ERROR, QString("<error reason='%1 not found'/>").arg(key));
        client->whenFinished(s);
        delete client;
        return;
    }
    int station_id = r.first()[0].toInt();

    vars.clear();
    vars << QVariant::fromValue(station_id) << QVariant::fromValue(epoch.posix());
    r = Database::Query("select id from epochs where station_id=? and start=?", vars);
    if (r.size()) {
        Status s(Status::ERROR, QString("<error reason='Epoch already present'/>"));
        client->whenFinished(s);
        delete client;
        return;
    }

    vars.clear();
    vars << QVariant::fromValue(station_id) << QVariant::fromValue(epoch.posix()) <<
            QVariant::fromValue(step.seconds) << QVariant::fromValue(points.size());
    Database::Control("insert into epochs (station_id, start, timedelta, patchsize) values (?, ?, ?, ?)", vars);

    vars.clear();
    vars << QVariant::fromValue(station_id) << QVariant::fromValue(epoch.posix());
    r = Database::Query("select id from epochs where station_id=? and start=?", vars);
    if (r.size() != 1) {
        Status s(Status::ERROR, QString("<error reason='Database error'/>"));
        client->whenFinished(s);
        delete client;
        return;
    }

    int epoch_id = r.first()[0].toInt();
    Database::Transaction();
    for (int i = 0; i < points.size(); ++i) {
        vars.clear();
        vars << QVariant::fromValue(epoch_id) << QVariant::fromValue(points[i].value);
        Database::Control("insert into readings (epoch_id, reading) values (?, ?)", vars);
    }
    Database::Commit();


    Status s(Status::SUCCESS, QString("<ok/>"));
    client->whenFinished(s);
    delete client;
    // enforce new station instance
    HarmonicsCreator::Delete(station_id);
    if (m_Loaded.contains(key)) {
        delete m_Loaded[key];
        m_Loaded.remove(key);
        m_LastDataPoint.remove(key);
    }
}


void WebFactory::storeAvail(ClientProxy* client, const QHash<QString, QString>& info, bool last) {
    QHashIterator<QString, QString> st(info);
    Database::Transaction();
    while (st.hasNext()) {
        st.next();
        Database::UpdateStationInfo(Address(m_Info.key, st.key()), st.value());
    }
    Database::Commit();
    Status s = last ? Status(Status::SUCCESS, QString("<ok/>")) : Status(Status::PENDING, QString("<ok/>"));
    client->whenFinished(s);
    delete client;
}

void WebFactory::storeLocation(const QString& key, ClientProxy* client, const QString& location) {
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    r = Database::Query("select id from stations where suid=? and fuid=?", vars);
    if (r.isEmpty()) {
        Status s(Status::ERROR, QString("<error reason='%1 not found'/>").arg(key));
        client->whenFinished(s);
        delete client;
        return;
    }
    int station_id = r.first()[0].toInt();

    vars.clear();
    vars << QVariant::fromValue(station_id);
    r = Database::Query("select id from locations where station_id=?", vars);
    vars.clear();
    vars << QVariant::fromValue(location) << QVariant::fromValue(station_id);
    if (r.isEmpty()) {
        Database::Control("insert into locations (location, station_id) values (?, ?)", vars);
    } else {
        Database::Control("update locations set location=? where station_id=?", vars);
    }

    m_Available[key].info.documentElement().setAttribute("location", location);
    vars.clear();
    vars << QVariant::fromValue(m_Available[key].info.toString()) << QVariant::fromValue(station_id);
    Database::Control("update stations set xmlinfo=? where id=?", vars);

    Status s(Status::SUCCESS, "<ok/>");
    client->whenFinished(s);
    delete client;
    return;
}


WebFactory::~WebFactory() {}

