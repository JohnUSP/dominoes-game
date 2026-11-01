#include "MatchViewModel.hpp"
#include "TileListModel.hpp"

#include <QAbstractItemModelTester>
#include <QSignalSpy>

#include <gtest/gtest.h>

#include <array>
#include <random>
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

} // namespace
} // namespace dominoes::gui
