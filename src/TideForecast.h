#ifndef TIDE_FORECAST_H
#define TIDE_FORECAST_H

#include <QObject>
#include <QtPlugin>

#include "WebFactory.h"


namespace Tide {

class TideForecast: public WebFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "net.kvanttiapina.tide.StationFactory/1.0")
    Q_INTERFACES(Tide::StationFactory)

public:

    TideForecast();

protected:

    QString stationUrl(const QString& key);
    QString locationUrl(const QString& key);
    QString availUrl();
    void handleDownloaded(ClientProxy* client, QNetworkReply*);

    QString countryUrl(int country_id);
    void handleStationUpdate(const QString& key, ClientProxy* client, QNetworkReply*);
    void handleLocationUpdate(const QString& key, ClientProxy* client, QNetworkReply*);
    void handleFirstAvailPage(ClientProxy* client, QNetworkReply*);
    void handleSecondAvailPage(const QString& url, ClientProxy* client, QNetworkReply*);

private:

    QString m_BaseUrl;
    QString m_AvailUrl;

    QHash<QString, QString> m_PendingCountries; // country url, country name
    QHash<QString, QString> m_KnownStations; // station url, station key
    QHash<QString, QString> m_KnownLocations; // location url, station key
};

}
#endif // TIDE_FORECAST_H
