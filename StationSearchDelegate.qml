import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Rectangle {


    MouseArea {
        id: mouseItem
        anchors.fill: parent
        property bool highlighted: pressed && containsMouse
        onClicked: {
            activeStationsModel.append(key)
            stackView.pop()
        }
    }

    Column {
        id: leftColumn
        width: parent.width / 2
        anchors {
            left: parent.left
            right: rightColumn.left
            top: parent.top
            bottom: parent.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            bottomMargin: Theme.paddingMedium
            topMargin: Theme.paddingMedium
        }
        Label {
            textFormat: Text.StyledText
            text: Theme.highlightText(model.name, stationModel.filter, Theme.highlightColor)
            color: mouseItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            font.pixelSize: Theme.fontSizeMedium
            wrapMode: Text.Wrap
            width: parent.width
        }
        Label {
            textFormat: Text.StyledText
            color: mouseItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: Theme.highlightText(model.detail, stationModel.filter, Theme.secondaryHighlightColor)
            font.pixelSize: Theme.fontSizeSmall
            wrapMode: Text.Wrap
            width: parent.width
        }
    }

    Column {
        id: rightColumn
        width: parent.width / 2
        anchors {
            right: parent.right
            left: leftColumn.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
            bottomMargin: Theme.paddingMedium
            topMargin: Theme.paddingMedium
        }

        Label {
            textFormat: Text.StyledText
            color: mouseItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            text: model.location
            font.pixelSize: Theme.fontSizeMedium
            horizontalAlignment: Text.AlignRight
            width: parent.width
        }

        Label {
            textFormat: Text.StyledText
            color: mouseItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            text: model.kind
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignRight
            width: parent.width
        }
    }
}
