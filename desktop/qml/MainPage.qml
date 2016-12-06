import QtQuick 2.0
import QtQuick.Controls 1.4
import net.kvanttiapina.tide.theme 1.0

Item {
    width: parent.width
    height: parent.height

    ListView {
        anchors.fill: parent
        model: activeStationsModel
        delegate: ActiveStationsDelegate {
            width: parent.width
        }

    }
}
