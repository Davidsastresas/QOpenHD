/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ADSBVehicleManager.h"
#include "localmessage.h"

#include <QDebug>

static ADSBVehicleManager* _instance = nullptr;

ADSBVehicleManager* ADSBVehicleManager::instance() 
{
    if ( _instance == nullptr ) {
        _instance = new ADSBVehicleManager();
    }
    return _instance;
}

ADSBVehicleManager::ADSBVehicleManager(QObject *parent) : QObject(parent)
{
}

ADSBVehicleManager::~ADSBVehicleManager() 
{
    // manually stop the threads
    _internetLink->quit();
    _internetLink->wait();
}

void ADSBVehicleManager::onStarted()
{
    #if defined(__rasp_pi__)
    groundAddress = "127.0.0.1";
    #endif

    connect(&_adsbVehicleCleanupTimer, &QTimer::timeout, this, &ADSBVehicleManager::_cleanupStaleVehicles);
    _adsbVehicleCleanupTimer.setSingleShot(false);
    _adsbVehicleCleanupTimer.start(1000);

    _internetLink = new ADSBInternet();
    connect(_internetLink, &ADSBInternet::adsbVehicleUpdate, this, &ADSBVehicleManager::adsbVehicleUpdate, Qt::QueuedConnection);
    connect(this, &ADSBVehicleManager::mapCenterChanged, _internetLink, &ADSBInternet::mapBoundsChanged, Qt::QueuedConnection);
}

// called from qml when the map is moved
void ADSBVehicleManager::newMapCenter(QGeoCoordinate center_coord) {
    emit mapCenterChanged(center_coord);
}

void ADSBVehicleManager::_cleanupStaleVehicles()
{
    // Remove all expired ADSB vehicles
    for (int i=_adsbVehicles.count()-1; i>=0; i--) {
        ADSBVehicle* adsbVehicle = _adsbVehicles.value<ADSBVehicle*>(i);
        if (adsbVehicle->expired()) {
            qDebug() << "Expired" << QStringLiteral("%1").arg(adsbVehicle->icaoAddress(), 0, 16);
            _adsbVehicles.removeAt(i);
            _adsbICAOMap.remove(adsbVehicle->icaoAddress());
            adsbVehicle->deleteLater();
        }
    }
}

void ADSBVehicleManager::adsbVehicleUpdate(const ADSBVehicle::VehicleInfo_t vehicleInfo)
{
    uint32_t icaoAddress = vehicleInfo.icaoAddress;

    if (_adsbICAOMap.contains(icaoAddress)) {
        _adsbICAOMap[icaoAddress]->update(vehicleInfo);
    } else {
        if (vehicleInfo.availableFlags & ADSBVehicle::LocationAvailable) {
            ADSBVehicle* adsbVehicle = new ADSBVehicle(vehicleInfo, this);
            _adsbICAOMap[icaoAddress] = adsbVehicle;
            _adsbVehicles.append(adsbVehicle);
        }
    }
}

ADSBInternet::ADSBInternet()
    : QThread()
{
    moveToThread(this);
    start();
}

ADSBInternet::~ADSBInternet(void)
{
    quit();
    wait();
}

void ADSBInternet::run(void)
{
    init();
    exec();
}

void ADSBInternet::init(void) {
    // qDebug() << "------------------AdsbInternet::init()";

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);

    m_manager = manager;

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReply(QNetworkReply*))) ;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ADSBInternet::requestData);

    // How frequently data is requested
    timer->start(timer_interval);
    mapBoundsChanged(QGeoCoordinate(40.48205, -3.35996)); // this shouldn't be necesary
}

void ADSBInternet::requestData(void) {
    adsb_url= "https://opensky-network.org/api/states/all?lamin="+lowerr_lat+"&lomin="+upperl_lon+"&lamax="+upperl_lat+"&lomax="+lowerr_lon;

    QNetworkRequest request;
    QUrl api_request = adsb_url;
    request.setUrl(api_request);
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    // qDebug() << "url=" << api_request;
    m_manager->get(request);
}

