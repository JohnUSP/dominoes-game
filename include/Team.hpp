#pragma once

#include <array>
#include <cstddef>

namespace dominoes {

struct Team {
  std::array<std::size_t, 2> player_indices;
};

} // namespace dominoes
