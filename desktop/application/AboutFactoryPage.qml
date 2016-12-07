import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    property int factory
    property string name
    property string about
    property string logo
    property url home

    Image {
        id: logoImage
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: nameLabel.verticalCenter
        source: "icons/" + logo + ".png"
    }

    Label {
        id: nameLabel
        width: parent.width
        anchors {
            left: logoImage.right
            right: parent.right
            top: parent.top
        }
        text: name
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeHuge
    }

    Label {
        id: aboutLabel
        width: parent.width
        height: parent.height * 0.4
        wrapMode: Text.Wrap
        anchors {
            left: parent.left
            right: parent.right
            top: nameLabel.bottom
        }
        text: about
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Button {
        id: homeButton
        anchors {
            left: parent.left
            right: parent.right
            top: aboutLabel.bottom
        }
        text: home.toString()
        onClicked: Qt.openUrlExternally(home)
    }
    Button {
        id: updateButton
        anchors {
            left: parent.left
            right: parent.right
            top: homeButton.bottom
        }
        //% "Update stations"
        text: qsTrId("tide-update-stations")
        onClicked: {
            factoryModel.update(factory)
            stackView.pop()
        }
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
