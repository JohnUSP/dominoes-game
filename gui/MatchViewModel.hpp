#pragma once

#include "Match.hpp"
#include "BoardTileModel.hpp"
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

  Q_PROPERTY(QString currentPlayerName READ currentPlayerName NOTIFY gameStateChanged)
  Q_PROPERTY(dominoes::gui::TileListModel *hand READ hand CONSTANT)
  Q_PROPERTY(dominoes::gui::BoardTileModel *board READ board CONSTANT)
  Q_PROPERTY(int selectedHandIndex READ selectedHandIndex NOTIFY selectionChanged)
  Q_PROPERTY(bool openingAvailable READ openingAvailable NOTIFY gameStateChanged)
  Q_PROPERTY(QString feedback READ feedback NOTIFY feedbackChanged)

public:
  MatchViewModel(std::array<std::string, 4> player_names,
                 std::mt19937 random_engine, QObject *parent = nullptr);

  QString currentPlayerName() const;
  TileListModel *hand() noexcept;
  BoardTileModel *board() noexcept;
  int selectedHandIndex() const noexcept;
  bool openingAvailable() const noexcept;
  QString feedback() const;

  // Reject invalid indices. Clicking the selected tile deselects it.
  Q_INVOKABLE bool toggleSelection(int index);
  // This GUI slice supports the opening move only.
  Q_INVOKABLE bool playOpening();

signals:
  void selectionChanged();
  void gameStateChanged();
  void feedbackChanged();

private:
  void refreshState();
  void setFeedback(QString message);

  std::mt19937 m_engine;
  Match m_match;
  TileListModel m_hand;
  BoardTileModel m_board;
  int m_selected_index{-1};
  QString m_feedback{QStringLiteral("Select a tile, then click Play to open the hand.")};
};

} // namespace dominoes::gui
