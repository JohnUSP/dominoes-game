#include "Match.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace dominoes {
namespace {

static_assert(!std::is_copy_constructible_v<Match>);
static_assert(!std::is_move_constructible_v<Match>);
static_assert(!std::is_copy_constructible_v<Game>);
static_assert(!std::is_move_constructible_v<Game>);
static_assert(std::is_same_v<decltype(std::declval<Match &>().players()),
                             const std::array<Player, 4> &>);
static_assert(std::is_same_v<decltype(std::declval<Match &>().current_hand()),
                             const Game &>);

std::array<std::string, 4> player_names() {
  return {"John", "Kyle", "Jane", "Kate"};
}

using Hands = std::array<std::vector<std::pair<int, int>>, 4>;

Hands snapshot_hands(const Match &match) {
  Hands hands;
  for (std::size_t player = 0; player < hands.size(); ++player) {
    for (const auto &block : match.players()[player].hand()) {
      hands[player].emplace_back(to_value(block.left()), to_value(block.right()));
    }
  }
  return hands;
}

void expect_fresh_hand(const Match &match) {
  std::set<std::pair<int, int>> tiles;
  for (const auto &player : match.players()) {
    EXPECT_EQ(player.hand().size(), 10u);
    for (const auto &block : player.hand()) {
      const int left = to_value(block.left());
      const int right = to_value(block.right());
      EXPECT_TRUE(tiles.emplace(std::min(left, right), std::max(left, right)).second);
    }
  }
  EXPECT_EQ(tiles.size(), 40u);
  EXPECT_TRUE(match.current_hand().board().blocks().empty());
  EXPECT_FALSE(match.current_hand().hand_result());
}

// Drive ordinary legal play, without changing engine state through test hooks.
void finish_hand(Match &match) {
  for (int turn = 0; turn < 200 && !match.current_hand().hand_result(); ++turn) {
    const auto &game = match.current_hand();
    const auto &hand = match.players()[game.current_player_index()].hand();
    if (game.board().blocks().empty()) {
      ASSERT_NE(match.play({0}), ActionResult::rejected);
      continue;
    }

    bool played = false;
    for (std::size_t index = 0; index < hand.size() && !played; ++index) {
      for (const BoardEnd end : {BoardEnd::left, BoardEnd::right}) {
        if (game.board().can_place(hand[index], end)) {
          ASSERT_NE(match.play({index, end}), ActionResult::rejected);
          played = true;
          break;
        }
      }
    }
    if (!played) {
      ASSERT_NE(match.pass(), ActionResult::rejected);
    }
  }
  ASSERT_TRUE(match.current_hand().hand_result());
}

TEST(MatchTest, ConstructionCreatesPlayersAndDealsReadyToPlayHand) {
  std::mt19937 engine{42};
  Match match{player_names(), engine};

  expect_fresh_hand(match);
  EXPECT_EQ(match.players()[0].name(), "John");
  EXPECT_EQ(match.players()[3].name(), "Kate");
  EXPECT_LT(match.current_hand().current_player_index(), 2u);
  EXPECT_EQ(match.team_names()[0], "John & Jane");
  EXPECT_EQ(match.team_names()[1], "Kyle & Kate");
  EXPECT_EQ(match.scores(), (std::array<int, 2>{0, 0}));
  EXPECT_FALSE(match.winning_team());
}

TEST(MatchTest, OwnsPlayerNamesIndependentlyOfCaller) {
  std::mt19937 engine{42};
  auto names = player_names();
  Match match{names, engine};
  names[0] = "Changed";

  EXPECT_EQ(match.players()[0].name(), "John");
}

TEST(MatchTest, KeepsCustomTeamNamesAndDefaultsEmptyNames) {
  std::mt19937 engine{42};
  Match match{player_names(), engine, {"Custom team", ""}};

  EXPECT_EQ(match.team_names()[0], "Custom team");
  EXPECT_EQ(match.team_names()[1], "Kyle & Kate");
}

TEST(MatchTest, RejectsEmptyPlayerNameAtConstruction) {
  std::mt19937 engine{42};
  auto names = player_names();
  names[2].clear();

  EXPECT_THROW((Match{names, engine}), std::invalid_argument);
}

TEST(MatchTest, RejectsPrematureRedealWithoutChangingStateOrRandomEngine) {
  std::mt19937 engine{42};
  Match match{player_names(), engine};
  const auto previous_hands = snapshot_hands(match);
  const auto previous_engine = engine;
  const auto opening = match.current_hand().current_player_index();

  EXPECT_FALSE(match.start_next_hand(engine));
  EXPECT_EQ(snapshot_hands(match), previous_hands);
  EXPECT_EQ(engine, previous_engine);
  EXPECT_EQ(match.current_hand().current_player_index(), opening);
  EXPECT_EQ(match.scores(), (std::array<int, 2>{0, 0}));
}

TEST(MatchTest, RejectedActionsDoNotChangeHandsTurnOrScores) {
  std::mt19937 engine{42};
  Match match{player_names(), engine};
  const auto previous_hands = snapshot_hands(match);
  const auto opening = match.current_hand().current_player_index();

  EXPECT_EQ(match.play({10}), ActionResult::rejected);
  EXPECT_EQ(match.pass(), ActionResult::rejected);
  EXPECT_EQ(snapshot_hands(match), previous_hands);
  EXPECT_EQ(match.current_hand().current_player_index(), opening);
  EXPECT_EQ(match.scores(), (std::array<int, 2>{0, 0}));
}

TEST(MatchTest, TransfersOpeningWithinTeamAndLimitsReturns) {
  std::mt19937 engine{42};
  Match match{player_names(), engine};
  const auto opening = match.current_hand().current_player_index();
  const auto hands = snapshot_hands(match);

  EXPECT_EQ(match.transfer_opening(), ActionResult::accepted);
  EXPECT_EQ(match.current_hand().current_player_index(), (opening + 2) % 4);
  EXPECT_EQ(match.transfer_opening(), ActionResult::accepted);
  EXPECT_EQ(match.current_hand().current_player_index(), opening);
  EXPECT_EQ(match.transfer_opening(), ActionResult::rejected);
  EXPECT_EQ(snapshot_hands(match), hands);
}

TEST(MatchTest, InvalidPassScoresImmediatelyAndCannotScoreAgain) {
  std::mt19937 engine{42};
  Match match{player_names(), engine};
  const auto opening = match.current_hand().current_player_index();
  const auto &opening_hand = match.players()[opening].hand();
  const auto &next_hand = match.players()[(opening + 1) % 4].hand();

  std::size_t chosen = opening_hand.size();
  for (std::size_t index = 0; index < opening_hand.size(); ++index) {
    const auto &candidate = opening_hand[index];
    for (const auto &other : next_hand) {
      if (candidate.left() == other.left() || candidate.left() == other.right() ||
          candidate.right() == other.left() || candidate.right() == other.right()) {
        chosen = index;
        break;
      }
    }
    if (chosen != opening_hand.size()) {
      break;
    }
  }
  ASSERT_LT(chosen, opening_hand.size());
  ASSERT_EQ(match.play({chosen}), ActionResult::accepted);

  const std::size_t losing_team = match.current_hand().current_player_index() % 2;
  int expected_points = 0;
  for (std::size_t index = losing_team; index < 4; index += 2) {
    for (const auto &block : match.players()[index].hand()) {
      expected_points += block.value();
    }
  }

  ASSERT_EQ(match.pass(), ActionResult::hand_ended);
  ASSERT_TRUE(match.current_hand().hand_result());
  EXPECT_EQ(match.current_hand().hand_result()->reason, HandEndReason::invalid_pass);
  EXPECT_EQ(match.scores()[1 - losing_team], expected_points);
  EXPECT_EQ(match.scores()[losing_team], 0);
  const auto scores = match.scores();

  EXPECT_EQ(match.pass(), ActionResult::rejected);
  EXPECT_EQ(match.play({0}), ActionResult::rejected);
  EXPECT_EQ(match.transfer_opening(), ActionResult::rejected);
  EXPECT_EQ(match.scores(), scores);
}

TEST(MatchTest, PlaysCompleteMatchesWithAutomaticScoringAndFreshDeals) {
  int redeals = 0;
  for (unsigned seed = 0; seed < 16; ++seed) {
    SCOPED_TRACE(seed);
    std::mt19937 engine{seed};
    Match match{player_names(), engine};
    const auto *players_address = &match.players();
    std::array<int, 2> expected_scores{};

    for (int hand = 0; hand < 64 && !match.winning_team(); ++hand) {
      const auto opening_team = match.current_hand().current_player_index() % 2;
      ASSERT_NO_FATAL_FAILURE(finish_hand(match));
      const auto result = *match.current_hand().hand_result();
      if (result.winning_team) {
        int points = 0;
        for (std::size_t index = 0; index < 4; ++index) {
          if (index % 2 != *result.winning_team) {
            for (const auto &block : match.players()[index].hand()) {
              points += block.value();
            }
          }
        }
        EXPECT_EQ(result.awarded_points, points);
        expected_scores[*result.winning_team] += points;
      } else {
        EXPECT_EQ(result.awarded_points, 0);
      }
      EXPECT_EQ(match.scores(), expected_scores);
      std::optional<std::size_t> expected_winner;
      for (std::size_t team = 0; team < expected_scores.size(); ++team) {
        if (expected_scores[team] >= Match::target_score) {
          expected_winner = team;
        }
      }
      EXPECT_EQ(match.winning_team(), expected_winner);

      if (match.winning_team()) {
        EXPECT_GE(expected_scores[*match.winning_team()], Match::target_score);
        const auto hands = snapshot_hands(match);
        const auto previous_engine = engine;
        EXPECT_FALSE(match.start_next_hand(engine));
        EXPECT_EQ(snapshot_hands(match), hands);
        EXPECT_EQ(engine, previous_engine);
      } else {
        const auto next_opening = result.winning_player.value_or(opening_team);
        ASSERT_TRUE(match.start_next_hand(engine));
        ++redeals;
        expect_fresh_hand(match);
        EXPECT_EQ(&match.players(), players_address);
        EXPECT_EQ(match.current_hand().current_player_index(), next_opening);
        EXPECT_EQ(match.scores(), expected_scores);
        // The opening-transfer allowance must also reset for each new hand.
        EXPECT_EQ(match.transfer_opening(), ActionResult::accepted);
        EXPECT_EQ(match.transfer_opening(), ActionResult::accepted);
      }
    }
    EXPECT_TRUE(match.winning_team());
  }
  EXPECT_GT(redeals, 0);
}

} // namespace
} // namespace dominoes
