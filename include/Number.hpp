#pragma once
#include <array>
namespace dominoes {

enum class Number {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  SIX = 6,
  SEVEN = 7,
  EIGHT = 8,
  NINE = 9
};
inline constexpr std::array doubleNineNumbers{
    Number::ZERO, Number::ONE, Number::TWO,   Number::THREE, Number::FOUR,
    Number::FIVE, Number::SIX, Number::SEVEN, Number::EIGHT, Number::NINE};

constexpr int to_value(Number num) { return static_cast<int>(num); }

} // namespace dominoes
