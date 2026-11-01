find_package(Qt6 6.5 REQUIRED COMPONENTS Quick Multimedia)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(dominoes_gui main.cpp)

qt_add_qml_module(dominoes_gui
    URI DominoesGui
    QML_FILES
        Main.qml
        DominoTile.qml
        DominoFace.qml
        TileGallery.qml
    RESOURCES
        sounds/tile-click.wav
)

target_link_libraries(dominoes_gui PRIVATE Qt6::Quick Qt6::Multimedia)
