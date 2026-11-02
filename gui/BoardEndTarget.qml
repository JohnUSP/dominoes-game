pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: target

    required property rect area
    required property string accessibleName

    signal clicked()

    x: area.x
    y: area.y
    width: area.width
    height: area.height
    activeFocusOnTab: true

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName
    Accessible.onPressAction: target.clicked()
    Keys.onReturnPressed: target.clicked()
    Keys.onSpacePressed: target.clicked()

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: target.clicked()
    }
}
