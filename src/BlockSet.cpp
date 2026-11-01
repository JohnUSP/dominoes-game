#include "BlockSet.hpp"
#include "Number.hpp"
#include <algorithm>
#include <stdexcept>

namespace dominoes {

BlockSet::BlockSet() { make_set(); }

void BlockSet::shuffle(std::mt19937 &engine) {
  std::shuffle(m_set.begin(), m_set.end(), engine);
}

void BlockSet::make_set() {
  for (Number left : doubleNineNumbers) {
    for (Number right : doubleNineNumbers) {
      if (left <= right) {
        m_set.emplace_back(left, right);
      }
    }
  }
}

Block BlockSet::pick_block() {
  if (m_set.empty()) {
    throw std::out_of_range("Cannot pick a block from an empty set");
  }
  Block block = m_set.back();
  m_set.pop_back();
  return block;
}
} // namespace dominoes
