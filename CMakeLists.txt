cmake_minimum_required(VERSION 3.20)

project(
    Dominoes
    VERSION 0.1.0
    DESCRIPTION "Dominoes Game"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(game_engine
    ${PROJECT_SOURCE_DIR}/src/Block.cpp
    ${PROJECT_SOURCE_DIR}/src/BlockSet.cpp
    ${PROJECT_SOURCE_DIR}/src/Board.cpp
    ${PROJECT_SOURCE_DIR}/src/Player.cpp
    ${PROJECT_SOURCE_DIR}/src/Game.cpp
    ${PROJECT_SOURCE_DIR}/src/Match.cpp
)

target_include_directories(game_engine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Presentation geometry is pure C++ and can be tested without Qt.
add_library(board_layout gui/BoardLayout.cpp)
target_include_directories(board_layout PUBLIC ${PROJECT_SOURCE_DIR}/gui)
target_link_libraries(board_layout PUBLIC game_engine)

add_executable(dominoes
    ${PROJECT_SOURCE_DIR}/app/main.cpp
)

target_link_libraries(dominoes
    PRIVATE
        game_engine
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(match_tests tests/MatchTest.cpp tests/BoardTest.cpp tests/GameTest.cpp)
    target_link_libraries(match_tests PRIVATE game_engine GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(match_tests)
    add_executable(board_layout_tests tests/BoardLayoutTest.cpp)
    target_link_libraries(board_layout_tests PRIVATE board_layout GTest::gtest_main)
    gtest_discover_tests(board_layout_tests)
endif()

option(BUILD_GUI "Build the Qt Quick desktop application" ON)
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
