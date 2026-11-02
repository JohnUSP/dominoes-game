pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Item {
    id: preview

    required property BoardTileModel tiles

    Column {
        id: heading

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        spacing: 8

        Text {
            text: "Snake board preview — " + preview.tiles.count + " tiles"
            width: heading.width
            wrapMode: Text.WordWrap
            font.pixelSize: 24
            color: "#222222"
        }

        Text {
            text: "The opening starts centered. The snake shifts vertically when it needs room."
            width: heading.width
            wrapMode: Text.WordWrap
            font.pixelSize: 14
            color: "#555555"
        }
    }

    BoardView {
        anchors.top: heading.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        tiles: preview.tiles
    }
}
