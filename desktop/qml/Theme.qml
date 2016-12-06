pragma Singleton
import QtQuick 2.0

Item {
    readonly property double paddingLarge: 10.0
    readonly property double paddingMedium: 5.0
    readonly property double paddingSmall: 2.5
    readonly property int fontSizeHuge: 24
    readonly property int fontSizeLarge: 16
    readonly property int fontSizeMedium: 12
    readonly property int fontSizeSmall: 8
    readonly property color highlightColor: "cyan"
    readonly property color primaryColor: "black"
    readonly property color secondaryHighlightColor: "cyan"
    readonly property color secondaryColor: "blue"
    readonly property int itemSizeMedium: 30
    readonly property double horizontalPageMargin: 5.0

    function highlightText(base, part, hicolor) {
        if (part === "") return base
        var mo = new RegExp(part, 'i')
        var parts = base.match(mo) // first match
        if (!parts) return base
        return base.replace(mo, "<font color='" + hicolor + "'>" + parts[0] + "</font>")
    }
}
