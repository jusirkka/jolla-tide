import QtQuick 2.2
import Sailfish.Silica 1.0

ListItem {

    id: stationItem

    // contentHeight: Math.max(leftColumn.implicitHeight, rightColumn.implicitHeight)
    contentHeight: Theme.itemSizeMedium

    onClicked: {
        activeStationsModel.append(key)
        pageStack.pop()
    }


    Column {
        id: leftColumn
        width: parent.width * 0.6
        anchors {
            left: parent.left
            right: rightColumn.left
            top: parent.top
            bottom: parent.bottom
            leftMargin: Theme.paddingSmall
            rightMargin: Theme.paddingSmall
            bottomMargin: Theme.paddingMedium
            topMargin: Theme.paddingMedium
        }
        Label {
            textFormat: Text.StyledText
            text: Theme.highlightText(model.name, stationModel.filter, Theme.highlightColor)
            color: stationItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            font.pixelSize: Theme.fontSizeMedium
            // wrapMode: Text.Wrap
            elide: Text.ElideRight
            width: parent.width
        }
        Label {
            textFormat: Text.StyledText
            color: stationItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: model.location
            font.pixelSize: Theme.fontSizeSmall
            // wrapMode: Text.Wrap
            elide: Text.ElideRight
            width: parent.width
        }
    }

    Column {
        id: rightColumn
        width: parent.width * 0.4
        anchors {
            right: parent.right
            left: leftColumn.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: Theme.paddingSmall
            rightMargin: Theme.paddingSmall
            bottomMargin: Theme.paddingMedium
            topMargin: Theme.paddingMedium
        }

        Label {
            textFormat: Text.StyledText
            color: stationItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            text: Theme.highlightText(model.detail, stationModel.filter, Theme.secondaryHighlightColor)
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideLeft
            width: parent.width
        }

        Label {
            textFormat: Text.StyledText
            color: stationItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: model.kind
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignRight
            width: parent.width
        }
    }
}
