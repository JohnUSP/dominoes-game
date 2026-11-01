# Dominoes Game

A modern C++ implementation of a dominoes game developed as a learning project.

## Goals

- Modern C++
- Object-Oriented Design
- Clean Architecture
- Unit Testing
- CMake
- Git Best Practices

## Match lifecycle

`Match` owns its players and current hand. Construction selects the opening team
and deals ten tiles per player:

```cpp
std::mt19937 engine{std::random_device{}()};
dominoes::Match match{{"John", "Kyle", "Jane", "Kate"}, engine};
```

Submit actions with `match.play(...)`, `match.pass()`, and
`match.transfer_opening()`. A completed hand is scored automatically. Once the
hand ends, `match.start_next_hand(engine)` deals again unless the match has ended.
Both deals use the same internal routine. The random engine is supplied by the
caller so tests can use reproducible seeds; Match does not retain a reference to it.

`players()` and `current_hand()` provide read-only access. Reacquire hand and tile
references after starting another hand. `Game` borrows the players owned by
`Match`; both classes disable copying and moving to protect live state and
references. Keep a Match at a stable address for its lifetime.

## Build and test

With CMake, a C++20 compiler, GoogleTest, and Qt 6.5 or newer (Qt Quick/QML and Multimedia)
installed:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Configure with `-DBUILD_TESTING=OFF` to build without GoogleTest.
Configure with `-DBUILD_GUI=OFF` to build the console and engine without Qt.

Run the console application with `./build/dominoes`, or open the desktop tile
preview with `./build/gui/dominoes_gui`. Clicking a tile toggles its selection
and plays a short sound.

On Ubuntu, selection audio also needs the `qt6-multimedia-dev` and
`qml6-module-qtmultimedia` packages:

```sh
sudo apt install qt6-multimedia-dev qml6-module-qtmultimedia
```
