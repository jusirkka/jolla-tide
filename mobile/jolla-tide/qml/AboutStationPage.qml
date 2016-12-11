import QtQuick 2.2
import Sailfish.Silica 1.0

Page {

    property string key

    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            title: stationModel.name(key)
        }

        Label {
            id: detailLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: header.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            text: stationModel.detail(key)
        }

        Label {
            id: locationLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: detailLabel.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            text: stationModel.location(key)
        }

        Label {
            id: typeLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: locationLabel.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            text: "Constituent scheme: <i>" + stationModel.kind(key) + "</i>"
        }


        SectionHeader {
            id: providerHeader
            anchors {
                left: parent.left
                right: parent.right
                top: typeLabel.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Data Provider"
            text: qsTrId("tide-data-provider")
        }

        Row {

            id: providerRow
            spacing: Theme.paddingLarge

            anchors {
                left: parent.left
                right: parent.right
                top: providerHeader.bottom
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }

            Image {
                x: Theme.horizontalPageMargin
                source: "qrc:///icons/" + stationModel.providerlogo(key) + ".png"
            }

            Label {
                text: stationModel.provider(key)
                font.pixelSize: Theme.fontSizeLarge
            }
        }
    }
}
