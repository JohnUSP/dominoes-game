#pragma once

#include "Number.hpp"

namespace dominoes {

class Block {
public:
  Block(Number left, Number right);

  Number left() const;
  Number right() const;
  bool is_double() const;
  int value() const;

  void flip();

private:
  Number m_left;
  Number m_right;
};

} // namespace dominoes
