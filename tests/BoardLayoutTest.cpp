#include "BoardLayout.hpp"
#include "BoardPreviewTiles.hpp"
#include "Match.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>

namespace dominoes::gui {
namespace {

constexpr double tolerance = 1e-7;

// K10 without a perfect matching has degree eight at every vertex, so its
// Euler circuit gives forty connected, unique, ordinary double-nine tiles.
std::vector<Block> forty_ordinary_tiles() {
  std::array<std::array<bool, 10>, 10> edges{};
  for (int a = 0; a < 10; ++a) {
    for (int b = 0; b < 10; ++b) edges[a][b] = a / 2 != b / 2;
  }
  std::vector<int> stack{0}, circuit;
  while (!stack.empty()) {
    const int a = stack.back();
    const auto next = std::find(edges[a].begin(), edges[a].end(), true);
    if (next == edges[a].end()) {
      circuit.push_back(a);
      stack.pop_back();
    } else {
      const int b = static_cast<int>(next - edges[a].begin());
      edges[a][b] = edges[b][a] = false;
      stack.push_back(b);
    }
  }
  std::reverse(circuit.begin(), circuit.end());
  std::vector<Block> tiles;
  for (std::size_t index = 1; index < circuit.size(); ++index) {
    tiles.emplace_back(static_cast<Number>(circuit[index - 1]),
                       static_cast<Number>(circuit[index]));
  }
  return tiles;
}
struct Rectangle { double left, top, right, bottom; };

Rectangle bounds(const BoardLayout &layout, const BoardTilePlacement &tile) {
  const bool horizontal = tile.rotation % 180 == 0;
  const double half_width = (horizontal ? layout.tile_width : layout.tile_height) / 2;
  const double half_height = (horizontal ? layout.tile_height : layout.tile_width) / 2;
  return {tile.center_x - half_width, tile.center_y - half_height,
          tile.center_x + half_width, tile.center_y + half_height};
}

std::pair<double, double> direction(int angle) {
  switch (angle) {
  case 0: return {1, 0};
  case 90: return {0, 1};
  case 180: return {-1, 0};
  default: return {0, -1};
  }
}

int outward_incoming(const BoardTilePlacement &tile, std::size_t index,
                     std::size_t opening) {
  return index < opening ? (tile.outgoing_direction + 180) % 360
                         : tile.incoming_direction;
}

std::pair<double, double> connection_half(const Block &block,
    const BoardTilePlacement &tile, bool incoming, bool left_arm, double face_size) {
  double x = tile.center_x;
  double y = tile.center_y;
  if (!block.is_double()) {
    const auto [dx, dy] = direction(tile.rotation);
    const double offset = incoming ? -face_size / 2 : face_size / 2;
    x += offset * dx;
    y += offset * dy;
  } else if (tile.incoming_direction != tile.outgoing_direction) {
    // A double is entered at the middle of its long side. If it turns, the
    // outward connection uses its end half. Reverse these roles on the left arm.
    if (left_arm && incoming) {
      const auto [dx, dy] = direction(tile.incoming_direction);
      x -= face_size / 2 * dx;
      y -= face_size / 2 * dy;
    } else if (!left_arm && !incoming) {
      const auto [dx, dy] = direction(tile.outgoing_direction);
      x += face_size / 2 * dx;
      y += face_size / 2 * dy;
    }
  }
  return {x, y};
}

void expect_valid(std::span<const Block> tiles, std::size_t opening,
                  const BoardLayout &layout, double width, double height) {
  ASSERT_EQ(layout.tiles.size(), tiles.size());
  ASSERT_GT(layout.tile_width, 0);
  ASSERT_GT(layout.tile_height, 0);
  EXPECT_NEAR(layout.tile_width, 2 * layout.tile_height, tolerance);
  ASSERT_LT(opening, layout.tiles.size());
  EXPECT_DOUBLE_EQ(layout.tiles[opening].center_x, width / 2);
  const double opening_y = height / 2 + layout.vertical_shift_steps * 2 * layout.tile_width;
  EXPECT_DOUBLE_EQ(layout.tiles[opening].center_y, opening_y);
  // Walk away from the opening, independently of logical engine ordering.
  // A complete connector has exactly two upright tiles before its return bend.
  for (const int step : {-1, 1}) {
    bool connecting = false;
    int vertical_tiles = 0;
    auto index = static_cast<std::ptrdiff_t>(opening) + step;
    for (; index >= 0 && index < static_cast<std::ptrdiff_t>(tiles.size()); index += step) {
      SCOPED_TRACE(index);
      const auto row = static_cast<std::size_t>(index);
      const auto &tile = layout.tiles[row];
      const int incoming = outward_incoming(tile, row, opening);
      const int outgoing = step < 0 ? (tile.incoming_direction + 180) % 360
                                    : tile.outgoing_direction;
      if (incoming % 180 == 0 && outgoing % 180 == 90) {
        EXPECT_FALSE(connecting);
        connecting = true;
        vertical_tiles = 0;
      } else if (incoming % 180 == 90 && outgoing % 180 == 0) {
        EXPECT_TRUE(connecting);
        EXPECT_EQ(vertical_tiles, 2);
        connecting = false;
      }
      if (connecting && tile.rotation % 180 == 90) ++vertical_tiles;
      EXPECT_LE(vertical_tiles, 2);
    }
  }
  for (std::size_t index = 0; index < tiles.size(); ++index) {
    SCOPED_TRACE(index);
    const auto &tile = layout.tiles[index];
    const auto rect = bounds(layout, tile);
    EXPECT_GE(rect.left, -tolerance);
    EXPECT_GE(rect.top, -tolerance);
    EXPECT_LE(rect.right, width + tolerance);
    EXPECT_LE(rect.bottom, height + tolerance);
    if (index < opening) EXPECT_GE(tile.center_y, opening_y - tolerance);
    if (index > opening) EXPECT_LE(tile.center_y, opening_y + tolerance);
    if (tiles[index].is_double()) {
      EXPECT_EQ((tile.rotation - outward_incoming(tile, index, opening) + 360) % 180, 90);
      if (index != opening) {
        const auto next = static_cast<std::ptrdiff_t>(index) + (index < opening ? -1 : 1);
        if (next < 0 || next >= static_cast<std::ptrdiff_t>(tiles.size())) {
          EXPECT_EQ(tile.incoming_direction, tile.outgoing_direction);
        }
      }
    }
    for (std::size_t other = 0; other < index; ++other) {
      const auto previous = bounds(layout, layout.tiles[other]);
      const double overlap_x = std::min(rect.right, previous.right) - std::max(rect.left, previous.left);
      const double overlap_y = std::min(rect.bottom, previous.bottom) - std::max(rect.top, previous.top);
      EXPECT_FALSE(overlap_x > tolerance && overlap_y > tolerance);
    }
    if (index == 0) continue;
    const auto &previous = layout.tiles[index - 1];
    EXPECT_EQ(previous.outgoing_direction, tile.incoming_direction);
    const auto [outgoing_x, outgoing_y] = connection_half(tiles[index - 1], previous,
        false, index - 1 < opening, layout.tile_height);
    const auto [incoming_x, incoming_y] = connection_half(tiles[index], tile,
        true, index < opening, layout.tile_height);
    const auto [join_dx, join_dy] = direction(previous.outgoing_direction);
    EXPECT_NEAR(incoming_x - outgoing_x, layout.tile_height * join_dx, tolerance);
    EXPECT_NEAR(incoming_y - outgoing_y, layout.tile_height * join_dy, tolerance);
  }
}

TEST(BoardLayoutTest, PreviewFixtureIsConnectedAndContainsFortyUniqueTiles) {
  const auto tiles = board_preview_tiles();
  ASSERT_EQ(tiles.size(), 40u);
  std::set<std::pair<int, int>> unique;
  for (std::size_t index = 0; index < tiles.size(); ++index) {
    const int left = to_value(tiles[index].left());
    const int right = to_value(tiles[index].right());
    EXPECT_TRUE(unique.emplace(std::min(left, right), std::max(left, right)).second);
    if (index != 0) EXPECT_EQ(tiles[index - 1].right(), tiles[index].left());
  }
}

TEST(BoardLayoutTest, CentersSingleTilesAndPlacesTheOpeningDoubleVertically) {
  for (const Block tile : {Block{Number::SEVEN, Number::NINE},
                          Block{Number::SIX, Number::SIX}}) {
    const std::array tiles{tile};
    const auto layout = layout_board(tiles, 0, 800, 400);
    expect_valid(tiles, 0, layout, 800, 400);
    EXPECT_EQ(layout.tiles[0].rotation, tile.is_double() ? 90 : 0);
  }
}

TEST(BoardLayoutTest, ShortChainGrowsToBothSidesOfTheActualOpening) {
  const std::array tiles{Block{Number::ONE, Number::TWO},
                        Block{Number::TWO, Number::THREE},
                        Block{Number::THREE, Number::FOUR}};
  const auto layout = layout_board(tiles, 1, 800, 400);
  expect_valid(tiles, 1, layout, 800, 400);
  EXPECT_LT(layout.tiles[0].center_x, 400);
  EXPECT_GT(layout.tiles[2].center_x, 400);
  for (const auto &tile : layout.tiles) {
    EXPECT_EQ(tile.rotation, 0);
    EXPECT_DOUBLE_EQ(tile.center_y, 200);
  }
  const auto opening = layout_board(std::span<const Block>{tiles}.subspan(1, 1), 0, 800, 400);
  EXPECT_DOUBLE_EQ(layout.tile_width, opening.tile_width);
}

TEST(BoardLayoutTest, MovesOpeningInTwoTileStepsWhenOnlyOneEndGrows) {
  const auto tiles = board_preview_tiles();
  for (const auto opening : {std::size_t{0}, tiles.size() - 1}) {
    const auto layout = layout_board(tiles, opening, 900, 600);
    expect_valid(tiles, opening, layout, 900, 600);
    if (opening == 0) EXPECT_GT(layout.vertical_shift_steps, 0);
    else EXPECT_LT(layout.vertical_shift_steps, 0);
  }
}

TEST(BoardLayoutTest, ShiftsTheWholeChainByTwoFixedTileLengthsAtEitherEdge) {
  const auto tiles = board_preview_tiles();
  for (const bool grow_left : {false, true}) {
    SCOPED_TRACE(grow_left);
    BoardLayout previous;
    bool shifted = false;
    for (std::size_t count = 1; count <= 40; ++count) {
      auto chain = std::span<const Block>{tiles};
      chain = grow_left ? chain.last(count) : chain.first(count);
      const auto opening = grow_left ? count - 1 : 0;
      const auto layout = layout_board(chain, opening, 900, 600, &previous);
      expect_valid(chain, opening, layout, 900, 600);
      if (count > 1) EXPECT_DOUBLE_EQ(layout.tile_width, previous.tile_width);
      if (count > 1 && layout.vertical_shift_steps != previous.vertical_shift_steps) {
        const int expected_step = grow_left ? -1 : 1;
        EXPECT_EQ(layout.vertical_shift_steps - previous.vertical_shift_steps, expected_step);
        EXPECT_DOUBLE_EQ(layout.half_lane, previous.half_lane);
        for (std::size_t index = 0; index < previous.tiles.size(); ++index) {
          const auto &before = previous.tiles[index];
          const auto &after = layout.tiles[index + (grow_left ? 1 : 0)];
          EXPECT_DOUBLE_EQ(after.center_x, before.center_x);
          EXPECT_DOUBLE_EQ(after.center_y - before.center_y, expected_step * 2 * layout.tile_width);
          EXPECT_EQ(after.rotation, before.rotation);
        }
        shifted = true;
        break;
      }
      previous = layout;
    }
    EXPECT_TRUE(shifted);
  }
}

TEST(BoardLayoutTest, DoesNotFreezeTheOpeningTilesUnusedRowWidth) {
  const auto tiles = board_preview_tiles();
  BoardLayout previous;
  for (std::size_t count = 1; count <= 10; ++count) {
    const auto chain = std::span<const Block>{tiles}.first(count);
    auto layout = layout_board(chain, 0, 1600, 1000, &previous);
    expect_valid(chain, 0, layout, 1600, 1000);
    for (const auto &tile : layout.tiles) {
      EXPECT_EQ(tile.incoming_direction, tile.outgoing_direction);
      EXPECT_DOUBLE_EQ(tile.center_y, 500);
    }
    previous = std::move(layout);
  }
}

TEST(BoardLayoutTest, KeepsFullSizeThroughFortyTilesOnASufficientlyLargeBoard) {
  BoardLayout previous;
  for (std::size_t count = 1; count <= 40; ++count) {
    const auto chain = board_preview_chain(count);
    auto layout = layout_board(chain.tiles, chain.opening_index, 1600, 1000, &previous);
    expect_valid(chain.tiles, chain.opening_index, layout, 1600, 1000);
    EXPECT_DOUBLE_EQ(layout.tile_width, 96);
    EXPECT_DOUBLE_EQ(layout.tile_height, 48);
    previous = std::move(layout);
  }
}

TEST(BoardLayoutTest, ChoosesTheCompleteHandSizeBeforeTheFirstMoveOnASmallBoard) {
  const auto tiles = board_preview_tiles();
  const auto initial = layout_board(std::span<const Block>{tiles}.subspan(12, 1), 0, 260, 140);
  const auto layout = layout_board(tiles, 12, 260, 140);
  expect_valid(tiles, 12, layout, 260, 140);
  EXPECT_LT(layout.tile_width, 96);
  EXPECT_DOUBLE_EQ(layout.tile_width, initial.tile_width);
  EXPECT_DOUBLE_EQ(layout.tile_height, initial.tile_height);
}

TEST(BoardLayoutTest, ReservesFortyOrdinaryTilesAtEveryOpeningAndNeverChangesTheirSize) {
  const auto tiles = forty_ordinary_tiles();
  ASSERT_EQ(tiles.size(), 40u);
  std::set<std::pair<int, int>> unique;
  for (std::size_t index = 0; index < tiles.size(); ++index) {
    ASSERT_FALSE(tiles[index].is_double());
    if (index > 0) ASSERT_EQ(tiles[index - 1].right(), tiles[index].left());
    const auto a = to_value(tiles[index].left());
    const auto b = to_value(tiles[index].right());
    ASSERT_TRUE(unique.emplace(std::min(a, b), std::max(a, b)).second);
  }
  for (const auto [width, height] : {std::pair{260., 140.}, {900., 600.},
                                   {420., 700.}, {1600., 1000.}}) {
    const auto initial = layout_board(std::span<const Block>{tiles}.first(1), 0, width, height);
    for (std::size_t opening = 0; opening < 40; ++opening) {
      SCOPED_TRACE(opening);
      const auto layout = layout_board(tiles, opening, width, height);
      expect_valid(tiles, opening, layout, width, height);
      EXPECT_DOUBLE_EQ(layout.tile_width, initial.tile_width);
      EXPECT_DOUBLE_EQ(layout.tile_height, initial.tile_height);
    }
    for (const bool grow_left : {false, true}) {
      BoardLayout previous;
      for (std::size_t count = 1; count <= 40; ++count) {
        auto chain = std::span<const Block>{tiles};
        chain = grow_left ? chain.last(count) : chain.first(count);
        auto layout = layout_board(chain, grow_left ? count - 1 : 0, width, height, &previous);
        expect_valid(chain, grow_left ? count - 1 : 0, layout, width, height);
        EXPECT_DOUBLE_EQ(layout.tile_width, initial.tile_width);
        previous = std::move(layout);
      }
    }
  }
}

TEST(BoardLayoutTest, PreviewExpandsAroundTheSameOpeningInAllSizes) {
  for (const auto [width, height] : {std::pair{900., 420.}, {420., 700.},
                                   {260., 140.}, {1200., 700.}}) {
    const auto one = board_preview_chain(1);
    const auto initial = layout_board(one.tiles, 0, width, height);
    for (std::size_t count = 1; count <= 40; ++count) {
      SCOPED_TRACE(count);
      const auto chain = board_preview_chain(count);
      EXPECT_EQ(chain.tiles[chain.opening_index].left(), Number::SIX);
      EXPECT_EQ(chain.tiles[chain.opening_index].right(), Number::SIX);
      const auto layout = layout_board(chain.tiles, chain.opening_index, width, height);
      expect_valid(chain.tiles, chain.opening_index, layout, width, height);
      EXPECT_DOUBLE_EQ(layout.tile_width, initial.tile_width);
      EXPECT_DOUBLE_EQ(layout.tile_height, initial.tile_height);
    }
  }
}

TEST(BoardLayoutTest, FitsWhenEitherEndGrowsOrTheViewportChanges) {
  const auto tiles = board_preview_tiles();
  const auto chain = std::span<const Block>{tiles};
  expect_valid(chain.subspan(1, 38), 11,
      layout_board(chain.subspan(1, 38), 11, 700, 350), 700, 350);
  expect_valid(chain.first(39), 12,
      layout_board(chain.first(39), 12, 700, 350), 700, 350);
  expect_valid(chain, 12, layout_board(chain, 12, 350, 700), 350, 700);
}

TEST(BoardLayoutTest, NormalCornersJoinAtTheIncomingHalfAsInReferencePhotos) {
  auto tiles = board_preview_tiles();
  std::erase_if(tiles, [](const auto &tile) { return tile.is_double(); });
  const std::size_t opening = tiles.size() / 2;
  const auto layout = layout_board(tiles, opening, 650, 500);
  expect_valid(tiles, opening, layout, 650, 500);
  int corners = 0;
  for (std::size_t index = 0; index < tiles.size(); ++index) {
    const auto &tile = layout.tiles[index];
    if (tile.incoming_direction == tile.outgoing_direction) continue;
    ++corners;
    // Reversing traversal on the left arm also reverses entry and exit roles.
    EXPECT_EQ(tile.rotation, index < opening ? tile.incoming_direction : tile.outgoing_direction);
  }
  EXPECT_GT(corners, 0);
}

TEST(BoardLayoutTest, DoubleCornersKeepTheOrientationOfTheirIncomingRun) {
  const auto tiles = board_preview_tiles();
  bool entered_horizontally = false;
  bool entered_vertically = false;
  for (std::size_t opening = 0; opening < tiles.size(); ++opening) {
    const auto layout = layout_board(tiles, opening, 650, 500);
    expect_valid(tiles, opening, layout, 650, 500);
    for (std::size_t index = 0; index < tiles.size(); ++index) {
      const auto &tile = layout.tiles[index];
      if (!tiles[index].is_double() || tile.incoming_direction == tile.outgoing_direction) continue;
      EXPECT_EQ((tile.rotation - outward_incoming(tile, index, opening) + 360) % 180, 90);
      entered_horizontally |= tile.incoming_direction % 180 == 0;
      entered_vertically |= tile.incoming_direction % 180 == 90;
    }
    if (entered_horizontally && entered_vertically) break;
  }
  EXPECT_TRUE(entered_horizontally);
  EXPECT_TRUE(entered_vertically);
}

TEST(BoardLayoutTest, ReportedDoubleTwoStaysVerticalOnTheIncomingHorizontalRun) {
  const auto tiles = board_preview_tiles();
  // This chain and viewport put 2|2 at the first left bend. Other sizes may
  // choose different row lengths and consequently different corner tiles.
  const auto layout = layout_board(std::span<const Block>{tiles}.first(13), 12, 900, 600);
  ASSERT_EQ(tiles[4].left(), Number::TWO);
  ASSERT_EQ(tiles[4].right(), Number::TWO);
  // This is the 2|2 at the left-hand corner in the user's screenshot.
  EXPECT_EQ(layout.tiles[4].rotation % 180, 90);
  EXPECT_EQ(layout.tiles[3].rotation % 180, 90);
  EXPECT_NEAR(layout.tiles[4].center_x, layout.tiles[3].center_x, tolerance);
  EXPECT_GT(layout.tiles[3].center_y, layout.tiles[4].center_y);
}

TEST(BoardLayoutTest, DoubleZeroIsCenteredAcrossTheReturningHorizontalRun) {
  const auto tiles = board_preview_tiles();
  const auto layout = layout_board(std::span<const Block>{tiles}.first(13), 12, 900, 600);
  ASSERT_EQ(tiles[0].left(), Number::ZERO);
  ASSERT_EQ(tiles[0].right(), Number::ZERO);
  EXPECT_EQ(layout.tiles[0].rotation % 180, 90);
  EXPECT_EQ(layout.tiles[1].rotation % 180, 0);
  EXPECT_NEAR(layout.tiles[0].center_y, layout.tiles[1].center_y, tolerance);
  EXPECT_NEAR(layout.tiles[0].center_x - layout.tiles[1].center_x,
              1.5 * layout.tile_height, tolerance);
}

TEST(BoardLayoutTest, ReportedOneZeroTurnsRightImmediatelyAfterDoubleOne) {
  const auto tiles = board_preview_tiles();
  const auto layout = layout_board(std::span<const Block>{tiles}.first(13), 12, 900, 600);
  ASSERT_EQ(tiles[1].left(), Number::ZERO);
  ASSERT_EQ(tiles[1].right(), Number::ONE);
  const auto &double_one = layout.tiles[2];
  const auto &one_zero = layout.tiles[1];
  EXPECT_EQ(double_one.rotation % 180, 0);
  EXPECT_EQ(one_zero.rotation % 180, 0);
  EXPECT_EQ(outward_incoming(double_one, 2, 12), 90);
  EXPECT_EQ((double_one.incoming_direction + 180) % 360, 0);
  EXPECT_EQ(outward_incoming(one_zero, 1, 12), 0);
  EXPECT_EQ((one_zero.incoming_direction + 180) % 360, 0);
  EXPECT_NEAR(one_zero.center_x, double_one.center_x + layout.tile_width, tolerance);
  EXPECT_NEAR(one_zero.center_y, double_one.center_y, tolerance);
}

TEST(BoardLayoutTest, KeepsTwoTileConnectorsAndFitsDifferentBoardSizesAndOpenings) {
  const auto tiles = board_preview_tiles();
  for (const double width : {180., 420., 900., 1600.}) {
    for (const double height : {120., 300., 600., 1000.}) {
      for (const std::size_t opening : {0u, 12u, 20u, 39u}) {
        SCOPED_TRACE(width);
        SCOPED_TRACE(height);
        SCOPED_TRACE(opening);
        expect_valid(tiles, opening, layout_board(tiles, opening, width, height), width, height);
      }
    }
  }
}

TEST(BoardLayoutTest, FitsChainsProducedByActualPlayAtEitherEnd) {
  for (unsigned seed = 0; seed < 8; ++seed) {
    std::mt19937 engine{seed};
    Match match{{"A", "B", "C", "D"}, engine};
    for (int turn = 0; turn < 200 && !match.current_hand().hand_result(); ++turn) {
      const auto &game = match.current_hand();
      const auto &hand = match.players()[game.current_player_index()].hand();
      bool played = false;
      for (std::size_t index = 0; index < hand.size() && !played; ++index) {
        const auto first = turn % 2 == 0 ? BoardEnd::left : BoardEnd::right;
        const auto second = first == BoardEnd::left ? BoardEnd::right : BoardEnd::left;
        for (const auto end : {first, second}) {
          if (game.board().blocks().empty() || game.board().can_place(hand[index], end)) {
            ASSERT_NE(match.play({index, end}), ActionResult::rejected);
            played = true;
            break;
          }
        }
      }
      if (!played) ASSERT_NE(match.pass(), ActionResult::rejected);
      const auto &blocks = game.board().blocks();
      const std::vector<Block> chain{blocks.begin(), blocks.end()};
      ASSERT_TRUE(game.board().opening_index());
      const auto opening = *game.board().opening_index();
      expect_valid(chain, opening, layout_board(chain, opening, 850, 360), 850, 360);
    }
    EXPECT_TRUE(match.current_hand().hand_result());
  }
}

TEST(BoardLayoutTest, EmptyBoardAndInvalidViewportOrOpeningAreHandledExplicitly) {
  EXPECT_TRUE(layout_board({}, 0, 800, 400).tiles.empty());
  EXPECT_THROW(layout_board({}, 0, 0, 400), std::invalid_argument);
  EXPECT_THROW(layout_board({}, 0, 800, -1), std::invalid_argument);
  EXPECT_THROW(layout_board({}, 0, std::numeric_limits<double>::infinity(), 400), std::invalid_argument);
  const std::array tiles{Block{Number::SIX, Number::SIX}};
  EXPECT_THROW(layout_board(tiles, 1, 800, 400), std::out_of_range);
  const std::vector<Block> too_many(41, {Number::ZERO, Number::ONE});
  EXPECT_THROW(layout_board(too_many, 0, 800, 400), std::out_of_range);
  EXPECT_THROW(board_preview_chain(0), std::out_of_range);
  EXPECT_THROW(board_preview_chain(41), std::out_of_range);
}

} // namespace
} // namespace dominoes::gui
