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

`Game::can_play(move)` checks the same rules as `play()` without changing state.
When the opening tile is a double, the first tile added to it must go to the
right. Later placements may use either legal end.

`players()` and `current_hand()` provide read-only access. Reacquire hand and tile
references after starting another hand. `Game` borrows the players owned by
`Match`; both classes disable copying and moving to protect live state and
references. Keep a Match at a stable address for its lifetime.

## Build and test

With CMake, a C++20 compiler, GoogleTest, and Qt 6.5 or newer (Qt Quick/QML,
Quick Controls 2, and Multimedia)
installed:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Configure with `-DBUILD_TESTING=OFF` to build without GoogleTest.
Configure with `-DBUILD_GUI=OFF` to build the console and engine without Qt.

Run the console application with `./build/dominoes`, or open the desktop hand
view with `./build/gui/dominoes_gui`. The desktop app creates a real match and
displays the opening player's name and ten dealt tiles. Clicking a tile toggles
its selection and plays a short sound. Select a tile and click Play to open the
hand. For later moves, select a tile and click anywhere in its destination half
of the board. With a single-row snake, the left/right halves choose the left/right
ends. Once the snake turns, the top half chooses the upper (right) end and the
bottom half chooses the lower (left) end. These regions have no visual highlight.
An upright opening double still uses left/right halves, and its first extension
must go right. An illegal end request leaves the tile selected and shows feedback.
Accepted moves update the board, current player and hand, and
clear selection. A rejected move preserves them. Completed hands disable play.
Passing, the score display and starting the next hand remain pending in the GUI.

Use `./build/gui/dominoes_gui --tile-gallery` to inspect all 55 tile designs.
Use `./build/gui/dominoes_gui --board-preview` to inspect a connected 40-tile
snake, or add `--preview-count 16` to inspect a shorter chain. The preview grows
around the same 6|6 opening, initially centered. Resize the window to check reflow.
Every double sits perpendicular to the incoming run from the opening, including
at corners. Its incoming connection stays centered on its long side; a corner
exits through an end. A terminal double stays centered across its incoming run.
Board tile size is chosen once for the board dimensions, up to 96 by 48 pixels,
reserving room for a complete forty-tile snake. Adding tiles never changes their
size. Resizing the board establishes a new fixed size. Hand tiles keep their
existing 72 by 128 pixel size.

The Qt presentation layer lives in `gui/`. `MatchViewModel` owns the match and
its random engine. `TileListModel` exposes read-only snapshots through the
`leftValue` and `rightValue` roles; it never retains references to engine tiles.
Selection is presentation state and clears when the hand model resets. The C++
engine has no dependency on Qt.

`BoardTileModel` exposes the ordered board snapshot with positions and rotations.
Its geometry comes from `BoardLayout`, a Qt-independent presentation component.
It starts the original opening at the board center, grows the right arm upward
and the left arm downward, and moves the entire snake vertically in steps of two
tile lengths when an edge needs room. It retains the offset between moves.
Tile size and row width depend on the capacity for forty tiles, rather than the
number already played. The reservation includes forty ordinary tiles, any
opening position and the extra sideways clearance of doubles. It checks both
board edges and rejects overlapping layouts. Normal corners turn at the incoming
matching half. Doubles keep their incoming-run orientation when forming corners.
Each vertical connector contains two lengthwise tiles before the return corner.
A vertical double at the first bend counts as one of those tiles; a horizontal
cross-double occupies its natural thickness. A corner double sends the next
tile directly along its end, on the same centerline. Layout is recomputed after board
changes or viewport resizing. A new hand resets the opening to the center.
The domain Board records the opening
tile's index as the logical chain grows, without knowing any screen coordinates.

On Ubuntu, selection audio also needs the `qt6-multimedia-dev` and
`qml6-module-qtmultimedia` packages:

```sh
sudo apt install qt6-multimedia-dev qml6-module-qtmultimedia
```
