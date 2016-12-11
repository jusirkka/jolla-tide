import QtQuick 2.2
import Sailfish.Silica 1.0

ListItem {

    id: eventItem

    contentHeight: eventIcon.height + 2 * Theme.paddingMedium

    Row {

        y: Theme.paddingMedium
        x: Theme.horizontalPageMargin
        anchors.leftMargin: Theme.horizontalPageMargin

        spacing: Theme.paddingLarge

        Image {
            id: eventIcon
            source: "qrc:///icons/" + model.icon + ".png"
        }

        Column {

            anchors.verticalCenter: eventIcon.verticalCenter
            spacing: Theme.paddingSmall

            Label {
                text: new Date(model.timestamp * 1000).toLocaleString(Qt.locale(), "ddd dd HH:mm")
                font.pixelSize: Theme.fontSizeMedium
            }

            Label {
                color: Theme.highlightColor
                text: model.description
                font.pixelSize: Theme.fontSizeMedium
            }
        }
    }
}

