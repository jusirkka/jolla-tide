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
    m_Info(key, QString("<factory name='%1' logo='%2' about='%3' home='%4'/>")
           .arg(name).arg(logo).arg(desc).arg(url)),
    m_Invalid(),
    m_Doc(key),
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
    Database::Control("create table if not exists constituents ("
                      "id       integer primary key, "
                      "epoch_id integer not null, "
                      "mode_id  integer not null, "
                      "rea real not null, "
                      "ima real not null)");
    Database::Control("create table if not exists modes ("
                      "id    integer primary key, "
                      "name  text not null, "
                      "omega real not null)");

    QString errMsg;
    int erow;
    int ecol;
    // qDebug() << m_Info.xmlDetail;
    m_Doc.setContent(m_Info.xmlDetail, &errMsg, &erow, &ecol);
    if (!errMsg.isEmpty()) {
        qDebug() << "WebFactory: xml parse error" << errMsg << "row = " << erow << "col = " << ecol;
    }

    connect(m_DLManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadReady(QNetworkReply*)));
}

const StationFactoryInfo& WebFactory::info() {
    return m_Info;
}

const QHash<QString, StationInfo>& WebFactory::available() {
    if (m_Available.isEmpty()) {
        QHashIterator<QString, QString> st(Database::AllStations(m_Info.key));
        while (st.hasNext()) {
            st.next();
            m_Available[st.key()] = StationInfo(st.key(), st.value());
        }
    }
    return m_Available;
}

const Station& WebFactory::instance(const QString& key) {
    if (m_Loaded.contains(key)) {
        return *m_Loaded[key];
    }
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    qDebug() << key << m_Info.key;
    r = Database::Query("select id from stations where suid=? and fuid=?", vars);
    if (r.isEmpty()) {
        return m_Invalid;
    }

    int station_id = r.first()[0].toInt();

    RunningSet* rset = HarmonicsCreator::CreateConstituents(station_id);
    if (!rset) {
        return m_Invalid;
    }

    m_Loaded[key] = new Station(rset);
    m_LastDataPoint[key] = HarmonicsCreator::LastDataPoint(station_id);

    return *m_Loaded[key];

}


bool WebFactory::updateNeeded(const QString& key) {
    if (m_LastDataPoint.contains(key) && m_LastDataPoint[key] > Timestamp::now()) {
        // qDebug() << "updateNeeded: last = " << m_LastDataPoint[key].posix();
        return false;
    }

    if (!m_Available.contains(key)) {
        // qDebug() << "updateNeeded: not available" << key;
        return false; // or true?
    }

    QString req = stationUrl(key);
    if (m_Pending.contains(req)) {
        // qDebug() << "updateNeeded: already active" << key;
        return false;
    }

    qDebug() << "updateNeeded: updating" << key;
    return true;
}


void WebFactory::update(const QString& key, ClientProxy* client) {
    if (m_LastDataPoint.contains(key) && m_LastDataPoint[key] > Timestamp::now()) {
        Status s;
        client->whenFinished(s);
        delete client;
        return;
    }
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
        qDebug() << "FATAL: request " << req << "not found in pending reuests!";
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

    qDebug() << "WebFactory::storeStation start";
    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << QVariant::fromValue(key) << QVariant::fromValue(m_Info.key);
    qDebug() << key << m_Info.key;
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

    qDebug() << "WebFactory::storeStation inserting start";
    int epoch_id = r.first()[0].toInt();
    Database::Transaction();
    for (int i = 0; i < points.size(); ++i) {
        vars.clear();
        vars << QVariant::fromValue(epoch_id) << QVariant::fromValue(points[i].value);
        Database::Control("insert into readings (epoch_id, reading) values (?, ?)", vars);
    }
    Database::Commit();
    qDebug() << "WebFactory::storeStation inserting end";


    qDebug() << "WebFactory::storeStation updatedb start";
    HarmonicsCreator::UpdateDB(station_id);
    qDebug() << "WebFactory::storeStation updatedb end";
    Status s(Status::SUCCESS, QString("<ok/>"));
    client->whenFinished(s);
    delete client;
    // enforce new station instance
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
        Database::UpdateStationInfo(m_Info.key, st.key(), st.value());
    }
    Database::Commit();
    Status s = last ? Status(Status::SUCCESS, QString("<ok/>")) : Status(Status::PENDING, QString("<ok/>"));
    client->whenFinished(s);
    delete client;
}

WebFactory::~WebFactory() {}