void ADSBInternet::processReply(QNetworkReply *reply) {
    bool icaoOk;
    uint32_t icaoAddress;

    QString callsign;
    int contact;
    double lat;
    double lon;
    int alt;
    int track;
    int velocity;
    double vertical;

    if (reply->error()) {
        qDebug() << "ADSB request error!";
        qDebug() << reply->errorString();
        LocalMessage::instance()->showMessage("ADSB Reply Error", 4);
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();

    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);

    if (doc.isNull()) {
        qDebug() << "Parse failed";
    }

    if(doc.isNull()){
        qDebug()<<"Failed to create JSON doc.";
        reply->deleteLater();
        return;
    }
    if(!doc.isObject()){
        qDebug()<<"JSON is not an object.";
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObject = doc.object();

    if(jsonObject.isEmpty()){
        qDebug()<<"JSON object is empty.";
        reply->deleteLater();
        return;
    }

    QJsonValue value = jsonObject.value("states");
        QJsonArray array = value.toArray();

        // qDebug() << "MYARRAY COUNT=" << array.count();

        foreach (const QJsonValue & v, array){
            QJsonArray innerarray = v.toArray();

            QString icaoAux = innerarray[0].toString();
            icaoAddress = icaoAux.toUInt(&icaoOk, 16);

            // skip this element if icao number is not all right
            if (!icaoOk) {
                continue;
            }

            callsign=innerarray[1].toString();
            if (callsign.length() == 0) {
                callsign = "N/A";
            }
            contact=innerarray[4].toInt();
            lat=innerarray[6].toDouble();
            lon=innerarray[5].toDouble();
            alt=innerarray[7].toDouble();
            velocity=innerarray[9].toDouble();
            track=innerarray[10].toDouble();
            vertical=innerarray[11].toDouble();

            // qDebug() << "icao=" << innerarray[0].toString();
            // qDebug() << "icaoNum=" << icaoAddress;
            // qDebug() << "callsign=" << innerarray[1].toString();
            // qDebug() << "last_contact=" << innerarray[4].toInt();
            // qDebug() << "lat=" << innerarray[6].toDouble();
            // qDebug() << "lon=" << innerarray[5].toDouble();
            // qDebug() << "alt=" << innerarray[7].toDouble();
            qDebug() << "velocity=" << innerarray[9].toDouble();
            // qDebug() << "track=" << innerarray[10].toDouble();
            // qDebug() << "vertical=" << innerarray[11].toDouble();
            // qDebug() << "distance=" << distance;
            // qDebug() << "----------------------------------------------------------";

            QGeoCoordinate location(lat, lon);

            ADSBVehicle::VehicleInfo_t adsbInfo;
            adsbInfo.icaoAddress = icaoAddress;
            adsbInfo.callsign = callsign;
            adsbInfo.location = location;
            adsbInfo.altitude = alt;
            adsbInfo.velocity = velocity;
            adsbInfo.heading = track;
            adsbInfo.availableFlags = ADSBVehicle::CallsignAvailable | ADSBVehicle::LocationAvailable | ADSBVehicle::AltitudeAvailable | ADSBVehicle::HeadingAvailable | ADSBVehicle::VelocityAvailable;
            emit adsbVehicleUpdate(adsbInfo);
        }
    reply->deleteLater();
}

// this is duplicated
void ADSBInternet::mapBoundsChanged(QGeoCoordinate center_coord) {
    // qreal adsb_distance_limit = _settings.value("adsb_distance_limit").toInt();
    qreal adsb_distance_limit = 1000000;

    center_lat= center_coord.latitude();
    center_lon= center_coord.longitude();

    QGeoCoordinate qgeo_upper_left;
    QGeoCoordinate qgeo_lower_right;

    qgeo_upper_left = center_coord.atDistanceAndAzimuth(adsb_distance_limit, 315, 0.0);
    qgeo_lower_right = center_coord.atDistanceAndAzimuth(adsb_distance_limit, 135, 0.0);

    upperl_lat= QString::number(qgeo_upper_left.latitude());
    upperl_lon= QString::number(qgeo_upper_left.longitude());
    lowerr_lat= QString::number(qgeo_lower_right.latitude());
    lowerr_lon= QString::number(qgeo_lower_right.longitude());
}