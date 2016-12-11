import QtQuick 2.2
import Sailfish.Silica 1.0

Page {

    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            //% "New Location"
            title: qsTrId("tide-new-location")
        }

        TextField {
            id: searchField
            anchors {
                left: parent.left
                right: parent.right
                top: header.bottom
            }
            //% "Search Locations"
            placeholderText: qsTrId("tide-search-locations")
            onFocusChanged: if (focus) forceActiveFocus()

            Binding {
                target: stationModel
                property: "filter"
                value: searchField.text.toLowerCase().trim()
            }
        }

        SilicaListView {
            id: locationListView
            anchors {
                left: parent.left
                right: parent.right
                top: searchField.bottom
                bottom: parent.bottom
                topMargin: Theme.paddingLarge
            }
            model: stationModel
            delegate: StationSearchDelegate {
                width: parent.width
            }

            ViewPlaceholder {
                enabled: locationListView.count == 0
                //% "No matching stations"
                text: qsTrId("tide-no-matching-stations")
                //% "Search and select new location"
                hintText: qsTrId("tide-search-and-select-new-location")
            }
        }

        VerticalScrollDecorator {}
    }
}
