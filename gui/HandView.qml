pragma ComponentBehavior: Bound

import QtQuick
import QtMultimedia
import DominoesGui

Item {
    id: view

    required property MatchViewModel matchViewModel
    implicitHeight: heading.implicitHeight + handList.height + 72

    SoundEffect {
        id: selectionSound

        source: Qt.resolvedUrl("sounds/tile-click.wav")
        volume: 0.6
    }

    Column {
        id: heading

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        spacing: 8

        Text {
            text: view.matchViewModel.currentPlayerName + "'s hand"
            width: heading.width
            wrapMode: Text.WordWrap
            color: "#222222"
            font.pixelSize: 24
        }

        Text {
            text: handList.count + " tiles — click a tile to toggle its selection."
            width: heading.width
            wrapMode: Text.WordWrap
            color: "#555555"
            font.pixelSize: 14
        }
    }

    ListView {
        id: handList

        anchors.top: heading.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        height: 128
        orientation: ListView.Horizontal
        spacing: 12
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: view.matchViewModel.hand
        enabled: !view.matchViewModel.handFinished

        delegate: Item {
            id: cell

            required property int index
            required property int leftValue
            required property int rightValue

            width: 72
            height: handList.height

            DominoTile {
                anchors.centerIn: parent
                width: cell.height
                height: cell.width
                rotation: 90
                leftValue: cell.leftValue
                rightValue: cell.rightValue
                selected: view.matchViewModel.selectedHandIndex === cell.index
                onClicked: {
                    if (view.matchViewModel.toggleSelection(cell.index)) {
                        selectionSound.play();
                    }
                }
            }
        }
    }
}
