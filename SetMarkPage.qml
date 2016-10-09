import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    height: parent.height


    property int station
    property string name
    property string level
    property string mark

    Column {
        id: headerColumn
        anchors {
            right: parent.right
            left: parent.left
            top: parent.top
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            bottomMargin: Theme.paddingLarge
            topMargin: Theme.paddingLarge
        }
        Label {
            horizontalAlignment: Text.AlignRight
            width: parent.width
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeHuge
            //% "Set mark"
            text: qsTrId("tide-set-mark")
        }
        Label {
            horizontalAlignment: Text.AlignRight
            width: parent.width
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeLarge
            text: name
        }
    }

    Column {
        id: levelColumn
        anchors {
            left: parent.left
            right: parent.right
            top: headerColumn.bottom
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            bottomMargin: Theme.paddingLarge
            topMargin: Theme.paddingLarge
        }
        Label {
            horizontalAlignment: Text.AlignLeft
            width: parent.width
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeLarge
            //% "Current level"
            text: qsTrId("tide-current-level")
        }
        Label {
            horizontalAlignment: Text.AlignLeft
            width: parent.width
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeMedium
            text: level
        }
    }

    Column {
        id: markColumn
        anchors {
            left: parent.left
            right: parent.right
            top: levelColumn.bottom
            leftMargin: Theme.paddingLarge
            rightMargin: Theme.paddingLarge
            bottomMargin: Theme.paddingLarge
            topMargin: Theme.paddingLarge
        }
        Label {
            horizontalAlignment: Text.AlignLeft
            width: parent.width
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeLarge
            //% "Mark"
            text: qsTrId("tide-mark")
        }
        TextField {
            id: markField
            font.pixelSize: Theme.fontSizeLarge
            text: mark.split(" ")[0]
        }
    }

    Button {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        text: "Save"
        onClicked: {
            activeStationsModel.setmark(station, markField.text + " " + level.split(" ")[1])
            stackView.pop()
        }
    }

}
