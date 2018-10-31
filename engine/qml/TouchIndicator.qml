import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: root
    width: 1
    height: 1
    visible: false
    property int size: Theme.itemSizeSmall

    function show(point, psize) {
        x = point.x
        y = point.y
        if (psize && psize.width > 0) {
            size = psize.width
        } else {
            size = Theme.itemSizeSmall
        }
        visible = true
    }

    function hide() {
        root.visible = false
    }

    Rectangle {
        anchors.centerIn: parent
        width: root.size
        height: root.size
        radius: root.size / 2
        border.color: Theme.highlightColor
        border.width: 1
        color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
    }
}
