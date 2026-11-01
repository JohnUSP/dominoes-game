#include "Block.hpp"
#include "Number.hpp"

namespace dominoes {

Block::Block(Number left, Number right) : m_left(left), m_right(right) {}

Number Block::left() const { return m_left; }

Number Block::right() const { return m_right; }

bool Block::is_double() const { return m_left == m_right; }

int Block::value() const { return to_value(m_left) + to_value(m_right); }

void Block::flip() {
  Number temp = m_left;
  m_left = m_right;
  m_right = temp;
}

} // namespace dominoes
