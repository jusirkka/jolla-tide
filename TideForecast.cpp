#include "TideForecast.h"
#include <QDomDocument>
#include <QRegularExpression>
#include <QNetworkReply>
#include <libxml/xpath.h>
#include <libxml/HTMLparser.h>

using namespace Tide;

TideForecast::TideForecast(): WebFactory("tfc",
                                         "tide-forecast",
                                         "tide-tide-forecast",
                                         "Tide Times &amp; Time Charts for the World",
                                         "http://www.tide-forecast.com")
{
    m_BaseUrl = m_Doc.firstChild().attributes().namedItem("home").nodeValue();
    m_AvailUrl = QString("%1/locations/nav").arg(m_BaseUrl);
}


QString TideForecast::stationUrl(const QString& key) {
    // http://www.tide-forecast.com/tides/Saint-Peter-Port.js
    QString url = QString("%1/tides/%2.js").arg(m_BaseUrl).arg(key);
    m_KnownStations[url] = key;
    return url;
}

QString TideForecast::availUrl() {
    return m_AvailUrl;
}

QString TideForecast::countryUrl(int country_id) {
    // http://www.tide-forecast.com/locations/nav?country_id=8
    return QString("%1?country_id=%2").arg(m_AvailUrl).arg(country_id);
}

void TideForecast::handleDownloaded(ClientProxy* client, QNetworkReply* reply) {
    QString req = reply->request().url().toString();
    qDebug() << req;
    if (m_KnownStations.contains(req)) {
        handleStationUpdate(m_KnownStations[req], client, reply);
    } else if (req == availUrl()) {
        handleFirstAvailPage(client, reply);
    } else if (m_PendingCountries.contains(req)) {
        handleSecondAvailPage(req, client, reply);
    } else {
        Status s(Status::ERROR, QString("<error status='%1: unknown request'/>").arg(req));
        client->whenFinished(s);
        delete client;
        return;
    }
}

void TideForecast::handleStationUpdate(const QString &key, ClientProxy *client, QNetworkReply* reply) {
    Timestamp stamp;
    Interval step;
    QVector<Amplitude> points;
    Timestamp expected_stamp;
    Timestamp latest = Timestamp::fromPosixTime(0);
    if (m_LastDataPoint.contains(key)) {
        latest = m_LastDataPoint[key];
    }
    QRegularExpression re("\\[\\d+,\\d+,(\\d+),([+-]?\\d+\\.?\\d*)\\]");
    char buf[512];
    qint64 lineLength = reply->readLine(buf, sizeof(buf));
    while (lineLength > 0) {
        QRegularExpressionMatch m = re.match(buf);
        lineLength = reply->readLine(buf, sizeof(buf));
        if (!m.hasMatch()) {
            continue;
        }
        Timestamp t = Timestamp::fromPosixTime(m.captured(1).toInt());
        if (t < latest) {
            // qDebug() << "Overlapping data point, skipping";
            continue;
        }
        points.append(Amplitude::fromDottedMeters(m.captured(2).toDouble(), 0));
        if (stamp.isnil()) {
            stamp = t;
            continue;
        }
        if (step.isnil()) {
            step = t - stamp;
            expected_stamp = stamp + 2*step;
            continue;
        }
        if (t != expected_stamp) {
            Status s(Status::ERROR, QString("<error status='%1: inconsistent level data'/>").arg(key));
            client->whenFinished(s);
            delete client;
            return;
        }
        expected_stamp = t + step;
    }
    if (step.isnil()) {
        Status s(Status::ERROR, QString("<error status='%1: no level data'/>").arg(key));
        client->whenFinished(s);
        delete client;
        return;
    }
    storeStation(key, client, points, stamp, step);
}

void TideForecast::handleFirstAvailPage(ClientProxy* client, QNetworkReply* reply) {
    // FIXME: remove after testing
    QList<int> testing;
    testing << 1 << 8 << 194;
    QByteArray page = reply->readAll();
    qDebug() << page.size();
    xmlDocPtr doc = htmlReadMemory(page.constData(), page.size(), "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *) "//select[@id='country_id']/option[@value]", xpathCtx);
    if (!xpathObj->nodesetval) {
        Status s(Status::ERROR, QString("<error status='no countries found'/>"));
        client->whenFinished(s);
        delete client;
        return;
    }
    m_Available.clear();
    for (int i = 0; i< xpathObj->nodesetval->nodeNr; ++i) {
        int country_id = QString((const char*) xmlGetProp(xpathObj->nodesetval->nodeTab[i], (const xmlChar *) "value")).toInt();
        if (!testing.contains(country_id)) {
            continue;
        }
        QString url = countryUrl(country_id);
        QString name = QString((const char*) xmlNodeGetContent(xpathObj->nodesetval->nodeTab[i]));
        m_Pending[url] = client->clone();
        m_PendingCountries[url] = name;
        m_DLManager->get(QNetworkRequest(url));
    }

    Status s(Status::PENDING, QString("<ok/>"));
    client->whenFinished(s);
    delete client;
}


void TideForecast::handleSecondAvailPage(const QString& url, ClientProxy* client, QNetworkReply* reply) {
    QString country = m_PendingCountries[url];
    m_PendingCountries.remove(url);
    QByteArray page = reply->readAll();
    // TODO: mem mgmt
    xmlDocPtr doc = htmlReadMemory(page.constData(), page.size(), "", NULL, HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR);
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *) "//select[@id='location_filename_part']/option[@value]", xpathCtx);
    if (!xpathObj->nodesetval) {
        QString err("<error status='no stations found'/>"); // TODO: better error info
        Status s = m_PendingCountries.isEmpty() ? Status(Status::SUCCESS, err) : Status(Status::PENDING, err);
        client->whenFinished(s);
        delete client;
        return;
    }
    QHash<QString, QString> info;
    for (int i = 0; i< xpathObj->nodesetval->nodeNr; ++i) {
        QString key = QString((const char*) xmlGetProp(xpathObj->nodesetval->nodeTab[i], (const xmlChar *) "value"));
        QString name = QString((const char*) xmlNodeGetContent(xpathObj->nodesetval->nodeTab[i]));
        QString xml = QString("<station type='Running' country='%1' name='%2' />").arg(country).arg(name);
        info[key] = xml;
    }
    storeAvail(client, info, m_PendingCountries.isEmpty());
}
