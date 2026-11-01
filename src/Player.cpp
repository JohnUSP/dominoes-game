#include "Player.hpp"

#include <stdexcept>
#include <utility>

namespace dominoes {

Player::Player(std::string name) : m_name(std::move(name)) {
  if (m_name.empty()) {
    throw std::invalid_argument("Player name cannot be empty");
  }
}

const std::string &Player::name() const noexcept { return m_name; }

const std::vector<Block> &Player::hand() const noexcept { return m_hand; }

void Player::take_block(Block block) { m_hand.push_back(std::move(block)); }

Block Player::remove_block(std::size_t index) {
  if (index >= m_hand.size()) {
    throw std::out_of_range("Invalid block index");
  }

  Block selected = std::move(m_hand[index]);
  m_hand.erase(m_hand.begin() + static_cast<std::ptrdiff_t>(index));
  return selected;
}

void Player::clear_hand() noexcept { m_hand.clear(); }

} // namespace dominoes
