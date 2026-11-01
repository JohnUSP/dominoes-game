#include "Match.hpp"
#include "BlockSet.hpp"

#include <utility>

namespace dominoes {

Match::Match(std::array<std::string, 4> player_names, std::mt19937 &engine,
             std::array<std::string, 2> team_names)
    : m_players{Player{std::move(player_names[0])},
                Player{std::move(player_names[1])},
                Player{std::move(player_names[2])},
                Player{std::move(player_names[3])}},
      m_opening_team(std::uniform_int_distribution<std::size_t>{0, 1}(engine)),
      m_team_names(std::move(team_names)) {
  for (std::size_t team = 0; team < m_team_names.size(); ++team) {
    if (m_team_names[team].empty()) {
      m_team_names[team] =
          m_players[team].name() + " & " + m_players[team + 2].name();
    }
  }
  start_hand(m_opening_team, engine);
}

const std::array<Player, 4> &Match::players() const noexcept { return m_players; }

const Game &Match::current_hand() const noexcept { return *m_current_hand; }

const std::array<std::string, 2> &Match::team_names() const noexcept {
  return m_team_names;
}

const std::array<int, 2> &Match::scores() const noexcept { return m_scores; }

std::optional<std::size_t> Match::winning_team() const noexcept {
  return m_winning_team;
}

ActionResult Match::play(const Move &move) {
  return record_result(m_current_hand->play(move));
}

ActionResult Match::pass() { return record_result(m_current_hand->pass()); }

ActionResult Match::transfer_opening() {
  return record_result(m_current_hand->transfer_opening());
}

ActionResult Match::record_result(ActionResult action) {
  if (action != ActionResult::hand_ended) {
    return action;
  }

  const auto &result = m_current_hand->hand_result();
  if (result->winning_team) {
    const std::size_t team = *result->winning_team;
    m_scores[team] += result->awarded_points;
    if (m_scores[team] >= target_score) {
      m_winning_team = team;
    }
  }

  return action;
}

bool Match::start_next_hand(std::mt19937 &engine) {
  const auto &result = m_current_hand->hand_result();
  if (!result || m_winning_team) {
    return false;
  }

  // Transfers stay within a team, so the original team also handles a null hand.
  const std::size_t opening_player =
      result->winning_player.value_or(m_opening_team);

  start_hand(opening_player, engine);
  return true;
}

void Match::start_hand(std::size_t opening_player, std::mt19937 &engine) {
  BlockSet block_set;
  block_set.shuffle(engine);
  for (Player &player : m_players) {
    player.clear_hand();
  }
  for (int round = 0; round < 10; ++round) {
    for (Player &player : m_players) {
      player.take_block(block_set.pick_block());
    }
  }

  m_current_hand.emplace(m_players, opening_player);
  m_opening_team = opening_player % 2;
}

} // namespace dominoes
