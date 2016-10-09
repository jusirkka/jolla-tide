import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {

    height: labelColumn.implicitHeight + 2*Theme.paddingMedium

    MouseArea {
        id: mus
        enabled: model.icon !== "tide-invalid"
        anchors.fill: parent
        property bool highlighted: pressed && containsMouse
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            if (mouse.button === Qt.LeftButton) {
                eventsModel.init(model.key, model.mark)
                stackView.push(Qt.resolvedUrl("EventsPage.qml"))
            }
            else if (mouse.button === Qt.RightButton) {
                contextMenu.popup()
            }
        }
    }

    Image {
        id: eventIcon
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: labelColumn.verticalCenter
        source: "icons/" + model.icon + ".png"
    }

    Column {
        id: labelColumn
        y: Theme.paddingMedium
        height: cityLabel.height + descriptionLabel.lineHeight
        anchors {
            left: eventIcon.right
            right: dateLabel.left
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
        }
        Label {
            id: cityLabel
            width: parent.width
            color: mus.highlighted ? Theme.highlightColor : Theme.primaryColor
            text: model.name
        }
        Label {
            id: descriptionLabel

            property real lineHeight: height/lineCount
            width: parent.width
            color: mus.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: model.description
            font.pixelSize: Theme.fontSizeSmall
            elide: Text.ElideRight
            wrapMode: Text.Wrap
        }
    }
    Label {
        id: dateLabel
        text: new Date(model.timestamp * 1000).toLocaleTimeString(Qt.locale(), "HH:mm")
        color: mus.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeHuge
        anchors {
            verticalCenter: labelColumn.verticalCenter
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
    }
    Menu {
        id: contextMenu
        MenuItem {
            //% "Move to top"
            text: qsTrId("tide-move_to_top")
            visible: model.index !== 0
            onTriggered: activeStationsModel.movetotop(model.index)
        }
        MenuItem {
            //% "Set mark"
            text: qsTrId("tide-set-mark")
            onTriggered: {
                stackView.push({item: Qt.resolvedUrl("SetMarkPage.qml"), properties: {
                                       station: model.index,
                                       name: model.name,
                                       level: model.level,
                                       mark: model.mark
                                   }})
            }
        }
        MenuItem {
            //% "Show points"
            text: qsTrId("tide-show-points")
            onTriggered: activeStationsModel.showpoints(model.index)
        }
        MenuItem {
            //% "Remove"
            text: qsTrId("tide-remove")
            onTriggered: activeStationsModel.remove(model.index)
        }
    }
}

