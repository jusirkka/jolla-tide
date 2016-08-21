import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

ApplicationWindow {

    id: tideApp
    width: 300
    height: 200
    visible: true


    menuBar: MenuBar {
        Menu {
            title: "File"
            MenuItem {
                id: newLocationMenuItem
                //% "New Location"
                text: qsTrId("tide-new-location")
                onTriggered: {
                    newLocationMenuItem.enabled = false
                    stackView.push(Qt.resolvedUrl("StationSearchPage.qml"))
                }
            }
        }
    }

    StackView {
        id: stackView
        focus: true
        anchors.fill: parent
        initialItem: Item {
            width: parent.width
            height: parent.height
            MainPage {}
        }

        onDepthChanged: {
            if (stackView.depth == 1) {
                newLocationMenuItem.enabled = true
            }
        }
    }
}
