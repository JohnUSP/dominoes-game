#pragma once

#include <cstddef>
#include <optional>

namespace dominoes {

enum class HandEndReason { domination, blocked, invalid_pass, timeout };

struct HandResult {
  HandEndReason reason;
  // Team 0: seats 0/2; team 1: seats 1/3. No winner means a null hand.
  std::optional<std::size_t> winning_team;
  // Losing team's remaining pips at completion; zero for a null hand.
  int awarded_points;
  // Individual winner; first teammate if teammates tie for the lowest total.
  // For timeout/invalid-pass wins, the winning team's first player receives
  // opening. Empty only for null hands.
  std::optional<std::size_t> winning_player;
};

} // namespace dominoes
