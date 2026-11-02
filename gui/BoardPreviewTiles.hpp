#pragma once

#include "Block.hpp"

#include <array>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace dominoes::gui {

// A connected, unique 40-tile fixture for visual layout inspection, not a deal.
inline std::vector<Block> board_preview_tiles() {
  constexpr std::array values{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
      7, 7, 8, 8, 9, 9, 0, 2, 4, 6, 8, 0, 3, 6, 9, 2, 5, 8, 1, 4, 7,
      0, 4, 8, 2, 6, 0};
  std::vector<Block> tiles;
  for (std::size_t index = 1; index < values.size(); ++index) {
    tiles.emplace_back(static_cast<Number>(values[index - 1]),
                       static_cast<Number>(values[index]));
  }
  return tiles;
}

struct BoardPreviewChain {
  std::vector<Block> tiles;
  std::size_t opening_index;
};

// Reveal tiles around the same 6|6 opening, rather than moving the opening.
inline BoardPreviewChain board_preview_chain(std::size_t count) {
  const auto all = board_preview_tiles();
  if (count == 0 || count > all.size()) {
    throw std::out_of_range("Preview count must be from 1 through 40");
  }
  constexpr std::size_t opening = 12;
  const auto left_count = std::min(opening, (count - 1) / 2);
  const auto first = all.begin() + static_cast<std::ptrdiff_t>(opening - left_count);
  return {{first, first + static_cast<std::ptrdiff_t>(count)}, left_count};
}

} // namespace dominoes::gui
