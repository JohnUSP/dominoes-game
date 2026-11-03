pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Rectangle {
    id: result

    required property MatchViewModel matchViewModel

    color: "#ffffff"
    radius: 8

    Column {
        anchors.centerIn: parent
        width: Math.max(0, parent.width - 48)
        spacing: 10

        Text {
            width: parent.width
            text: result.matchViewModel.matchFinished ? "Match finished" : "Hand finished"
            color: "#222222"
            font.pixelSize: 24
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: result.matchViewModel.matchFinished
                ? "Final hand: " + result.matchViewModel.handEndReason
                : result.matchViewModel.handEndReason
            color: "#405c45"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: result.matchViewModel.matchFinished
                ? "Match winner: " + result.matchViewModel.matchWinningTeamName
                : result.matchViewModel.handTied ? "Tied hand"
                    : "Winning team: " + result.matchViewModel.handWinningTeamName
            color: "#222222"
            font.pixelSize: 18
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: "Points awarded: " + result.matchViewModel.handAwardedPoints
            color: "#405c45"
            font.pixelSize: 16
        }
    }
}
