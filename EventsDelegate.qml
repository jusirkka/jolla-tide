import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    height: eventIcon.implicitHeight + 2*Theme.paddingSmall
    Image {
        id: eventIcon
        x: Theme.horizontalPageMargin
        source: "icons/" + model.icon + ".png"
    }

    Label {
        id: descriptionLabel

        property real lineHeight: height/lineCount
        color: Theme.secondaryColor
        text: model.description
        font.pixelSize: Theme.fontSizeMedium
        elide: Text.ElideRight
        wrapMode: Text.Wrap
        anchors {
            left: eventIcon.right
            verticalCenter: eventIcon.verticalCenter
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
        }
    }
    Label {
        id: dateLabel
        text: new Date(model.timestamp * 1000).toLocaleString(Qt.locale(), "MMM dd HH:mm")
        font.pixelSize: Theme.fontSizeMedium
        horizontalAlignment: Text.AlignRight
        anchors {
            right: parent.right
            left: descriptionLabel.right
            verticalCenter: descriptionLabel.verticalCenter
            rightMargin: Theme.horizontalPageMargin
        }
    }
}

