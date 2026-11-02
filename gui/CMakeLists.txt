find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Multimedia)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(dominoes_gui main.cpp)

set(presentation_sources
    MatchViewModel.cpp MatchViewModel.hpp
    TileListModel.cpp TileListModel.hpp
    BoardTileModel.cpp BoardTileModel.hpp
)

qt_add_qml_module(dominoes_gui
    URI DominoesGui
    SOURCES ${presentation_sources}
    QML_FILES
        Main.qml
        DominoTile.qml
        DominoFace.qml
        TileGallery.qml
        HandView.qml
        BoardView.qml
        BoardEndTarget.qml
        GameView.qml
        BoardPreview.qml
    RESOURCES
        sounds/tile-click.wav
)

target_include_directories(dominoes_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dominoes_gui
    PRIVATE game_engine board_layout Qt6::Quick Qt6::QuickControls2 Qt6::Multimedia
)

if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(gui_presentation_tests
        ${PROJECT_SOURCE_DIR}/tests/GuiPresentationTest.cpp
        ${presentation_sources}
    )
    target_include_directories(gui_presentation_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(gui_presentation_tests
        PRIVATE game_engine board_layout Qt6::Qml Qt6::Test GTest::gtest_main
    )
    gtest_discover_tests(gui_presentation_tests)
endif()
