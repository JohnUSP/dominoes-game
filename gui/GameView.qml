pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import DominoesGui

Item {
    id: gameView

    required property MatchViewModel matchViewModel

    BoardView {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: handView.top
        anchors.margins: 24
        tiles: gameView.matchViewModel.board
    }

    HandView {
        id: handView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: actions.top
        anchors.bottomMargin: 16
        height: implicitHeight
        matchViewModel: gameView.matchViewModel
    }

    Item {
        id: actions

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        height: Math.max(40, feedbackText.implicitHeight)

        Button {
            id: playButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: "Play"
            enabled: gameView.matchViewModel.openingAvailable
                && gameView.matchViewModel.selectedHandIndex >= 0
            onClicked: gameView.matchViewModel.playOpening()
        }

        Text {
            id: feedbackText

            anchors.left: playButton.right
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 16
            text: gameView.matchViewModel.feedback
            color: "#333333"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
    }
}
