import QtQuick 2.2
import Sailfish.Silica 1.0

Page {

    SilicaFlickable {

        anchors.fill: parent

        PageHeader {
            id: header
            //% "About"
            title: qsTrId("tide-about")
        }

        Label {
            id: aboutLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: header.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
            text: "JollaTide is a tide calendar application. The tide and Sun/Moon event computation code is from " +
                  "<a href=\"http://www.flaterco.com/xtide/\">XTide</a> © David Flater."
        }

        SectionHeader {
            id: authorHeader
            anchors {
                left: parent.left
                right: parent.right
                top: aboutLabel.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Author"
            text: qsTrId("tide-author")
        }

        Label {
            id: authorLabel
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: authorHeader.bottom
                leftMargin: Theme.horizontalPageMargin
            }
            text: "Jukka Sirkka"
        }

        SectionHeader {
            id: sourceHeader
            anchors {
                left: parent.left
                right: parent.right
                top: authorLabel.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Source"
            text: qsTrId("tide-source")
        }

        Button {
            id: sourceButton
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            anchors {
                left: parent.left
                right: parent.right
                top: sourceHeader.bottom
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            text: "github"
            onClicked: Qt.openUrlExternally("https://github.com/jusirkka/jolla-tide")
        }

        SectionHeader {
            id: providerHeader
            anchors {
                left: parent.left
                right: parent.right
                top: sourceButton.bottom
                rightMargin: Theme.horizontalPageMargin
            }
            //% "Data Provider Plugins"
            text: qsTrId("tide-data-provider-plugins")
        }


        SilicaListView {

            id: providersView

            anchors {
                top: providerHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            model: factoryModel
            delegate: Component {
                FactoryDelegate {
                    width: parent.width
                }
            }

        }

        VerticalScrollDecorator {}
    }
}
