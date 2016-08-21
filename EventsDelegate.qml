import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    height: Math.max(dateLabel.implicitHeight, descriptionLabel.implicitHeight) + 2*Theme.paddingMedium

    Image {
        id: eventIcon
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: descriptionLabel.verticalCenter
        source: "icons/" + model.icon + ".png"
    }

    Label {
        id: descriptionLabel

        property real lineHeight: height/lineCount
        width: parent.width
        color: Theme.secondaryColor
        text: model.description
        font.pixelSize: Theme.fontSizeMedium
        elide: Text.ElideRight
        wrapMode: Text.Wrap
        anchors {
            left: eventIcon.right
            right: dateLabel.left
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
        }
    }
    Label {
        id: dateLabel
        text: new Date(model.timestamp * 1000).toLocaleString(Qt.locale(), "MMM dd HH:mm")
        font.pixelSize: Theme.fontSizeMedium
        anchors {
            verticalCenter: descriptionLabel.verticalCenter
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
    }
}

