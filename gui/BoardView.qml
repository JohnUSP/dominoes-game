pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Rectangle {
    id: boardView

    required property BoardTileModel tiles
    property bool endSelectionEnabled: false

    signal leftEndClicked()
    signal rightEndClicked()

    function updateViewport(): void {
        if (boardView.tiles && surface) {
            boardView.tiles.setViewport(surface.width, surface.height);
        }
    }

    Component.onCompleted: boardView.updateViewport()

    color: "#d6e3d2"
    radius: 8
    clip: true

    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        text: "Board"
        color: "#263b29"
        font.pixelSize: 20
    }

    Text {
        anchors.centerIn: surface
        width: surface.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        visible: boardView.tiles.count === 0
        text: "Select a tile from your hand to open."
        color: "#405c45"
        font.pixelSize: 14
    }

    Item {
        id: surface

        anchors.fill: parent
        anchors.margins: 16
        anchors.topMargin: 52
        onWidthChanged: boardView.updateViewport()
        onHeightChanged: boardView.updateViewport()

        Repeater {
            model: boardView.tiles

            delegate: DominoTile {
                required property real centerX
                required property real centerY
                required property real angle
                required property real tileWidth
                required property real tileHeight

                x: centerX - width / 2
                y: centerY - height / 2
                width: tileWidth
                height: tileHeight
                rotation: angle
                enabled: false
            }
        }

    }

    BoardEndTarget {
        z: 1
        area: boardView.tiles.hasTurns
            ? Qt.rect(0, boardView.height / 2, boardView.width, boardView.height / 2)
            : Qt.rect(0, 0, boardView.width / 2, boardView.height)
        accessibleName: boardView.tiles.hasTurns ? "Play at the lower end" : "Play at the left end"
        visible: boardView.endSelectionEnabled
        onClicked: boardView.leftEndClicked()
    }

    BoardEndTarget {
        z: 1
        area: boardView.tiles.hasTurns
            ? Qt.rect(0, 0, boardView.width, boardView.height / 2)
            : Qt.rect(boardView.width / 2, 0, boardView.width / 2, boardView.height)
        accessibleName: boardView.tiles.hasTurns ? "Play at the upper end" : "Play at the right end"
        visible: boardView.endSelectionEnabled
        onClicked: boardView.rightEndClicked()
    }
}
