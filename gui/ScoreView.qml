pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Item {
    id: scores

    required property MatchViewModel matchViewModel
    implicitHeight: 48

    Row {
        id: teams

        anchors.fill: parent
        spacing: 12

        Repeater {
            model: 2

            delegate: Rectangle {
                id: team

                required property int index

                width: (teams.width - teams.spacing) / 2
                height: teams.height
                color: "#ffffff"
                radius: 6

                Text {
                    anchors.left: parent.left
                    anchors.right: total.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    text: team.index === 0 ? scores.matchViewModel.firstTeamName
                        : scores.matchViewModel.secondTeamName
                    color: "#263b29"
                    font.pixelSize: 16
                    elide: Text.ElideRight
                }

                Text {
                    id: total

                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.rightMargin: 12
                    text: team.index === 0 ? scores.matchViewModel.firstTeamScore
                        : scores.matchViewModel.secondTeamScore
                    color: "#263b29"
                    font.pixelSize: 22
                    font.bold: true
                }
            }
        }
    }
}
