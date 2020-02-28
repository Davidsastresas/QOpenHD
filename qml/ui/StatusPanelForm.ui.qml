import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Qt.labs.settings 1.0

import OpenHD 1.0

import "../ui" as Ui

import "../ui/elements"

Rectangle {
    id: element2
    width: 504
    height: 300

    Label {
        id: ohdLabel
        color: "black"
        text: "Last openhd heartbeat: " + (OpenHD.last_openhd_heartbeat >= 0 && OpenHD.last_openhd_heartbeat <= 300000 ? OpenHD.last_openhd_heartbeat + "ms" : "N/A")
        font.pixelSize: 12
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.bottomMargin: 12
        anchors.leftMargin: 24

        z: 1.1
    }

    Label {
        id: mavLabel
        color: "black"
        text: "Last telemetry heartbeat: " + (OpenHD.last_telemetry_heartbeat >= 0 && OpenHD.last_telemetry_heartbeat <= 300000 ? OpenHD.last_telemetry_heartbeat  + "ms": "N/A")
        font.pixelSize: 12
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 12
        anchors.rightMargin: 142

        z: 1.1
    }

    Loader {
        z: 1.0
        anchors.fill: parent
        source: EnableCharts ? "./elements/StatusChart.qml" : ""
    }

    Label {
        id: peerLabel
        color: "black"
        text: "Peer address: " + link.link_peer
        font.pixelSize: 12
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.bottomMargin: 12
        anchors.rightMargin: 142

        z: 1.1
    }
}
