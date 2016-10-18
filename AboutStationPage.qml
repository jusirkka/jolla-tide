import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    property string key

    Label {
        id: nameLabel
        width: parent.width
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        color: Theme.primaryColor
        text: stationModel.name(key)
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeHuge
    }

    Column {
        id: aboutColumn
        width: parent.width
        spacing: Theme.paddingSmall
        anchors {
            left: parent.left
            right: parent.right
            top: nameLabel.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            bottomMargin: Theme.paddingLarge
            topMargin: Theme.paddingMedium
        }
        Label {
            textFormat: Text.StyledText
            color: Theme.primaryColor
            text: stationModel.detail(key)
            font.pixelSize: Theme.fontSizeMedium
            elide: Text.ElideRight
            width: parent.width
        }
        Label {
            textFormat: Text.StyledText
            color: Theme.primaryColor
            text: stationModel.location(key)
            font.pixelSize: Theme.fontSizeMedium
            width: parent.width
        }
        Label {
            textFormat: Text.StyledText
            color: Theme.secondaryColor
            text: stationModel.kind(key)
            font.pixelSize: Theme.fontSizeSmall
            width: parent.width
        }
    }

    Image {
        id: logoImage
        anchors {
            left: parent.left
            top: aboutColumn.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            bottomMargin: Theme.paddingMedium
            topMargin: Theme.paddingMedium
            verticalCenter: providerLabel.verticalCenter
        }
        source: "icons/" + stationModel.providerlogo(key) + ".png"
    }

    Label {
        id: providerLabel
        width: parent.width - logoImage.implicitWidth - 2 * Theme.paddingMedium
        height: logoImage.implicitHeight + 2 * Theme.paddingMedium
        anchors {
            left: logoImage.right
            right: parent.right
            top: aboutColumn.bottom
        }
        textFormat: Text.StyledText
        color: Theme.primaryColor
        text: stationModel.provider(key)
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: Theme.fontSizeLarge
    }

    Button {
        id: backButton
        height: parent.height * 0.15
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        //% "Back"
        text: qsTrId("tide-update-back")
        onClicked: stackView.pop()
    }

}
