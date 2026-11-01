import QtQuick

Rectangle {
    id: root

    required property int leftValue
    required property int rightValue
    property bool selected: false

    signal clicked()

    implicitWidth: 144
    implicitHeight: 72
    radius: 8
    color: "#fffaf0"
    border.color: root.selected ? "#1565c0" : "#444444"
    border.width: root.selected ? 3 : 1

    DominoFace {
        anchors.left: parent.left
        anchors.top: parent.top
        width: root.width / 2
        height: root.height
        value: root.leftValue
    }

    DominoFace {
        anchors.right: parent.right
        anchors.top: parent.top
        width: root.width / 2
        height: root.height
        value: root.rightValue
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 12
        width: 1
        color: "#444444"
    }

    TapHandler {
        cursorShape: Qt.PointingHandCursor
        onTapped: root.clicked()
    }
}
