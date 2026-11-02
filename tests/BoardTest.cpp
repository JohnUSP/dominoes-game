#include "Board.hpp"

#include <gtest/gtest.h>

namespace dominoes {

TEST(BoardTest, TracksTheOriginalOpeningWhenEitherEndGrows) {
  Board board;
  EXPECT_FALSE(board.opening_index());
  ASSERT_TRUE(board.open({Number::FOUR, Number::THREE}));
  ASSERT_EQ(board.opening_index(), 0u);
  ASSERT_TRUE(board.place({Number::TWO, Number::FOUR}, BoardEnd::left));
  EXPECT_EQ(board.opening_index(), 1u);
  ASSERT_TRUE(board.place({Number::THREE, Number::FIVE}, BoardEnd::right));
  EXPECT_EQ(board.opening_index(), 1u);
  ASSERT_TRUE(board.place({Number::SIX, Number::TWO}, BoardEnd::left));
  ASSERT_EQ(board.opening_index(), 2u);
  EXPECT_EQ(board.blocks()[*board.opening_index()].left(), Number::FOUR);
  EXPECT_EQ(board.blocks()[*board.opening_index()].right(), Number::THREE);
}

TEST(BoardTest, RejectedActionsPreserveTheOpeningIdentity) {
  Board board;
  EXPECT_FALSE(board.place({Number::ONE, Number::TWO}, BoardEnd::left));
  EXPECT_FALSE(board.opening_index());
  ASSERT_TRUE(board.open({Number::SIX, Number::SIX}));
  EXPECT_FALSE(board.open({Number::ONE, Number::ONE}));
  EXPECT_FALSE(board.place({Number::ONE, Number::TWO}, BoardEnd::right));
  EXPECT_EQ(board.opening_index(), 0u);
  EXPECT_EQ(board.blocks().size(), 1u);
}

} // namespace dominoes
