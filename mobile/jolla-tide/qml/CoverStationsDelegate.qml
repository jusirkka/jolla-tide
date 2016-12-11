import QtQuick 2.2
import Sailfish.Silica 1.0

ListItem {

    id: coverStationItem

    contentHeight: eventIcon.height + 2*Theme.paddingSmall

    Row {

        y: Theme.paddingSmall

        spacing: Theme.paddingSmall

        Image {
            id: eventIcon
            source: "qrc:///icons/" + model.icon + ".png"
        }

        Column {

            anchors.verticalCenter: eventIcon.verticalCenter
            spacing: Theme.paddingSmall

            Label {
                text: new Date(model.timestamp * 1000).toLocaleString(Qt.locale(), "ddd dd HH:mm")
                font.pixelSize: Theme.fontSizeTiny
            }

            Label {
                color: Theme.highlightColor
                text: model.description
                font.pixelSize: Theme.fontSizeTiny
                elide: Text.ElideRight
            }
        }
    }
}

