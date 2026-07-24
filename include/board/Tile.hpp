#pragma once

namespace board {

class Tile {
public:
  Tile(int left, int right);

  int left() const;
  int right() const;
  bool is_double() const;

  void flip();

private:
  int m_left;
  int m_right;
};

enum class SideValue {
  Zero = 0,
  One = 1,
  Two = 2,
  Three = 3,
  Four = 4,
  Five = 5,
  Six = 6,
  Seven = 7,
  Eight = 8,
  Nine = 9
};

} // namespace board
