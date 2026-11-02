#include "MatchViewModel.hpp"
#include "TileListModel.hpp"
#include "BoardPreviewTiles.hpp"

#include <QAbstractItemModelTester>
#include <QSignalSpy>

#include <gtest/gtest.h>

#include <array>
#include <random>
#include <optional>
#include <string>
#include <vector>

namespace dominoes::gui {
namespace {

std::array<std::string, 4> names() { return {"John", "Kyle", "Jane", "Kate"}; }

TEST(TileListModelTest, ExposesReadOnlyRolesAndRejectsInvalidIndices) {
  TileListModel model;
  QAbstractItemModelTester tester{
      &model, QAbstractItemModelTester::FailureReportingMode::Fatal};
  EXPECT_EQ(model.rowCount(), 0);

  const std::array tiles{Block{Number::SEVEN, Number::NINE}};
  model.set_tiles(tiles);
  ASSERT_EQ(model.rowCount(), 1);
  const auto index = model.index(0, 0);
  EXPECT_EQ(model.data(index, TileListModel::LeftValueRole).toInt(), 7);
  EXPECT_EQ(model.data(index, TileListModel::RightValueRole).toInt(), 9);
  EXPECT_EQ(model.roleNames().value(TileListModel::LeftValueRole), "leftValue");
  EXPECT_EQ(model.roleNames().value(TileListModel::RightValueRole), "rightValue");
  EXPECT_EQ(model.rowCount(index), 0);
  EXPECT_FALSE(model.data({}, TileListModel::LeftValueRole).isValid());
  EXPECT_FALSE(model.data(model.index(1, 0), TileListModel::LeftValueRole).isValid());
  EXPECT_FALSE(model.data(model.index(0, 1), TileListModel::LeftValueRole).isValid());
  EXPECT_FALSE(model.data(index, Qt::DisplayRole).isValid());
  EXPECT_FALSE(model.flags(index).testFlag(Qt::ItemIsEditable));
  EXPECT_FALSE(model.setData(index, 3, TileListModel::LeftValueRole));

  TileListModel other;
  other.set_tiles(tiles);
  EXPECT_FALSE(model.data(other.index(0, 0), TileListModel::LeftValueRole).isValid());
}

TEST(TileListModelTest, SnapshotSurvivesChangesAndDestructionOfSourceTiles) {
  TileListModel model;
  {
    std::vector tiles{Block{Number::SEVEN, Number::NINE},
                      Block{Number::ZERO, Number::ONE}};
    model.set_tiles(tiles);
    tiles[0].flip();
    tiles.clear();
  }

  ASSERT_EQ(model.rowCount(), 2);
  EXPECT_EQ(model.data(model.index(0, 0), TileListModel::LeftValueRole).toInt(), 7);
  EXPECT_EQ(model.data(model.index(0, 0), TileListModel::RightValueRole).toInt(), 9);
  EXPECT_EQ(model.data(model.index(1, 0), TileListModel::LeftValueRole).toInt(), 0);
}

TEST(TileListModelTest, RefreshNotifiesViewsAndCanClearAllRows) {
  TileListModel model;
  QAbstractItemModelTester tester{
      &model, QAbstractItemModelTester::FailureReportingMode::Fatal};
  QSignalSpy resets{&model, &QAbstractItemModel::modelReset};
  const std::array tiles{Block{Number::ZERO, Number::NINE}};

  model.set_tiles(tiles);
  EXPECT_EQ(resets.count(), 1);
  model.set_tiles({});
  EXPECT_EQ(resets.count(), 2);
  EXPECT_EQ(model.rowCount(), 0);
}

TEST(MatchViewModelTest, DisplaysTheEngineCurrentPlayersTenTiles) {
  std::mt19937 expected_engine{42};
  Match expected{names(), expected_engine};
  MatchViewModel view_model{names(), std::mt19937{42}};
  const auto &player = expected.players()[expected.current_hand().current_player_index()];

  EXPECT_EQ(view_model.currentPlayerName().toStdString(), player.name());
  ASSERT_EQ(view_model.hand()->rowCount(), 10);
  for (int row = 0; row < view_model.hand()->rowCount(); ++row) {
    const auto index = view_model.hand()->index(row, 0);
    const auto &tile = player.hand()[static_cast<std::size_t>(row)];
    EXPECT_EQ(view_model.hand()->data(index, TileListModel::LeftValueRole).toInt(),
              to_value(tile.left()));
    EXPECT_EQ(view_model.hand()->data(index, TileListModel::RightValueRole).toInt(),
              to_value(tile.right()));
  }
  EXPECT_EQ(view_model.selectedHandIndex(), -1);
  EXPECT_EQ(view_model.board()->rowCount(), 0);
  EXPECT_TRUE(view_model.openingAvailable());
}

TEST(MatchViewModelTest, SelectionTogglesAndMovesWithoutChangingTheHand) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  QSignalSpy changes{&view_model, &MatchViewModel::selectionChanged};
  QSignalSpy resets{view_model.hand(), &QAbstractItemModel::modelReset};

