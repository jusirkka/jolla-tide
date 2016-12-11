import QtQuick 2.2
import Sailfish.Silica 1.0

Page {

    property string key

    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            //% "Events"
            title: qsTrId("tide-events")
        }

        SilicaListView {

            id: eventsView

            Component.onCompleted: {
                eventsModel.deltaChanged.connect(adjustTop)
                contentY = (eventsModel.today - 0.5 * visibleArea.heightRatio) * contentHeight
            }

            function adjustTop(v) {
                contentY = v * contentHeight
            }

            onAtYBeginningChanged: if (atYBeginning) {
                                       eventsModel.rewind()
                                   }

            onAtYEndChanged: if (atYEnd) {
                                 eventsModel.forward()
                             }

            anchors {
                top: header.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            model: eventsModel
            delegate: Component {
                EventsDelegate {
                    width: parent.width
                }
            }

        }

        VerticalScrollDecorator {}

        PullDownMenu {

            MenuItem {
                //% "Station Info"
                text: qsTrId("tide-station-info")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutStationPage.qml"), {key: key})
                }
            }
        }

    }
}
