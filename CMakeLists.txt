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
    add_executable(match_tests tests/MatchTest.cpp)
    target_link_libraries(match_tests PRIVATE game_engine GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(match_tests)
endif()
