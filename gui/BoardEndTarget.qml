pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

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

    ToolTip.visible: hover.hovered
    ToolTip.delay: 400
    ToolTip.text: accessibleName

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: target.clicked()
    }
}
