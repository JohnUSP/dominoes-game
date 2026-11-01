#pragma once

#include "Game.hpp"

#include <array>
#include <cstddef>
#include <optional>
#include <random>
#include <string>

namespace dominoes {

class Match {
public:
  static constexpr int target_score = 100;

  // Creates the players, randomly selects the opening team, and deals the hand.
  // Empty team names default to the names of the players in seats 0/2 and 1/3.
  Match(std::array<std::string, 4> player_names, std::mt19937 &engine,
        std::array<std::string, 2> team_names = {});

  // The current Game borrows this Match's players, whose addresses stay fixed.
  Match(const Match &) = delete;
  Match &operator=(const Match &) = delete;
  Match(Match &&) = delete;
  Match &operator=(Match &&) = delete;

  const std::array<Player, 4> &players() const noexcept;
  // References to a hand must not be retained across start_next_hand().
  const Game &current_hand() const noexcept;
  const std::array<std::string, 2> &team_names() const noexcept;
  const std::array<int, 2> &scores() const noexcept;
  std::optional<std::size_t> winning_team() const noexcept;

  // Submit actions through Match so completed hands are scored immediately.
  ActionResult play(const Move &move);
  ActionResult pass();
  ActionResult transfer_opening();

  // Requires a finished hand and an unfinished match. Creates a fresh
  // set and deals ten tiles per player. Rejected transitions change no state.
  bool start_next_hand(std::mt19937 &engine);

private:
  void start_hand(std::size_t opening_player, std::mt19937 &engine);
  ActionResult record_result(ActionResult result);

  // Declared before the borrowing Game so players are destroyed last.
  std::array<Player, 4> m_players;
  std::optional<Game> m_current_hand;
  std::size_t m_opening_team;
  std::array<std::string, 2> m_team_names;
  std::array<int, 2> m_scores{};
  std::optional<std::size_t> m_winning_team;
};

} // namespace dominoes
