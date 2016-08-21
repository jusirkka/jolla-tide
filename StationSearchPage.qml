import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Item {
    Label {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        //% "New location"
        text: qsTrId("tide-new_location")
        horizontalAlignment: Text.AlignRight
        font.pixelSize: Theme.fontSizeLarge
    }
    TextField {
        id: searchField
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: header.bottom

        //% "Search locations"
        placeholderText: qsTrId("tide-search_locations")
        onFocusChanged: if (focus) forceActiveFocus()

        Binding {
            target: stationModel
            property: "filter"
            value: searchField.text.toLowerCase().trim()
        }
    }

    Label {
        id: placeHolder
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchField.bottom
        visible: locationListView.count == 0
        horizontalAlignment: Text.AlignHCenter
        //% "Search and select new location"
        text: qsTrId("tide-search_and_select_location")
        font.pixelSize: Theme.fontSizeLarge
    }


    ListView {
        id: locationListView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: placeHolder.bottom
        anchors.bottom: parent.bottom
        model: stationModel
        delegate: StationSearchDelegate {
            width: parent.width
            height: Theme.itemSizeMedium
        }
    }
}
