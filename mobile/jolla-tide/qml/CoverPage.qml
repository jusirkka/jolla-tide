import QtQuick 2.2
import Sailfish.Silica 1.0

CoverBackground {

    id: cover

    onStatusChanged: {
        if (cover.status == Cover.Active) {
            console.log("checking events")
            activeStationsModel.computeNextEvent()
        }
    }

    Label {
        id: nameLabel
        width: parent.width
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeSmall
        text: coverModel.station
        elide: Text.ElideRight
    }

    SilicaListView {

        id: coverStationsView

        anchors {
            top: nameLabel.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        model: coverModel
        delegate: Component {
            CoverStationsDelegate {
                width: parent.width
            }
        }

        ViewPlaceholder {
            enabled: coverStationsView.count == 0
            //% "No Active Stations"
            text: qsTrId("tide-no-active-stations")
        }
    }


}
