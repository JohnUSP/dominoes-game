#include "Board.hpp"

namespace dominoes {

bool Board::open(Block block) {
  if (!m_blocks.empty()) {
    return false;
  }

  m_blocks.push_back(block);
  m_opening_index = 0;
  return true;
}

bool Board::can_place(const Block &block, BoardEnd destination) const {
  if (m_blocks.empty()) {
    return false;
  }

  switch (destination) {
  case BoardEnd::left:
    return block.left() == m_blocks.front().left() ||
           block.right() == m_blocks.front().left();
  case BoardEnd::right:
    return block.left() == m_blocks.back().right() ||
           block.right() == m_blocks.back().right();
  }

  return false;
}

bool Board::place(Block block, BoardEnd destination) {
  if (!can_place(block, destination)) {
    return false;
  }

  if (destination == BoardEnd::left) {
    if (block.right() != m_blocks.front().left()) {
      block.flip();
    }
    m_blocks.push_front(block);
    ++*m_opening_index;
  } else {
    if (block.left() != m_blocks.back().right()) {
      block.flip();
    }
    m_blocks.push_back(block);
  }

  return true;
}

const std::deque<Block> &Board::blocks() const noexcept { return m_blocks; }

std::optional<std::size_t> Board::opening_index() const noexcept {
  return m_opening_index;
}

std::optional<Number> Board::left_end() const noexcept {
  if (m_blocks.empty()) {
    return std::nullopt;
  }
  return m_blocks.front().left();
}

std::optional<Number> Board::right_end() const noexcept {
  if (m_blocks.empty()) {
    return std::nullopt;
  }
  return m_blocks.back().right();
}

} // namespace dominoes
