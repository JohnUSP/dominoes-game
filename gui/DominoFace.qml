pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: face

    required property int value

    // Each position indexes a cell in a 3 x 3 grid, from top left to bottom right.
    readonly property var pipLayouts: [
        [],
        [4],
        [0, 8],
        [0, 4, 8],
        [0, 2, 6, 8],
        [0, 2, 4, 6, 8],
        [0, 1, 2, 6, 7, 8],
        [0, 1, 2, 4, 6, 7, 8],
        [0, 1, 2, 3, 5, 6, 7, 8],
        [0, 1, 2, 3, 4, 5, 6, 7, 8]
    ]

    readonly property real pipDiameter: Math.min(width, height) * 0.18
    readonly property real pipSpacing: Math.min(width, height) * 0.24

    Repeater {
        model: face.value >= 0 && face.value < face.pipLayouts.length
            ? face.pipLayouts[face.value] : []

        delegate: Rectangle {
            required property int modelData

            width: face.pipDiameter
            height: width
            radius: width / 2
            color: "#222222"

            x: face.width / 2 + (modelData % 3 - 1) * face.pipSpacing - width / 2
            y: face.height / 2 + (Math.floor(modelData / 3) - 1) * face.pipSpacing - height / 2
        }
    }
}