  EXPECT_TRUE(view_model.toggleSelection(2));
  EXPECT_EQ(view_model.selectedHandIndex(), 2);
  EXPECT_TRUE(view_model.toggleSelection(4));
  EXPECT_EQ(view_model.selectedHandIndex(), 4);
  EXPECT_TRUE(view_model.toggleSelection(4));
  EXPECT_EQ(view_model.selectedHandIndex(), -1);
  EXPECT_EQ(changes.count(), 3);
  EXPECT_EQ(view_model.hand()->rowCount(), 10);
  EXPECT_EQ(resets.count(), 0);
}

TEST(MatchViewModelTest, InvalidSelectionDoesNotChangeStateOrEmitNotification) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  ASSERT_TRUE(view_model.toggleSelection(3));
  QSignalSpy changes{&view_model, &MatchViewModel::selectionChanged};

  EXPECT_FALSE(view_model.toggleSelection(-1));
  EXPECT_FALSE(view_model.toggleSelection(10));
  EXPECT_EQ(view_model.selectedHandIndex(), 3);
  EXPECT_EQ(changes.count(), 0);
}

TEST(MatchViewModelTest, ReplacingTheDisplayedHandClearsSelection) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  ASSERT_TRUE(view_model.toggleSelection(2));
  QSignalSpy changes{&view_model, &MatchViewModel::selectionChanged};
  const std::array replacement{Block{Number::SIX, Number::SEVEN}};

  view_model.hand()->set_tiles(replacement);
  EXPECT_EQ(view_model.selectedHandIndex(), -1);
  EXPECT_EQ(changes.count(), 1);
  EXPECT_EQ(view_model.hand()->rowCount(), 1);
  // A refresh with no selection does not send a redundant selection notification.
  view_model.hand()->set_tiles({});
  EXPECT_EQ(changes.count(), 1);
}

TEST(MatchViewModelTest, OpeningWithoutSelectionOnlyUpdatesFeedback) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  const auto player = view_model.currentPlayerName();
  QSignalSpy state_changes{&view_model, &MatchViewModel::gameStateChanged};
  QSignalSpy feedback_changes{&view_model, &MatchViewModel::feedbackChanged};
  QSignalSpy hand_resets{view_model.hand(), &QAbstractItemModel::modelReset};
  QSignalSpy board_resets{view_model.board(), &QAbstractItemModel::modelReset};

  EXPECT_FALSE(view_model.playOpening());
  EXPECT_EQ(view_model.currentPlayerName(), player);
  EXPECT_EQ(view_model.hand()->rowCount(), 10);
  EXPECT_EQ(view_model.board()->rowCount(), 0);
  EXPECT_EQ(view_model.selectedHandIndex(), -1);
  EXPECT_TRUE(view_model.openingAvailable());
  EXPECT_EQ(view_model.feedback(), "Select a tile before playing.");
  EXPECT_EQ(feedback_changes.count(), 1);
  EXPECT_EQ(state_changes.count(), 0);
  EXPECT_EQ(hand_resets.count(), 0);
  EXPECT_EQ(board_resets.count(), 0);
}

