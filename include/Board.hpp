#pragma once

#include "Block.hpp"
#include "Move.hpp"

#include <deque>
#include <cstddef>
#include <optional>

namespace dominoes {

class Board {
public:
  // Opening has no matching requirement and is accepted only on an empty board.
  bool open(Block block);

  bool can_place(const Block &block, BoardEnd destination) const;
  // Rejected placements leave the board unchanged.
  bool place(Block block, BoardEnd destination);

  const std::deque<Block> &blocks() const noexcept;
  // Position of the original opening tile, updated when tiles are added left.
  std::optional<std::size_t> opening_index() const noexcept;
  std::optional<Number> left_end() const noexcept;
  std::optional<Number> right_end() const noexcept;

private:
  // Ordered left to right, with matching values on adjacent tile sides.
  std::deque<Block> m_blocks;
  std::optional<std::size_t> m_opening_index;
};

} // namespace dominoes
