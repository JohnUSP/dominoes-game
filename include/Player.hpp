#pragma once

#include "Block.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace dominoes {

class Player {
public:
  explicit Player(std::string name);

  const std::string &name() const noexcept;
  const std::vector<Block> &hand() const noexcept;

  void take_block(Block block);
  Block remove_block(std::size_t index);
  void clear_hand() noexcept;

private:
  std::string m_name;
  std::vector<Block> m_hand;
};

} // namespace dominoes
