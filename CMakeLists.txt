cmake_minimum_required(VERSION 3.20)

project(
    Dominoes
    VERSION 0.1.0
    DESCRIPTION "Dominoes Game"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    dominoesapp
    app/main.cpp
)
