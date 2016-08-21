pragma Singleton
import QtQuick 2.0

Item {
    readonly property double paddingLarge: 10.0
    readonly property int fontSizeLarge: 16
    readonly property int fontSizeSmall: 8
    readonly property color highlightColor: "gray"
    readonly property color primaryColor: "black"
    readonly property color secondaryHighlightColor: "cyan"
    readonly property color secondaryColor: "blue"
    readonly property int itemSizeMedium: 30
    readonly property double horizontalPageMargin: 5.0
    readonly property double paddingMedium: 5.0
    readonly property int fontSizeHuge: 24
    readonly property int fontSizeMedium: 12

    function highlightText(base, part, hicolor) {
        if (part === "" || base.search(part) === -1) return base
        return base.replace(part, "<font color='" + hicolor + "'>" + part + "</font>")
    }
}
