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
    Row {
        Column {
            Label {
                textFormat: Text.StyledText
                text: Theme.highlightText(model.name, stationModel.filter, Theme.highlightColor)
                color: mouseItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            Label {
                textFormat: Text.StyledText
                text: model.kind
                color: mouseItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }
        }
        Column {
            Label {
                textFormat: Text.StyledText
                text: Theme.highlightText(model.detail, stationModel.filter, Theme.highlightColor)
                color: mouseItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                horizontalAlignment: Text.AlignRight
            }
            Label {
                textFormat: Text.StyledText
                text: model.location
                color: mouseItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignRight
            }
        }
    }
}
