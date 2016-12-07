import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQml 2.2

ApplicationWindow {

    id: tideApp
    width: 300
    height: 400
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
        Menu {
            id: helpMenu
            title: "Help"

            Instantiator {
                model: factoryModel
                MenuItem {
                    //% "About"
                    text: qsTrId("tide-about") + " " + model.name
                    onTriggered: {
                        stackView.push({item: Qt.resolvedUrl("AboutFactoryPage.qml"), properties: {
                                               factory: model.index,
                                               name: model.name,
                                               about: model.about,
                                               logo: model.logo,
                                               home: Qt.resolvedUrl(model.home)
                                           }
                                       }
                                       )
                    }
                }
                onObjectAdded: helpMenu.insertItem(index, object)
                onObjectRemoved: helpMenu.removeItem(object)
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
