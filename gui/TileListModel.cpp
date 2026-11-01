#include "TileListModel.hpp"

#include <QVariant>

#include <utility>

namespace dominoes::gui {

TileListModel::TileListModel(QObject *parent) : QAbstractListModel(parent) {}

int TileListModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_tiles.size());
}

QVariant TileListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.model() != this || index.column() != 0 ||
      index.row() < 0 || index.row() >= rowCount()) {
    return {};
  }

  const auto &tile = m_tiles[static_cast<std::size_t>(index.row())];
  switch (role) {
  case LeftValueRole:
    return tile.left;
  case RightValueRole:
    return tile.right;
  default:
    return {};
  }
}

QHash<int, QByteArray> TileListModel::roleNames() const {
  return {{LeftValueRole, "leftValue"}, {RightValueRole, "rightValue"}};
}

void TileListModel::set_tiles(std::span<const Block> tiles) {
  std::vector<TileValues> snapshot;
  snapshot.reserve(tiles.size());
  for (const auto &tile : tiles) {
    snapshot.push_back({to_value(tile.left()), to_value(tile.right())});
  }

  beginResetModel();
  m_tiles = std::move(snapshot);
  endResetModel();
}

} // namespace dominoes::gui