TEST(MatchViewModelTest, OpeningUpdatesBoardPlayerHandAndSelectionFromEngine) {
  std::mt19937 expected_engine{42};
  Match expected{names(), expected_engine};
  const auto opening = expected.current_hand().current_player_index();
  const Block played_tile = expected.players()[opening].hand()[3];
  ASSERT_EQ(expected.play({3}), ActionResult::accepted);
  ASSERT_EQ(expected.players()[opening].hand().size(), 9u);
  const auto &next_player = expected.players()[expected.current_hand().current_player_index()];

  MatchViewModel view_model{names(), std::mt19937{42}};
  ASSERT_TRUE(view_model.toggleSelection(3));
  QSignalSpy state_changes{&view_model, &MatchViewModel::gameStateChanged};
  QSignalSpy selection_changes{&view_model, &MatchViewModel::selectionChanged};
  QSignalSpy hand_resets{view_model.hand(), &QAbstractItemModel::modelReset};
  QSignalSpy board_resets{view_model.board(), &QAbstractItemModel::modelReset};

  ASSERT_TRUE(view_model.playOpening());
  EXPECT_EQ(view_model.currentPlayerName().toStdString(), next_player.name());
  ASSERT_EQ(view_model.board()->rowCount(), 1);
  EXPECT_EQ(view_model.board()->data(view_model.board()->index(0, 0),
                                   BoardTileModel::LeftValueRole).toInt(),
            to_value(played_tile.left()));
  EXPECT_EQ(view_model.board()->data(view_model.board()->index(0, 0),
                                   BoardTileModel::RightValueRole).toInt(),
            to_value(played_tile.right()));
  ASSERT_EQ(view_model.hand()->rowCount(), 10);
  for (int row = 0; row < view_model.hand()->rowCount(); ++row) {
    const auto index = view_model.hand()->index(row, 0);
    EXPECT_EQ(view_model.hand()->data(index, TileListModel::LeftValueRole).toInt(),
              to_value(next_player.hand()[static_cast<std::size_t>(row)].left()));
    EXPECT_EQ(view_model.hand()->data(index, TileListModel::RightValueRole).toInt(),
              to_value(next_player.hand()[static_cast<std::size_t>(row)].right()));
  }
  EXPECT_EQ(view_model.selectedHandIndex(), -1);
  EXPECT_FALSE(view_model.openingAvailable());
  EXPECT_EQ(state_changes.count(), 1);
  EXPECT_EQ(selection_changes.count(), 1);
  EXPECT_EQ(hand_resets.count(), 1);
  EXPECT_EQ(board_resets.count(), 1);
  EXPECT_TRUE(view_model.feedback().contains(view_model.currentPlayerName()));
}

TEST(MatchViewModelTest, RepeatedOpeningRequestPreservesTheNextPlayersState) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  ASSERT_TRUE(view_model.toggleSelection(0));
  ASSERT_TRUE(view_model.playOpening());
  ASSERT_TRUE(view_model.toggleSelection(2));
  const auto player = view_model.currentPlayerName();
  QSignalSpy state_changes{&view_model, &MatchViewModel::gameStateChanged};
  QSignalSpy hand_resets{view_model.hand(), &QAbstractItemModel::modelReset};
  QSignalSpy board_resets{view_model.board(), &QAbstractItemModel::modelReset};

  EXPECT_FALSE(view_model.playOpening());
  EXPECT_EQ(view_model.currentPlayerName(), player);
  EXPECT_EQ(view_model.selectedHandIndex(), 2);
  EXPECT_EQ(view_model.hand()->rowCount(), 10);
  EXPECT_EQ(view_model.board()->rowCount(), 1);
  EXPECT_EQ(view_model.feedback(), "The hand is already open.");
  EXPECT_EQ(state_changes.count(), 0);
  EXPECT_EQ(hand_resets.count(), 0);
  EXPECT_EQ(board_resets.count(), 0);
}

TEST(MatchViewModelTest, EngineRejectsAnIndexFromAnInvalidDisplaySnapshot) {
  MatchViewModel view_model{names(), std::mt19937{42}};
  // Simulate a stale projection containing an index absent from the real hand.
  const std::vector stale_tiles(11, Block{Number::ZERO, Number::ZERO});
  view_model.hand()->set_tiles(stale_tiles);
  ASSERT_TRUE(view_model.toggleSelection(10));
  const auto player = view_model.currentPlayerName();
  QSignalSpy state_changes{&view_model, &MatchViewModel::gameStateChanged};
  QSignalSpy hand_resets{view_model.hand(), &QAbstractItemModel::modelReset};
  QSignalSpy board_resets{view_model.board(), &QAbstractItemModel::modelReset};

  EXPECT_FALSE(view_model.playOpening());
  EXPECT_EQ(view_model.currentPlayerName(), player);
  EXPECT_TRUE(view_model.openingAvailable());
  EXPECT_EQ(view_model.board()->rowCount(), 0);
  EXPECT_EQ(view_model.hand()->rowCount(), 11);
  EXPECT_EQ(view_model.selectedHandIndex(), 10);
  EXPECT_EQ(view_model.feedback(), "The selected tile cannot be played.");
  EXPECT_EQ(state_changes.count(), 0);
  EXPECT_EQ(hand_resets.count(), 0);
  EXPECT_EQ(board_resets.count(), 0);
}

