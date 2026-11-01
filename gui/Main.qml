pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Window {
    id: window

    required property MatchViewModel matchViewModel
    property bool showTileGallery: false

    width: 960
    height: 640
    minimumWidth: 320
    minimumHeight: 320
    visible: true
    title: window.showTileGallery ? "Dominoes — Tile preview" : "Dominoes"
    color: "#eeeeee"

    Loader {
        anchors.fill: parent
        sourceComponent: window.showTileGallery ? galleryComponent : handComponent
    }

    Component {
        id: handComponent

        HandView {
            matchViewModel: window.matchViewModel
        }
    }

    Component {
        id: galleryComponent

        TileGallery {}
    }
}
