#ifndef WEB_FACTORY_H
#define WEB_FACTORY_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDomDocument>

#include "Amplitude.h"
#include "StationFactory.h"

namespace Tide {

class WebFactory : public QObject, public StationFactory
{
    Q_OBJECT

public:

    WebFactory(const QString& key, const QString& name, const QString& logo, const QString& description, const QString& visiturl);

    const StationFactoryInfo& info();
    const QHash<QString, StationInfo>& available();
    const Station& instance(const QString& key);
    void update(const QString& key, ClientProxy* client);
    bool updateNeeded(const QString& key);
    void updateAvailable(ClientProxy* client);
    void updateStationInfo(const QString& attr, const QString& key, ClientProxy* client);
    void reset();

    ~WebFactory();


protected:


    virtual QString stationUrl(const QString& key) = 0;
    virtual QString locationUrl(const QString& key) = 0;
    virtual QString availUrl() = 0;
    virtual void handleDownloaded(ClientProxy* client, QNetworkReply*) = 0;

    void storeStation(const QString& key, ClientProxy* client, const QVector<Amplitude>& points, const Timestamp& epoch, const Interval& step);
    void storeAvail(ClientProxy* client, const QHash<QString, QString>& info, bool last=false);
    void storeLocation(const QString& key, ClientProxy* client, const QString& location);

protected slots:

    void downloadReady(QNetworkReply*);

protected:

    StationFactoryInfo m_Info;
    QHash<QString, StationInfo> m_Available;
    QHash<QString, Station*> m_Loaded;
    QHash<QString, Timestamp> m_LastDataPoint;
    Station m_Invalid;
    QNetworkAccessManager* m_DLManager;
    QHash<QString, ClientProxy*> m_Pending;

};

}
#endif // WEB_FACTORY_H
