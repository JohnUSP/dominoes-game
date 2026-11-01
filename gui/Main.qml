import QtQuick

Window {
    width: 960
    height: 640
    minimumWidth: 320
    minimumHeight: 320
    visible: true
    title: "Dominoes — Tile preview"
    color: "#eeeeee"

    TileGallery {
        anchors.fill: parent
    }
}
