pragma ComponentBehavior: Bound

import QtQuick
import QtMultimedia

Item {
    id: gallery

    property int selectedIndex: -1

    SoundEffect {
        id: selectionSound

        source: Qt.resolvedUrl("sounds/tile-click.wav")
        volume: 0.6
    }

    // Preview data only; actual gameplay will use the C++ engine's tiles.
    readonly property var tiles: {
        const pairs = [];
        for (let left = 0; left <= 9; ++left) {
            for (let right = left; right <= 9; ++right) {
                pairs.push({ leftValue: left, rightValue: right });
            }
        }
        return pairs;
    }

    Column {
        id: heading

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        spacing: 8

        Text {
            text: "Double-nine set — " + gallery.tiles.length + " tiles"
            width: heading.width
            wrapMode: Text.WordWrap
            color: "#222222"
            font.pixelSize: 24
        }

        Text {
            text: "Scroll to inspect all tiles. Click a tile to toggle its selection."
            width: heading.width
            wrapMode: Text.WordWrap
            color: "#555555"
            font.pixelSize: 14
        }
    }

    GridView {
        id: grid

        readonly property int columns: Math.max(1, Math.floor(width / 164))

        anchors.top: heading.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        cellWidth: width / grid.columns
        cellHeight: 96
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: gallery.tiles

        delegate: Item {
            id: cell

            required property int index
            required property var modelData

            width: grid.cellWidth
            height: grid.cellHeight

            DominoTile {
                id: tile

                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                leftValue: cell.modelData.leftValue
                rightValue: cell.modelData.rightValue
                selected: gallery.selectedIndex === cell.index
                onClicked: {
                    gallery.selectedIndex = tile.selected ? -1 : cell.index;
                    selectionSound.play();
                }
            }

        }
    }
}
