/****************************************************************************
 *
 * This file has been ported from QGroundControl project <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QGeoCoordinate>
#include <QElapsedTimer>

class ADSBVehicle : public QObject
{
    Q_OBJECT

public:
    enum {
        CallsignAvailable =     1 << 1,
        LocationAvailable =     1 << 2,
        AltitudeAvailable =     1 << 3,
        HeadingAvailable =      1 << 4,
        AlertAvailable =        1 << 5,
        VelocityAvailable =     1 << 6
    };

    typedef struct {
        uint32_t        icaoAddress;    // Required
        QString         callsign;
        QGeoCoordinate  location;
        double          altitude;
        double          velocity;
        double          heading;
        bool            alert;
        uint32_t        availableFlags;
    } VehicleInfo_t;

    ADSBVehicle(const VehicleInfo_t& vehicleInfo, QObject* parent);

    Q_PROPERTY(int              icaoAddress READ icaoAddress    CONSTANT)
    Q_PROPERTY(QString          callsign    READ callsign       NOTIFY callsignChanged)
    Q_PROPERTY(QGeoCoordinate   coordinate  READ coordinate     NOTIFY coordinateChanged)
    Q_PROPERTY(double           lat         READ lat            NOTIFY coordinateChanged)
    Q_PROPERTY(double           lon         READ lon            NOTIFY coordinateChanged)
    Q_PROPERTY(double           altitude    READ altitude       NOTIFY altitudeChanged)     // NaN for not available
    Q_PROPERTY(double           velocity    READ velocity       NOTIFY velocityChanged)     // NaN for not available
    Q_PROPERTY(double           heading     READ heading        NOTIFY headingChanged)      // NaN for not available
    Q_PROPERTY(bool             alert       READ alert          NOTIFY alertChanged)        // Collision path

    int             icaoAddress (void) const { return static_cast<int>(_icaoAddress); }
    QString         callsign    (void) const { return _callsign; }
    QGeoCoordinate  coordinate  (void) const { return _coordinate; }
    double          lat         (void) const { return _coordinate.latitude(); }
    double          lon         (void) const { return _coordinate.longitude(); }
    double          altitude    (void) const { return _altitude; }
    double          velocity    (void) const { return _velocity; }
    double          heading     (void) const { return _heading; }
    bool            alert       (void) const { return _alert; }

    void update(const VehicleInfo_t& vehicleInfo);

    /// check if the vehicle is expired and should be removed
    bool expired();

signals:
    void coordinateChanged  ();
    void callsignChanged    ();
    void altitudeChanged    ();
    void velocityChanged    ();
    void headingChanged     ();
    void alertChanged       ();

private:
    /* According with Thomas Voß, we should be using 2 minutes for the time being
    static constexpr qint64 expirationTimeoutMs = 5000; ///< timeout with no update in ms after which the vehicle is removed.
                                                        ///< AirMap sends updates for each vehicle every second.
    */
    static constexpr qint64 expirationTimeoutMs = 120000;

    uint32_t        _icaoAddress;
    QString         _callsign;
    QGeoCoordinate  _coordinate;
    double          _altitude;
    double          _velocity;
    double          _heading;
    bool            _alert;

    QElapsedTimer   _lastUpdateTimer;
};

Q_DECLARE_METATYPE(ADSBVehicle::VehicleInfo_t)

