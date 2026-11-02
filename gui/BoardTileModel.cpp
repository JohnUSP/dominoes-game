#include "BoardTileModel.hpp"

#include <QVariant>

#include <algorithm>
#include <cmath>
#include <utility>
#include <stdexcept>

namespace dominoes::gui {

BoardTileModel::BoardTileModel(QObject *parent) : QAbstractListModel(parent) {}

int BoardTileModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : count();
}

int BoardTileModel::count() const noexcept { return static_cast<int>(m_tiles.size()); }

QVariant BoardTileModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.model() != this || index.column() != 0 ||
      index.row() < 0 || index.row() >= count()) {
    return {};
  }
  const auto row = static_cast<std::size_t>(index.row());
  const auto &placement = m_layout.tiles[row];
  switch (role) {
  case LeftValueRole: return to_value(m_tiles[row].left());
  case RightValueRole: return to_value(m_tiles[row].right());
  case CenterXRole: return placement.center_x;
  case CenterYRole: return placement.center_y;
  case AngleRole: return placement.rotation;
  case TileWidthRole: return m_layout.tile_width;
  case TileHeightRole: return m_layout.tile_height;
  default: return {};
  }
}

QHash<int, QByteArray> BoardTileModel::roleNames() const {
  return {{LeftValueRole, "leftValue"}, {RightValueRole, "rightValue"},
          {CenterXRole, "centerX"}, {CenterYRole, "centerY"},
          {AngleRole, "angle"}, {TileWidthRole, "tileWidth"},
          {TileHeightRole, "tileHeight"}};
}

void BoardTileModel::set_tiles(std::span<const Block> tiles,
                               std::optional<std::size_t> opening_index) {
  if (tiles.empty() != !opening_index) {
    throw std::invalid_argument("A nonempty board requires its opening index");
  }
  std::vector<Block> snapshot{tiles.begin(), tiles.end()};
  const BoardLayout *previous = nullptr;
  if (m_opening_index && opening_index && *opening_index >= *m_opening_index) {
    const auto added_left = *opening_index - *m_opening_index;
    if (added_left <= snapshot.size() && m_tiles.size() <= snapshot.size() - added_left &&
        std::equal(m_tiles.begin(), m_tiles.end(), snapshot.begin() + added_left,
                   [](const Block &a, const Block &b) {
                     return a.left() == b.left() && a.right() == b.right();
                   })) {
      previous = &m_layout;
    }
  }
  auto layout = layout_board(snapshot, opening_index.value_or(0),
                             m_viewport_width, m_viewport_height, previous);
  const bool count_changed = snapshot.size() != m_tiles.size();
  beginResetModel();
  m_tiles = std::move(snapshot);
  m_layout = std::move(layout);
  m_opening_index = opening_index;
  endResetModel();
  if (count_changed) emit countChanged();
}

void BoardTileModel::setViewport(double width, double height) {
  // QML can temporarily report zero sizes while constructing or resizing a view.
  if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0 ||
      (width == m_viewport_width && height == m_viewport_height)) {
    return;
  }
  auto layout = layout_board(m_tiles, m_opening_index.value_or(0), width, height, &m_layout);
  beginResetModel();
  m_viewport_width = width;
  m_viewport_height = height;
  m_layout = std::move(layout);
  endResetModel();
}

} // namespace dominoes::gui
