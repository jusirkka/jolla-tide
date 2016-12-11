import QtQuick 2.2
import Sailfish.Silica 1.0

Page {

    property int station
    property string name
    property string level
    property string mark

    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            //% "Set Mark"
            title: qsTrId("tide-set-mark")
        }

        InfoLabel {
            id: nameLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: header.bottom
            }
            text: name
        }


        SectionHeader {
            id: levelHeader
            anchors {
                left: parent.left
                right: parent.right
                top: nameLabel.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Current level"
            text: qsTrId("tide-current-level")
        }

        Label {
            id: levelLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: levelHeader.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            text: level
        }

        SectionHeader {
            id: markHeader
            anchors {
                left: parent.left
                right: parent.right
                top: levelLabel.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Mark"
            text: qsTrId("tide-mark")
        }

        TextField {
            id: markField
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: markHeader.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            font.pixelSize: Theme.fontSizeLarge
            text: mark.split(" ")[0]
        }

        Button {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: markField.bottom
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            text: qsTrId("tide-set-mark")
            onClicked: {
                activeStationsModel.setmark(station, markField.text + " " + level.split(" ")[1])
                pageStack.pop()
            }
        }

        VerticalScrollDecorator {}
    }
}
