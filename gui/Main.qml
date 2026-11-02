pragma ComponentBehavior: Bound

import QtQuick
import DominoesGui

Window {
    id: window

    required property MatchViewModel matchViewModel
    required property BoardTileModel previewBoard
    property bool showTileGallery: false
    property bool showBoardPreview: false

    width: 1100
    height: 800
    minimumWidth: 320
    minimumHeight: window.showTileGallery || window.showBoardPreview ? 320 : 560
    visible: true
    title: window.showTileGallery ? "Dominoes — Tile preview"
        : window.showBoardPreview ? "Dominoes — Board preview" : "Dominoes"
    color: "#eeeeee"

    Loader {
        anchors.fill: parent
        sourceComponent: window.showTileGallery ? galleryComponent
            : window.showBoardPreview ? boardPreviewComponent : gameComponent
    }

    Component {
        id: gameComponent

        GameView {
            matchViewModel: window.matchViewModel
        }
    }

    Component {
        id: galleryComponent

        TileGallery {}
    }

    Component {
        id: boardPreviewComponent

        BoardPreview {
            tiles: window.previewBoard
        }
    }
}
