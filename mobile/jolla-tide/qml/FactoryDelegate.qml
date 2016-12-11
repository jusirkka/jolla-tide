import QtQuick 2.2
import Sailfish.Silica 1.0

ListItem {

    id: factoryItem
    contentHeight: contentColumn.height + 2*Theme.paddingMedium


    Column {

        id: contentColumn

        y: Theme.paddingMedium

        spacing: Theme.paddingSmall

        anchors.horizontalCenter: parent.horizontalCenter

        Row {

            id: providerRow
            spacing: Theme.paddingLarge

            Image {
                x: Theme.horizontalPageMargin
                source: "qrc:///icons/" + model.logo + ".png"
            }

            Label {
                text: model.name
                font.pixelSize: Theme.fontSizeLarge
            }

        }

        Label {
            id: aboutLabel
            width: parent.width
            wrapMode: Text.Wrap
            text: model.about
            horizontalAlignment: Text.AlignHCenter
        }

        Row {

            spacing: Theme.paddingLarge

            Button {
                //% "Home"
                text: qsTrId("tide-home")
                onClicked: Qt.openUrlExternally(model.home)
            }

            Button {
                //% "Update Stations"
                text: qsTrId("tide-update-stations")
                onClicked: {
                    factoryModel.update(model.index)
                    pageStack.pop()
                }
            }
        }
    }

}

