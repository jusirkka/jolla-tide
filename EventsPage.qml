import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {
    width: parent.width
    height: parent.height

    ListView {
        id: eventsList
        height: parent.height * 0.9
        width: parent.width

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        model: eventsModel
        delegate: EventsDelegate {
            width: parent.width
        }

        Component.onCompleted: {
            contentY = (eventsModel.today - 0.5 * visibleArea.heightRatio) * contentHeight
            eventsModel.deltaChanged.connect(adjustTop)
        }

        function adjustTop(v) {
            contentY = v * contentHeight
        }

        onAtYBeginningChanged: if (atYBeginning) {
                                   console.log("rewind")
                                   eventsModel.rewind()
                               }

        onAtYEndChanged: if (atYEnd) {
                             console.log("forward")
                             eventsModel.forward()
                         }

        // onContentYChanged: console.log("adjust to " + contentY)

    }

    Button {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            top: eventsList.bottom
        }
        text: "Back"
        onClicked: stackView.pop()
    }
}
