import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13
import QtGraphicalEffects 1.13
import Qt.labs.settings 1.0

import OpenHD 1.0

BaseWidget {
    id: speedWidget
    width: 40
    height: 25
    defaultXOffset: 20
    defaultVCenter: true

    widgetIdentifier: "speed_widget"

    defaultHCenter: false

    hasWidgetDetail: true
    widgetDetailComponent: Column {
        Item {
            width: parent.width
            height: 24
            Text {
                text: "Toggle Airspeed/GPS"
                horizontalAlignment: Text.AlignRight
                color: "white"
                font.bold: true
                anchors.left: parent.left
            }
            Switch {
                width: 32
                height: parent.height
                anchors.rightMargin: 12
                anchors.right: parent.right
                // @disable-check M222
                Component.onCompleted: checked = settings.value("airpeed_gps",
                                                                true)
                // @disable-check M222
                onCheckedChanged: settings.setValue("airspeed_gps", checked)
            }
        }
    }

    Item {
        id: speed_Text
        anchors.fill: parent

        Text {
            id: speed_text
            color: "white"
            text: qsTr(OpenHD.speed)
            horizontalAlignment: Text.AlignRight
            anchors.right: parent.right
            anchors.rightMargin: 0
            bottomPadding: 2
            topPadding: 2
        }

        Text {
            id: speed_glyph
            y: 0
            width: 40
            height: 18
            color: "#ffffff"
            text: "\ufdd7"
            anchors.right: parent.right
            anchors.rightMargin: -40
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Font Awesome 5 Free-Solid-900.otf"
            font.pixelSize: 14
        }

        antialiasing: true
    }
}

/*##^##
Designer {
    D{i:5;anchors_height:15;anchors_width:30}
}
##^##*/
