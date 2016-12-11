import QtQuick 2.2
import Sailfish.Silica 1.0

Page {


    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            //% "Active Stations"
            title: qsTrId("tide-active-stations")
        }

        SilicaListView {

            id: activeStationsView

            anchors {
                top: header.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            model: activeStationsModel
            delegate: Component {
                ActiveStationsDelegate {
                    width: parent.width
                }
            }

            ViewPlaceholder {
                enabled: activeStationsView.count == 0
                //% "No Active Stations"
                text: qsTrId("tide-no-active-stations")
                //% "Pull down to add new location"
                hintText: qsTrId("tide-pull-down-to-add-new-location")
            }

        }

        VerticalScrollDecorator {}

        PullDownMenu {

            MenuItem {
                //% "About"
                text: qsTrId("tide-about")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
            }

            MenuItem {
                //% "New Location"
                text: qsTrId("tide-new-location")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("StationSearchPage.qml"))
                }
            }
        }
    }
}