TEST(BoardTileModelTest, ExposesCopiedValuesAndLayoutRoles) {
  BoardTileModel model;
  QAbstractItemModelTester tester{
      &model, QAbstractItemModelTester::FailureReportingMode::Fatal};
  QSignalSpy count_changes{&model, &BoardTileModel::countChanged};
  {
    std::vector tiles{Block{Number::SIX, Number::SIX}};
    model.set_tiles(tiles, 0);
    tiles.clear();
  }
  ASSERT_EQ(model.count(), 1);
  const auto index = model.index(0, 0);
  EXPECT_EQ(model.data(index, BoardTileModel::LeftValueRole).toInt(), 6);
  EXPECT_EQ(model.data(index, BoardTileModel::RightValueRole).toInt(), 6);
  EXPECT_EQ(model.data(index, BoardTileModel::AngleRole).toInt(), 90);
  EXPECT_GT(model.data(index, BoardTileModel::TileWidthRole).toDouble(), 0);
  EXPECT_GT(model.data(index, BoardTileModel::TileHeightRole).toDouble(), 0);
  EXPECT_EQ(model.roleNames().value(BoardTileModel::CenterXRole), "centerX");
  EXPECT_EQ(model.rowCount(index), 0);
  EXPECT_FALSE(model.data({}, BoardTileModel::AngleRole).isValid());
  EXPECT_FALSE(model.data(index, Qt::DisplayRole).isValid());
  EXPECT_FALSE(model.setData(index, 4, BoardTileModel::LeftValueRole));
  EXPECT_EQ(count_changes.count(), 1);
  model.set_tiles({}, std::nullopt);
  EXPECT_EQ(model.count(), 0);
  EXPECT_EQ(count_changes.count(), 2);
}

TEST(BoardTileModelTest, ViewportChangesReflowWithoutChangingValuesOrCount) {
  BoardTileModel model;
  QAbstractItemModelTester tester{
      &model, QAbstractItemModelTester::FailureReportingMode::Fatal};
  model.set_tiles(board_preview_tiles(), 12);
  QSignalSpy resets{&model, &QAbstractItemModel::modelReset};
  QSignalSpy count_changes{&model, &BoardTileModel::countChanged};
  model.setViewport(300, 600);
  ASSERT_EQ(model.count(), 40);
  EXPECT_DOUBLE_EQ(model.data(model.index(12, 0), BoardTileModel::CenterXRole).toDouble(), 150);
  EXPECT_DOUBLE_EQ(model.data(model.index(12, 0), BoardTileModel::CenterYRole).toDouble(), 300);
  EXPECT_EQ(resets.count(), 1);
  EXPECT_EQ(count_changes.count(), 0);
  const auto fixture = board_preview_tiles();
  for (int row = 0; row < model.count(); ++row) {
    const auto index = model.index(row, 0);
    EXPECT_EQ(model.data(index, BoardTileModel::LeftValueRole).toInt(),
              to_value(fixture[static_cast<std::size_t>(row)].left()));
    EXPECT_GE(model.data(index, BoardTileModel::CenterXRole).toDouble(), 0);
    EXPECT_LE(model.data(index, BoardTileModel::CenterXRole).toDouble(), 300);
    EXPECT_GE(model.data(index, BoardTileModel::CenterYRole).toDouble(), 0);
    EXPECT_LE(model.data(index, BoardTileModel::CenterYRole).toDouble(), 600);
  }
  model.setViewport(300, 600);
  model.setViewport(0, 600);
  EXPECT_EQ(resets.count(), 1);
}

TEST(BoardTileModelTest, RetainsOpeningAnchorWhenTheLeftEndAndViewportChange) {
  Board board;
  ASSERT_TRUE(board.open({Number::SIX, Number::SIX}));
  ASSERT_TRUE(board.place({Number::TWO, Number::SIX}, BoardEnd::left));
  ASSERT_TRUE(board.place({Number::SIX, Number::NINE}, BoardEnd::right));
  BoardTileModel model;
  model.setViewport(500, 300);
  const auto update = [&] {
    const std::vector<Block> tiles{board.blocks().begin(), board.blocks().end()};
    model.set_tiles(tiles, board.opening_index());
  };
  update();
  EXPECT_DOUBLE_EQ(model.data(model.index(1, 0), BoardTileModel::CenterXRole).toDouble(), 250);
  EXPECT_DOUBLE_EQ(model.data(model.index(1, 0), BoardTileModel::CenterYRole).toDouble(), 150);
  ASSERT_TRUE(board.place({Number::THREE, Number::TWO}, BoardEnd::left));
  update();
  EXPECT_DOUBLE_EQ(model.data(model.index(2, 0), BoardTileModel::CenterXRole).toDouble(), 250);
  EXPECT_DOUBLE_EQ(model.data(model.index(2, 0), BoardTileModel::CenterYRole).toDouble(), 150);
  model.setViewport(800, 500);
  EXPECT_DOUBLE_EQ(model.data(model.index(2, 0), BoardTileModel::CenterXRole).toDouble(), 400);
  EXPECT_DOUBLE_EQ(model.data(model.index(2, 0), BoardTileModel::CenterYRole).toDouble(), 250);
}

