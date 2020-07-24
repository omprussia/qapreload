// Copyright (c) 2019-2020 Open Mobile Platform LLC.
import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: root
    width: 1
    height: 1
    opacity: 0.0
    property int size: Theme.itemSizeSmall

    function show(point, psize) {
        hideAnimation.stop()
        x = point.x
        y = point.y
        if (psize && psize.width > 0) {
            size = psize.width
        } else {
            size = Theme.itemSizeSmall
        }
        opacity = 1.0
    }

    function hide() {
        hideAnimation.start()
    }

    function hideImmediately() {
        hideAnimation.stop()
        opacity = 0.0
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

    NumberAnimation {
        id: hideAnimation
        target: root
        property: "opacity"
        to: 0.0
        duration: 250
        easing.type: Easing.InQuad
    }
}
