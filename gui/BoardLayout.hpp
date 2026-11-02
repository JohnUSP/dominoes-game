#pragma once

#include "Block.hpp"

#include <span>
#include <cstddef>
#include <vector>

namespace dominoes::gui {

struct BoardTilePlacement {
  double center_x;
  double center_y;
  int rotation;
  // Logical traversal directions: 0 east, 90 south, 180 west, 270 north.
  // Different directions identify a corner on this tile.
  int incoming_direction;
  int outgoing_direction;
};

struct BoardLayout {
  std::vector<BoardTilePlacement> tiles;
  double tile_width{96};
  double tile_height{48};
  double half_lane{0}; // Row half-width in unscaled face widths.
  int vertical_shift_steps{0}; // Each step is two rendered tile lengths.
  double viewport_width{0};
  double viewport_height{0};
};

// Pure presentation geometry. Input remains in engine chain order.
// Starts the opening at the viewport center and grows two separate arms.
// Every double crosses its incoming run; at a corner it exits through an end.
// Vertical connectors contain two lengthwise tiles before returning horizontally.
// Chooses tile size and row width from the complete forty-tile capacity, including
// double clearance, once per viewport size. Tile count never changes that size.
// A previous layout retains its vertical offset; shifts are two tile lengths.
// Viewport dimensions must be finite and positive.
BoardLayout layout_board(std::span<const Block> tiles, std::size_t opening_index,
                         double width, double height,
                         const BoardLayout *previous = nullptr);

} // namespace dominoes::gui
