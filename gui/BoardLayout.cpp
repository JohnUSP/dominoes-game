#include "BoardLayout.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <map>
#include <optional>
#include <stdexcept>

namespace dominoes::gui {
namespace {

constexpr double epsilon = 1e-9;
constexpr double face_size = 48;
constexpr double shift_step = 4; // Two long tile edges in face widths.
constexpr std::size_t maximum_tiles = 40;

struct Point { double x, y; };

Point direction(int angle) {
  switch (angle) {
  case 0: return {1, 0};
  case 90: return {0, 1};
  case 180: return {-1, 0};
  default: return {0, -1};
  }
}

struct DraftTile {
  Point center;
  int rotation;
  int incoming;
  int outgoing;
  bool is_double;
};

struct Bounds { double left, top, right, bottom; };

Bounds bounds(const DraftTile &tile) {
  const bool horizontal = tile.rotation % 180 == 0;
  const double half_width = horizontal ? 1 : 0.5;
  const double half_height = horizontal ? 0.5 : 1;
  return {tile.center.x - half_width, tile.center.y - half_height,
          tile.center.x + half_width, tile.center.y + half_height};
}

bool overlaps(const Bounds &a, const Bounds &b) {
  return std::min(a.right, b.right) - std::max(a.left, b.left) > epsilon &&
         std::min(a.bottom, b.bottom) - std::max(a.top, b.top) > epsilon;
}

DraftTile next_tile(const DraftTile &previous, bool is_double, int outgoing) {
  const int incoming = previous.outgoing;
  // A double always crosses the incoming run. Turning changes its exit port,
  // not its orientation or the centered connection from the preceding tile.
  const int rotation = is_double ? (incoming + 90) % 360 : outgoing;
  const auto previous_axis = direction(previous.rotation);
  const auto incoming_axis = direction(incoming);
  const auto tile_axis = direction(rotation);
  auto connection = previous.center;
  if (!previous.is_double) {
    connection.x += 0.5 * previous_axis.x;
    connection.y += 0.5 * previous_axis.y;
  } else if (previous.incoming != previous.outgoing) {
    // A corner double exits through its end, in the outgoing direction.
    const auto exit_axis = direction(previous.outgoing);
    connection.x += 0.5 * exit_axis.x;
    connection.y += 0.5 * exit_axis.y;
  }
  const double current_half = is_double ? 0 : 0.5;
  return {{connection.x + incoming_axis.x
              + current_half * tile_axis.x,
           connection.y + incoming_axis.y
              + current_half * tile_axis.y},
          rotation, incoming, outgoing, is_double};
}

struct DraftBoard {
  std::vector<DraftTile> tiles;
};

struct ArmCursor {
  DraftTile previous;
  int horizontal;
  bool in_connector{false};
  int vertical_tiles{0};
};

DraftTile advance(ArmCursor &cursor, bool is_double, double half_lane,
                  int vertical, bool terminal = false) {
  int outgoing = cursor.horizontal;
  if (cursor.in_connector) {
    if (cursor.vertical_tiles == 2) {
      cursor.horizontal = (cursor.horizontal + 180) % 360;
      cursor.in_connector = false;
      outgoing = cursor.horizontal;
    } else {
      outgoing = vertical;
    }
  } else {
    const auto extent = bounds(next_tile(cursor.previous, is_double, cursor.horizontal));
    // Test the edge toward which the run grows. A return run can still be
    // outside the opposite edge while it is heading back into the board.
    const bool beyond_edge = cursor.horizontal == 0
        ? extent.right > half_lane + epsilon : extent.left < -half_lane - epsilon;
    if (!cursor.previous.is_double && beyond_edge) {
      outgoing = vertical;
      cursor.in_connector = true;
      cursor.vertical_tiles = 0;
    }
  }
  if (terminal && is_double) outgoing = cursor.previous.outgoing;
  auto tile = next_tile(cursor.previous, is_double, outgoing);
  if (cursor.in_connector && tile.rotation % 180 == 90) ++cursor.vertical_tiles;
  cursor.previous = tile;
  return tile;
}

std::optional<DraftBoard> snake(std::span<const Block> tiles,
                                std::size_t opening_index, double half_lane) {
  DraftBoard result{std::vector<DraftTile>(tiles.size())};
  const bool opening_double = tiles[opening_index].is_double();
  const DraftTile opening{{0, 0}, opening_double ? 90 : 0, 0, 0, opening_double};
  result.tiles[opening_index] = opening;
  std::vector<Bounds> occupied{bounds(opening)};

  // Right arm grows upward; left arm grows downward, as in the references.
  for (const int step : {1, -1}) {
    const int vertical = step == 1 ? 270 : 90;
    ArmCursor cursor{opening, step == 1 ? 0 : 180};
    if (step == -1) {
      cursor.previous.rotation = (opening.rotation + 180) % 360;
      cursor.previous.incoming = cursor.previous.outgoing = 180;
    }

    auto index = static_cast<std::ptrdiff_t>(opening_index) + step;
    for (; index >= 0 && index < static_cast<std::ptrdiff_t>(tiles.size()); index += step) {
      const auto row = static_cast<std::size_t>(index);
      const auto following_index = index + step;
      const bool terminal = following_index < 0 ||
          following_index >= static_cast<std::ptrdiff_t>(tiles.size());
      const auto candidate = advance(cursor, tiles[row].is_double(), half_lane, vertical, terminal);
      const auto extent = bounds(candidate);
      if (std::any_of(occupied.begin(), occupied.end(), [&](const auto &other) {
            return overlaps(extent, other);
          })) {
        return std::nullopt;
      }
      occupied.push_back(extent);
      result.tiles[row] = candidate;
      if (step == -1) {
        // Left-arm traversal is reversed relative to the engine's left/right
        // values. Rotate the drawing, not the stored engine tile.
        auto &logical = result.tiles[row];
        logical.rotation = (candidate.rotation + 180) % 360;
        logical.incoming = (candidate.outgoing + 180) % 360;
        logical.outgoing = (candidate.incoming + 180) % 360;
      }
    }
  }
  return result;
}

struct DraftFit {
  double scale{-1};
  int shift_steps{0};
};

DraftFit fit_bounds(double extent_x, double top, double bottom,
                     double width, double height, int preferred_shift,
                     double maximum_scale) {
  // Beyond these steps the whole snake is on one side of the board's center;
  // moving it farther can only reduce the scale that fits.
  const int limit = static_cast<int>(std::ceil(std::max(-top, bottom) / shift_step)) + 1;
  DraftFit best;
  for (int steps = -limit; steps <= limit; ++steps) {
    const double offset = steps * shift_step;
    const double extent_y = std::max(-(top + offset), bottom + offset);
    const double scale = std::min({maximum_scale,
        width / (2 * extent_x * face_size), height / (2 * extent_y * face_size)});
    if (scale < best.scale - epsilon ||
        (std::abs(scale - best.scale) <= epsilon &&
         std::abs(steps - preferred_shift) >= std::abs(best.shift_steps - preferred_shift))) continue;
    best = {scale, steps};
  }
  return best;
}

std::optional<int> shift_to_fit(const DraftBoard &draft, double width, double height,
                                int preferred_shift, double tile_height) {
  double extent_x = 0;
  double top = 0;
  double bottom = 0;
  for (const auto &tile : draft.tiles) {
    const auto extent = bounds(tile);
    extent_x = std::max({extent_x, std::abs(extent.left), std::abs(extent.right)});
    top = std::min(top, extent.top);
    bottom = std::max(bottom, extent.bottom);
  }
  if (2 * extent_x * tile_height > width + epsilon) return std::nullopt;
  const int limit = static_cast<int>(std::ceil(std::max(-top, bottom) / shift_step)) + 1;
  std::optional<int> best;
  for (int steps = -limit; steps <= limit; ++steps) {
    const double offset = steps * shift_step;
    const double extent_y = std::max(-(top + offset), bottom + offset);
    if (2 * extent_y * tile_height > height + epsilon) continue;
    if (!best || std::abs(steps - preferred_shift) < std::abs(*best - preferred_shift)) best = steps;
  }
  return best;
}

// Capacity depends only on tile shapes, not pip values. Merge equivalent arm
// states, retaining the most extreme vertical position. This bounds every
// 40-tile shape, including forty ordinary tiles and up to ten separated doubles.
struct CapacityState {
  int x_half, rotation, incoming, outgoing, horizontal, vertical_tiles, doubles;
  bool is_double, in_connector;
  auto operator<=>(const CapacityState &) const = default;
};

struct Capacity {
  double half_lane;
  double extent_x{1};
  std::array<double, maximum_tiles> top;
};

Capacity capacity_for(double half_lane) {
  Capacity capacity{half_lane, 1, {}};
  capacity.top.fill(-1); // Opening and sideways corners can extend one face.
  std::map<CapacityState, double> states;
  for (const bool is_double : {false, true}) {
    states.emplace(CapacityState{0, is_double ? 90 : 0, 0, 0, 0, 0,
                                is_double ? 1 : 0, is_double, false}, 0);
  }
  for (std::size_t count = 1; count < maximum_tiles; ++count) {
    std::map<CapacityState, double> next;
    capacity.top[count] = capacity.top[count - 1];
    for (const auto &[state, y] : states) {
      for (const bool is_double : {false, true}) {
        // Connected unique dominoes cannot contain two adjacent doubles.
        if (is_double && (state.is_double || state.doubles == 10)) continue;
        ArmCursor cursor{{{state.x_half / 2., y}, state.rotation, state.incoming,
                           state.outgoing, state.is_double},
                          state.horizontal, state.in_connector, state.vertical_tiles};
        const auto tile = advance(cursor, is_double, half_lane, 270);
        const auto extent = bounds(tile);
        capacity.extent_x = std::max({capacity.extent_x, std::abs(extent.left),
                                      std::abs(extent.right)});
        capacity.top[count] = std::min(capacity.top[count], extent.top);
        const CapacityState key{static_cast<int>(tile.center.x * 2), tile.rotation,
            tile.incoming, tile.outgoing, cursor.horizontal, cursor.vertical_tiles,
            state.doubles + (is_double ? 1 : 0), is_double, cursor.in_connector};
        const auto [found, inserted] = next.emplace(key, tile.center.y);
        if (!inserted) found->second = std::min(found->second, tile.center.y);
      }
    }
    states = std::move(next);
  }
  return capacity;
}

const std::vector<Capacity> &capacities() {
  // Geometry is invariant across board sizes, hands and matches. Compute this
  // finite capacity table once; viewport changes only evaluate its bounds.
  static const auto table = [] {
    std::vector<Capacity> result;
    for (std::size_t steps = 6; steps <= 4 * maximum_tiles + 4; ++steps) {
      result.push_back(capacity_for(static_cast<double>(steps) / 2));
    }
    return result;
  }();
  return table;
}

BoardLayout configure(double width, double height) {
  BoardLayout result;
  double best_scale = -1;
  for (const auto &capacity : capacities()) {
    double scale = 1;
    // Either end may receive all remaining tiles. Reserve every possible
    // split of the complete hand around its actual opening.
    for (std::size_t left = 0; left < maximum_tiles; ++left) {
      const auto right = maximum_tiles - 1 - left;
      scale = std::min(scale, fit_bounds(capacity.extent_x, capacity.top[right],
          -capacity.top[left], width, height, 0, 1).scale);
    }
    if (scale < best_scale - epsilon) continue;
    best_scale = scale;
    result.tile_width = 2 * face_size * scale;
    result.tile_height = face_size * scale;
    result.half_lane = capacity.half_lane;
  }
  result.viewport_width = width;
  result.viewport_height = height;
  return result;
}

BoardLayout place(const DraftBoard &draft, BoardLayout result, int shift_steps) {
  result.tiles.clear();
  result.vertical_shift_steps = shift_steps;
  for (const auto &tile : draft.tiles) {
    result.tiles.push_back({result.viewport_width / 2 + tile.center.x * result.tile_height,
        result.viewport_height / 2 + (tile.center.y + shift_steps * shift_step) * result.tile_height,
        tile.rotation, tile.incoming, tile.outgoing});
  }
  return result;
}

} // namespace

BoardLayout layout_board(std::span<const Block> tiles, std::size_t opening_index,
                          double width, double height, const BoardLayout *previous) {
  if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0) {
    throw std::invalid_argument("Board viewport must be finite and positive");
  }
  if (tiles.size() > maximum_tiles) {
    throw std::out_of_range("A board can contain at most forty played tiles");
  }
  if (tiles.empty() && opening_index != 0) {
    throw std::out_of_range("Empty board has no opening tile");
  }
  if (!tiles.empty() && opening_index >= tiles.size()) {
    throw std::out_of_range("Opening tile index is outside the board");
  }

  const bool same_viewport = previous && previous->half_lane != 0 &&
      width == previous->viewport_width && height == previous->viewport_height;
  auto plan = same_viewport ? *previous : configure(width, height);
  if (tiles.empty()) {
    plan.tiles.clear();
    plan.vertical_shift_steps = 0;
    return plan;
  }
  const auto draft = snake(tiles, opening_index, plan.half_lane);
  if (!draft) throw std::logic_error("Reserved snake geometry contains an overlap");
  const int preferred_shift = previous ? previous->vertical_shift_steps : 0;
  const auto shifted = shift_to_fit(*draft, width, height, preferred_shift, plan.tile_height);
  if (!shifted) {
    throw std::logic_error("Snake exceeds its reserved forty-tile capacity");
  }
  return place(*draft, std::move(plan), *shifted);
}

} // namespace dominoes::gui
