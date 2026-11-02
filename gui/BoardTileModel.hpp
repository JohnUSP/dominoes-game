#pragma once

#include "BoardLayout.hpp"

#include <QAbstractListModel>
#include <QtQml/qqmlregistration.h>

#include <span>
#include <optional>
#include <vector>

namespace dominoes::gui {

class BoardTileModel : public QAbstractListModel {
  Q_OBJECT
  QML_ELEMENT
  QML_UNCREATABLE("Board models are owned by the presentation layer")
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(bool hasTurns READ hasTurns NOTIFY geometryChanged)

public:
  enum Role {
    LeftValueRole = Qt::UserRole + 1, RightValueRole,
    CenterXRole, CenterYRole, AngleRole, TileWidthRole, TileHeightRole
  };
  Q_ENUM(Role)

  explicit BoardTileModel(QObject *parent = nullptr);
  int rowCount(const QModelIndex &parent = {}) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;
  int count() const noexcept;
  bool hasTurns() const;

  void set_tiles(std::span<const Block> tiles,
                 std::optional<std::size_t> opening_index);
  Q_INVOKABLE void setViewport(double width, double height);

signals:
  void countChanged();
  void geometryChanged();

private:
  std::vector<Block> m_tiles;
  BoardLayout m_layout;
  std::optional<std::size_t> m_opening_index;
  double m_viewport_width{960};
  double m_viewport_height{480};
};

} // namespace dominoes::gui
