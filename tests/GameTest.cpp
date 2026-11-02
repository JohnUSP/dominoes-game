#include "Game.hpp"

#include <gtest/gtest.h>

namespace dominoes {
namespace {

std::array<Player, 4> players() {
  std::array<Player, 4> result{Player{"A"}, Player{"B"}, Player{"C"}, Player{"D"}};
  for (std::size_t index = 0; index < result.size(); ++index) {
    const auto value = static_cast<Number>(index);
    result[index].take_block({value, value});
  }
  return result;
}

TEST(GameTest, FirstTileAfterAnOpeningDoubleMustGoRightThenBothEndsAreAvailable) {
  auto seats = players();
  seats[0].take_block({Number::SIX, Number::SIX});
  seats[1].take_block({Number::SIX, Number::THREE});
  seats[2].take_block({Number::TWO, Number::SIX});
  seats[2].take_block({Number::THREE, Number::FOUR});
  Game game{seats, 0};
  ASSERT_EQ(game.play({1}), ActionResult::accepted);

  EXPECT_FALSE(game.can_play({1, BoardEnd::left}));
  EXPECT_TRUE(game.can_play({1, BoardEnd::right}));
  EXPECT_TRUE(game.can_play({1}));
  EXPECT_EQ(game.play({1, BoardEnd::left}), ActionResult::rejected);
  EXPECT_EQ(game.current_player_index(), 1u);
  EXPECT_EQ(seats[1].hand().size(), 2u);
  EXPECT_EQ(game.board().blocks().size(), 1u);
  ASSERT_EQ(game.play({1}), ActionResult::accepted);
  EXPECT_EQ(game.board().opening_index(), 0u);
  EXPECT_TRUE(game.can_play({1, BoardEnd::left}));
  EXPECT_TRUE(game.can_play({2, BoardEnd::right}));
  EXPECT_EQ(game.play({1, BoardEnd::left}), ActionResult::accepted);
  EXPECT_EQ(game.board().opening_index(), 1u);
}

TEST(GameTest, OrdinaryOpeningAllowsBothEndsAndLegalityQueriesPreserveState) {
  auto seats = players();
  seats[0].take_block({Number::FOUR, Number::SEVEN});
  seats[1].take_block({Number::TWO, Number::FOUR});
  seats[1].take_block({Number::SEVEN, Number::THREE});
  seats[2].take_block({Number::THREE, Number::FOUR});
  Game game{seats, 0};
  EXPECT_TRUE(game.can_play({1}));
  EXPECT_FALSE(game.can_play({2}));
  ASSERT_EQ(game.play({1}), ActionResult::accepted);
  EXPECT_TRUE(game.can_play({1, BoardEnd::left}));
  EXPECT_TRUE(game.can_play({2, BoardEnd::right}));
  EXPECT_TRUE(game.can_play({1}));
  EXPECT_FALSE(game.can_play({0, BoardEnd::left}));
  EXPECT_FALSE(game.can_play({1, static_cast<BoardEnd>(42)}));
  EXPECT_EQ(game.current_player_index(), 1u);
  EXPECT_EQ(game.board().blocks().size(), 1u);
  EXPECT_EQ(seats[1].hand().size(), 3u);
  ASSERT_EQ(game.play({2, BoardEnd::right}), ActionResult::accepted);
  EXPECT_TRUE(game.can_play({1, BoardEnd::left}));
  EXPECT_TRUE(game.can_play({1, BoardEnd::right}));
  EXPECT_FALSE(game.can_play({1})); // Distinct exposed values require a choice.
  EXPECT_EQ(game.play({1}), ActionResult::rejected);
  EXPECT_EQ(game.play({1, BoardEnd::left}), ActionResult::accepted);
}

TEST(GameTest, FinishedHandHasNoLegalPlacements) {
  auto seats = players();
  seats[0].clear_hand();
  seats[0].take_block({Number::FOUR, Number::SEVEN});
  Game game{seats, 0};
  ASSERT_EQ(game.play({0}), ActionResult::hand_ended);
  EXPECT_FALSE(game.can_play({0, BoardEnd::left}));
  EXPECT_FALSE(game.can_play({0, BoardEnd::right}));
  EXPECT_EQ(game.play({0}), ActionResult::rejected);
}

} // namespace
} // namespace dominoes
