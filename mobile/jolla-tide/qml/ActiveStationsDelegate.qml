import QtQuick 2.2
import Sailfish.Silica 1.0

ListItem {

    id: activeStationItem
    menu: contextMenu

    contentHeight: Theme.itemSizeLarge

    ListView.onRemove: animateRemoval(activeStationItem)

    function remove() {
        //% "Removing"
        remorseAction(qsTrId("tide-removing"), function () {
            activeStationsModel.remove(index)
        })
    }

    onClicked: {
        if (model.icon !== "tide-invalid") {
            eventsModel.init(model.key, model.mark)
            pageStack.push(Qt.resolvedUrl("EventsPage.qml"), {key: model.key})
        }
    }

    Image {
        id: eventIcon
        x: Theme.horizontalPageMargin
        anchors.verticalCenter: labelColumn.verticalCenter
        source: "qrc:///icons/" + model.icon + ".png"
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
            color: activeStationItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            text: model.name
        }
        Label {
            id: descriptionLabel

            property real lineHeight: height/lineCount
            width: parent.width
            color: activeStationItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: model.description
            font.pixelSize: Theme.fontSizeSmall
            elide: Text.ElideRight
        }
    }
    Label {
        id: dateLabel
        text: new Date(model.timestamp * 1000).toLocaleTimeString(Qt.locale(), "HH:mm")
        color: activeStationItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
        font.pixelSize: Theme.fontSizeHuge
        anchors {
            verticalCenter: labelColumn.verticalCenter
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
    }

    Component {
        id: contextMenu

        ContextMenu {

            MenuItem {
                //% "Move to top"
                text: qsTrId("tide-move-to-top")
                visible: model.index !== 0
                onClicked: activeStationsModel.movetotop(model.index)
            }

            MenuItem {
                enabled: model.icon !== "tide-invalid"
                //% "Set Mark"
                text: qsTrId("tide-set-mark")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("SetMarkPage.qml"), {
                                           station: model.index,
                                           name: model.name,
                                           level: model.level,
                                           mark: model.mark
                                   })
                }
            }

            MenuItem {
                //% "Remove"
                text: qsTrId("tide-remove")
                onClicked: remove()
            }
        }
    }
}

