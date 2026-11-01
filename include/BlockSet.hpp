#pragma once
#include "Block.hpp"
#include <random>
#include <vector>
namespace dominoes {
class BlockSet {

public:
  BlockSet();
  Block pick_block();
  void shuffle(std::mt19937 &engine);

private:
  void make_set();
  std::vector<Block> m_set;
};
} // namespace dominoes
