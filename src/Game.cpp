#include "Game.hpp"

#include <stdexcept>

namespace dominoes {

Game::Game(std::array<Player, 4> &players, std::size_t opening_player)
    : m_players(players), m_current_player(opening_player) {
  if (opening_player >= m_players.size()) {
    throw std::invalid_argument("Opening player index is out of range");
  }
}

std::size_t Game::current_player_index() const noexcept {
  return m_current_player;
}

const Board &Game::board() const noexcept { return m_board; }

const std::optional<HandResult> &Game::hand_result() const noexcept {
  return m_hand_result;
}

ActionResult Game::play(const Move &move) {
  if (m_hand_result) {
    return ActionResult::rejected;
  }

  Player &player = m_players[m_current_player];
  if (move.hand_index >= player.hand().size()) {
    return ActionResult::rejected;
  }

  // Place first so an allocation failure cannot remove a tile from the hand.
  const Block &block = player.hand()[move.hand_index];
  bool placed;
  if (m_board.blocks().empty()) {
    placed = m_board.open(block);
  } else {
    auto destination = move.destination;
    if (!destination) {
      const bool matches_left = m_board.can_place(block, BoardEnd::left);
      const bool matches_right = m_board.can_place(block, BoardEnd::right);
      if (!matches_left && !matches_right) {
        return ActionResult::rejected;
      }
      if (matches_left && matches_right &&
          m_board.left_end() != m_board.right_end()) {
        return ActionResult::rejected;
      }
      destination = matches_right ? BoardEnd::right : BoardEnd::left;
    }
    placed = m_board.place(block, *destination);
  }
  if (!placed) {
    return ActionResult::rejected;
  }

  player.remove_block(move.hand_index);
  m_consecutive_passes = 0;
  if (player.hand().empty()) {
    finish_hand(HandEndReason::domination, m_current_player % 2,
                m_current_player);
    return ActionResult::hand_ended;
  }

  m_current_player = (m_current_player + 1) % m_players.size();
  return ActionResult::accepted;
}

ActionResult Game::pass() {
  if (m_hand_result || m_board.blocks().empty()) {
    return ActionResult::rejected;
  }

  if (has_legal_move(m_players[m_current_player])) {
    finish_hand(HandEndReason::invalid_pass, 1 - m_current_player % 2);
    return ActionResult::hand_ended;
  }

  ++m_consecutive_passes;
  if (m_consecutive_passes == m_players.size()) {
    resolve_blocked_hand();
    return ActionResult::hand_ended;
  }

  m_current_player = (m_current_player + 1) % m_players.size();
  return ActionResult::accepted;
}

ActionResult Game::transfer_opening() {
  if (m_hand_result || !m_board.blocks().empty() || m_opening_transfers >= 2) {
    return ActionResult::rejected;
  }

  m_current_player = (m_current_player + 2) % m_players.size();
  ++m_opening_transfers;
  return ActionResult::accepted;
}

bool Game::has_legal_move(const Player &player) const {
  for (const Block &block : player.hand()) {
    if (m_board.can_place(block, BoardEnd::left) ||
        m_board.can_place(block, BoardEnd::right)) {
      return true;
    }
  }
  return false;
}

void Game::resolve_blocked_hand() {
  std::array<int, 4> totals{};
  std::size_t best_player = 0;
  for (std::size_t index = 0; index < m_players.size(); ++index) {
    for (const Block &block : m_players[index].hand()) {
      totals[index] += block.value();
    }
    if (totals[index] < totals[best_player]) {
      best_player = index;
    }
  }

  std::optional<std::size_t> winning_team = best_player % 2;
  for (std::size_t index = 0; index < m_players.size(); ++index) {
    if (totals[index] == totals[best_player] && index % 2 != *winning_team) {
      winning_team.reset();
      break;
    }
  }

  finish_hand(HandEndReason::blocked, winning_team,
              winning_team ? std::optional<std::size_t>{best_player}
                           : std::nullopt);
}

void Game::finish_hand(HandEndReason reason,
                       std::optional<std::size_t> winning_team,
                       std::optional<std::size_t> winning_player) {
  int awarded_points = 0;
  if (winning_team) {
    if (reason == HandEndReason::invalid_pass || reason == HandEndReason::timeout) {
      // Team 0 starts at seat 0; team 1 starts at seat 1.
      winning_player = *winning_team;
    }
    for (std::size_t index = 0; index < m_players.size(); ++index) {
      if (index % 2 != *winning_team) {
        for (const Block &block : m_players[index].hand()) {
          awarded_points += block.value();
        }
      }
    }
  }

  m_hand_result =
      HandResult{reason, winning_team, awarded_points, winning_player};
}

} // namespace dominoes
