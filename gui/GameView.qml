pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import DominoesGui

Item {
    id: gameView

    required property MatchViewModel matchViewModel

    ScoreView {
        id: scores

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 24
        height: implicitHeight
        matchViewModel: gameView.matchViewModel
    }

    BoardView {
        anchors.top: scores.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: handView.top
        anchors.margins: 24
        anchors.topMargin: 16
        tiles: gameView.matchViewModel.board
        endSelectionEnabled: gameView.matchViewModel.selectedHandIndex >= 0
            && !gameView.matchViewModel.openingAvailable
            && !gameView.matchViewModel.handFinished
        onLeftEndClicked: gameView.matchViewModel.playLeft()
        onRightEndClicked: gameView.matchViewModel.playRight()
    }

    HandView {
        id: handView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: actions.top
        anchors.bottomMargin: 16
        height: implicitHeight
        matchViewModel: gameView.matchViewModel
        // Preserve the hand's measurements while the final result occupies its area.
        opacity: gameView.matchViewModel.handFinished ? 0 : 1
    }

    HandResultView {
        anchors.fill: handView
        visible: gameView.matchViewModel.handFinished
        matchViewModel: gameView.matchViewModel
    }

    Item {
        id: actions

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 24
        // Status updates must not resize the board and change its fixed tile size.
        height: 56

        Button {
            id: turnButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: gameView.matchViewModel.handFinished ? "Next hand"
                : gameView.matchViewModel.openingAvailable ? "Play" : "Pass"
            visible: !gameView.matchViewModel.matchFinished
            width: visible ? implicitWidth : 0
            enabled: gameView.matchViewModel.handFinished
                ? gameView.matchViewModel.nextHandAvailable
                : gameView.matchViewModel.openingAvailable
                    ? gameView.matchViewModel.selectedHandIndex >= 0
                    : gameView.matchViewModel.passAvailable
            onClicked: {
                if (gameView.matchViewModel.handFinished) {
                    gameView.matchViewModel.startNextHand();
                } else if (gameView.matchViewModel.openingAvailable) {
                    gameView.matchViewModel.playOpening();
                } else {
                    gameView.matchViewModel.passTurn();
                }
            }
        }

        Text {
            id: feedbackText

            anchors.left: turnButton.right
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: turnButton.visible ? 16 : 0
            text: gameView.matchViewModel.feedback
            color: "#333333"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
    }
}
