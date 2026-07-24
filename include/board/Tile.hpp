#pragma once

#include "SideValue.hpp"
namespace board {
class Tile {
public:
  Tile(SideValue left, SideValue right);

  SideValue left() const;
  SideValue right() const;
  bool is_double() const;

  void flip();

private:
  SideValue m_left;
  SideValue m_right;
};

} // namespace board
