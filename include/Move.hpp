#pragma once

#include <cstddef>
#include <optional>

namespace dominoes {

enum class BoardEnd { left, right };

// A placement request; Game validates it before changing the hand or board.
struct Move {
  // Position in the player's hand; the hand must not reorder before validation.
  std::size_t hand_index;
  // If omitted, use the only legal end, or right if both exposed numbers match.
  // Two legal ends with different exposed numbers require an explicit choice.
  std::optional<BoardEnd> destination{};
};

} // namespace dominoes
