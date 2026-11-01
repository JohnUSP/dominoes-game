#pragma once

#include "ActionResult.hpp"
#include "Board.hpp"
#include "HandResult.hpp"
#include "Move.hpp"
#include "Player.hpp"

#include <array>
#include <cstddef>
#include <optional>

namespace dominoes {

class Game {

public:
  // Players are supplied as A1, B1, A2, B2: partners are at indices 0/2 and 1/3.
  // Players must outlive Game; player order must remain unchanged.
  // The caller supplies the player initially entitled to open.
  Game(std::array<Player, 4> &players, std::size_t opening_player);

  // A hand is live state tied to its players, not an independently copyable value.
  Game(const Game &) = delete;
  Game &operator=(const Game &) = delete;
  Game(Game &&) = delete;
  Game &operator=(Game &&) = delete;

  std::size_t current_player_index() const noexcept;
  const Board &board() const noexcept;
  const std::optional<HandResult> &hand_result() const noexcept;

  // Plays for the current player. Rejected moves leave game state unchanged.
  // On an empty board, destination is ignored.
  // Omitted destination: use the only legal end, or right when both ends have
  // the same number. Two legal ends with different numbers require a choice.
  ActionResult play(const Move &move);
  // Passing is explicitly requested by the player, never chosen by Game.
  // Before opening or after completion, passing is rejected.
  ActionResult pass();
  // Before opening: offer to the partner, who may return it once.
  ActionResult transfer_opening();

private:
  bool has_legal_move(const Player &player) const;
  void resolve_blocked_hand();
  void finish_hand(HandEndReason reason,
                   std::optional<std::size_t> winning_team,
                   std::optional<std::size_t> winning_player = std::nullopt);

  std::array<Player, 4> &m_players;
  Board m_board;
  std::size_t m_current_player{0};
  std::size_t m_opening_transfers{0};
  std::size_t m_consecutive_passes{0};
  std::optional<HandResult> m_hand_result;
};
} // namespace dominoes
