#pragma once

#include "Block.hpp"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

#include <span>
#include <vector>

namespace dominoes::gui {

class TileListModel : public QAbstractListModel {
  Q_OBJECT
  QML_ELEMENT
  QML_UNCREATABLE("Tile models are provided by the presentation layer")

public:
  enum Role { LeftValueRole = Qt::UserRole + 1, RightValueRole };
  Q_ENUM(Role)

  explicit TileListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = {}) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // Copies values; no references to engine tiles survive a refresh.
  // This is a C++ adapter operation, not a QML-invokable edit.
  void set_tiles(std::span<const Block> tiles);

private:
  struct TileValues {
    int left;
    int right;
  };

  std::vector<TileValues> m_tiles;
};

} // namespace dominoes::gui
