#pragma once

#include "Block.hpp"
#include "Move.hpp"

#include <deque>
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
  std::optional<Number> left_end() const noexcept;
  std::optional<Number> right_end() const noexcept;

private:
  // Ordered left to right, with matching values on adjacent tile sides.
  std::deque<Block> m_blocks;
};

} // namespace dominoes