TEST(BoardTileModelTest, InvalidOpeningMetadataDoesNotReplaceTheSnapshot) {
  BoardTileModel model;
  const std::array tiles{Block{Number::SIX, Number::SIX}};
  model.set_tiles(tiles, 0);
  QSignalSpy resets{&model, &QAbstractItemModel::modelReset};
  EXPECT_THROW(model.set_tiles(tiles, std::nullopt), std::invalid_argument);
  EXPECT_THROW(model.set_tiles(tiles, 1), std::out_of_range);
  EXPECT_THROW(model.set_tiles({}, 0), std::invalid_argument);
  EXPECT_EQ(resets.count(), 0);
  EXPECT_EQ(model.count(), 1);
  EXPECT_EQ(model.data(model.index(0, 0), BoardTileModel::LeftValueRole).toInt(), 6);
}

TEST(BoardTileModelTest, RetainsShiftThroughGrowthAndLeftInsertionThenCentersANewHand) {
  BoardTileModel model;
  QAbstractItemModelTester tester{
      &model, QAbstractItemModelTester::FailureReportingMode::Fatal};
  model.setViewport(900, 600);
  const auto tiles = board_preview_tiles();
  const auto source = std::span<const Block>{tiles};
  double fixed_width = 0;
  for (std::size_t count = 1; count <= 39; ++count) {
    model.set_tiles(source.first(count), 0);
    const auto width = model.data(model.index(0, 0), BoardTileModel::TileWidthRole).toDouble();
    if (count == 1) fixed_width = width;
    EXPECT_DOUBLE_EQ(width, fixed_width);
  }
  const auto value = [&](int row, int role) {
    return model.data(model.index(row, 0), role).toDouble();
  };
  EXPECT_DOUBLE_EQ(value(0, BoardTileModel::TileWidthRole), fixed_width);
  EXPECT_DOUBLE_EQ(value(0, BoardTileModel::CenterYRole), 300 + 2 * fixed_width);
  // A repeated snapshot must preserve geometry, not just the offset.
  std::vector<std::pair<double, double>> positions;
  for (int row = 0; row < model.count(); ++row) {
    positions.emplace_back(value(row, BoardTileModel::CenterXRole),
                           value(row, BoardTileModel::CenterYRole));
  }
  model.set_tiles(source.first(39), 0);
  for (int row = 0; row < model.count(); ++row) {
    EXPECT_DOUBLE_EQ(value(row, BoardTileModel::CenterXRole), positions[row].first);
    EXPECT_DOUBLE_EQ(value(row, BoardTileModel::CenterYRole), positions[row].second);
  }
  std::vector<Block> extended{{Number::FIVE, Number::ZERO}};
  extended.insert(extended.end(), tiles.begin(), tiles.begin() + 39);
  model.set_tiles(extended, 1);
  EXPECT_DOUBLE_EQ(value(1, BoardTileModel::CenterYRole), 300 + 2 * fixed_width);
  EXPECT_DOUBLE_EQ(value(1, BoardTileModel::TileWidthRole), fixed_width);
  model.setViewport(1800, 1200);
  const double resized_width = value(1, BoardTileModel::TileWidthRole);
  EXPECT_DOUBLE_EQ(value(1, BoardTileModel::CenterYRole), 600 + 2 * resized_width);
  model.set_tiles({}, std::nullopt);
  model.set_tiles(source.first(1), 0);
  EXPECT_DOUBLE_EQ(value(0, BoardTileModel::CenterXRole), 900);
  EXPECT_DOUBLE_EQ(value(0, BoardTileModel::CenterYRole), 600);
}

} // namespace
} // namespace dominoes::gui
