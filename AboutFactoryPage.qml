import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    property int factory
    property string name
    property string about
    property url home

    Label {
        id: nameLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        text: name
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeLarge
    }

    Label {
        id: aboutLabel
        height: parent.height * 0.4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: nameLabel.bottom
        text: about
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Button {
        id: homeButton
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: aboutLabel.bottom
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
