#pragma once

#include "Match.hpp"
#include "BoardTileModel.hpp"
#include "TileListModel.hpp"

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <array>
#include <optional>
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
  Q_PROPERTY(bool handFinished READ handFinished NOTIFY gameStateChanged)
  Q_PROPERTY(bool passAvailable READ passAvailable NOTIFY gameStateChanged)
  Q_PROPERTY(bool canPlayLeft READ canPlayLeft NOTIFY legalMovesChanged)
  Q_PROPERTY(bool canPlayRight READ canPlayRight NOTIFY legalMovesChanged)
  Q_PROPERTY(QString feedback READ feedback NOTIFY feedbackChanged)

public:
  MatchViewModel(std::array<std::string, 4> player_names,
                 std::mt19937 random_engine, QObject *parent = nullptr);

  QString currentPlayerName() const;
  TileListModel *hand() noexcept;
  BoardTileModel *board() noexcept;
  int selectedHandIndex() const noexcept;
  bool openingAvailable() const noexcept;
  bool handFinished() const noexcept;
  // Available during an open hand; the engine penalizes passes when a move exists.
  bool passAvailable() const noexcept;
  bool canPlayLeft() const;
  bool canPlayRight() const;
  QString feedback() const;

  // Reject invalid indices. Clicking the selected tile deselects it.
  Q_INVOKABLE bool toggleSelection(int index);
  Q_INVOKABLE bool playOpening();
  Q_INVOKABLE bool playLeft();
  Q_INVOKABLE bool playRight();
  Q_INVOKABLE bool passTurn();

signals:
  void selectionChanged();
  void gameStateChanged();
  void feedbackChanged();
  void legalMovesChanged();

private:
  void refreshState();
  void refreshHand();
  void setFeedback(QString message);
  bool canPlayAt(BoardEnd end) const;
  bool playAt(BoardEnd end);
  bool submitSelected(std::optional<BoardEnd> end);

  std::mt19937 m_engine;
  Match m_match;
  TileListModel m_hand;
  BoardTileModel m_board;
  int m_selected_index{-1};
  QString m_feedback{QStringLiteral("Select a tile, then click Play to open the hand.")};
};

} // namespace dominoes::gui
