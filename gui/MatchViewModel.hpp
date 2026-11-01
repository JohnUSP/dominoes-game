#pragma once

#include "Match.hpp"
#include "TileListModel.hpp"

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <array>
#include <random>
#include <string>

namespace dominoes::gui {

class MatchViewModel : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_UNCREATABLE("The application creates and owns the match")

  // This slice only displays the initial hand; turn-changing commands come later.
  Q_PROPERTY(QString currentPlayerName READ currentPlayerName CONSTANT)
  Q_PROPERTY(dominoes::gui::TileListModel *hand READ hand CONSTANT)
  Q_PROPERTY(int selectedHandIndex READ selectedHandIndex NOTIFY selectionChanged)

public:
  MatchViewModel(std::array<std::string, 4> player_names,
                 std::mt19937 random_engine, QObject *parent = nullptr);

  QString currentPlayerName() const;
  TileListModel *hand() noexcept;
  int selectedHandIndex() const noexcept;

  // Reject invalid indices. Clicking the selected tile deselects it.
  Q_INVOKABLE bool toggleSelection(int index);

signals:
  void selectionChanged();

private:
  std::mt19937 m_engine;
  Match m_match;
  TileListModel m_hand;
  int m_selected_index{-1};
};

} // namespace dominoes::gui
