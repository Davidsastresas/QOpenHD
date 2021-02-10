#pragma once

#include "QmlObjectListModel.h"
#include "ADSBVehicle.h"

#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <QGeoCoordinate>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QGeoCoordinate>
#include <QSettings>


class ADSBInternet : public QThread
{
    Q_OBJECT

public:
    ADSBInternet();
    ~ADSBInternet();

signals:
    void adsbVehicleUpdate(const ADSBVehicle::VehicleInfo_t vehicleInfo);

protected: 
    void run(void) final;

public slots:
    void mapBoundsChanged(QGeoCoordinate center_coord);

private slots:
    void processReply(QNetworkReply *reply);
    void requestData();

private:
    void init();

    // network 
    QNetworkAccessManager * m_manager;
    QString adsb_url;

    // boundingbox parameters
    QString upperl_lat;
    QString upperl_lon;
    QString lowerr_lat;
    QString lowerr_lon;
    double center_lat;
    double center_lon;
    
    // timer for requests
    int timer_interval = 1000;
    QTimer *timer;

    QSettings _settings;
};

class ADSBVehicleManager : public QObject {
    Q_OBJECT
    
public:
    ADSBVehicleManager(QObject* parent = nullptr);
    ~ADSBVehicleManager();
    static ADSBVehicleManager* instance();

    Q_PROPERTY(QmlObjectListModel* adsbVehicles READ adsbVehicles CONSTANT)

    QmlObjectListModel* adsbVehicles(void) { return &_adsbVehicles; }

    // called from qml when the map has moved
    Q_INVOKABLE void newMapCenter(QGeoCoordinate center_coord);

signals:
    // sent to ADSBInternet to make requests based into this
    void mapCenterChanged(QGeoCoordinate center_coord);

public slots:
    void adsbVehicleUpdate  (const ADSBVehicle::VehicleInfo_t vehicleInfo);
    void onStarted();

    Q_INVOKABLE void setGroundIP(QString address) { _groundAddress = address; }

private slots:
    void _cleanupStaleVehicles(void);

private:
    QmlObjectListModel              _adsbVehicles;
    QMap<uint32_t, ADSBVehicle*>    _adsbICAOMap;
    QTimer                          _adsbVehicleCleanupTimer;
    ADSBInternet*                   _internetLink = nullptr;
    QString                         _groundAddress;
    QSettings                       _settings;

};
