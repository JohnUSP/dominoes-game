#include "MatchViewModel.hpp"

#include <utility>
#include <vector>

namespace dominoes::gui {

MatchViewModel::MatchViewModel(std::array<std::string, 4> player_names,
                             std::mt19937 random_engine, QObject *parent)
    : QObject(parent), m_engine(std::move(random_engine)),
      m_match(std::move(player_names), m_engine), m_hand(this), m_board(this) {
  QObject::connect(this, &MatchViewModel::selectionChanged, this, &MatchViewModel::legalMovesChanged);
  QObject::connect(this, &MatchViewModel::gameStateChanged, this, &MatchViewModel::legalMovesChanged);
  QObject::connect(&m_hand, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
    if (m_selected_index != -1) {
      m_selected_index = -1;
      emit selectionChanged();
    }
  });

  refreshState();
}

QString MatchViewModel::currentPlayerName() const {
  const auto player = m_match.current_hand().current_player_index();
  return QString::fromStdString(m_match.players()[player].name());
}

TileListModel *MatchViewModel::hand() noexcept { return &m_hand; }

BoardTileModel *MatchViewModel::board() noexcept { return &m_board; }

int MatchViewModel::selectedHandIndex() const noexcept { return m_selected_index; }

bool MatchViewModel::openingAvailable() const noexcept {
  const auto &game = m_match.current_hand();
  return game.board().blocks().empty() && !game.hand_result();
}

QString MatchViewModel::feedback() const { return m_feedback; }

bool MatchViewModel::handFinished() const noexcept {
  return m_match.current_hand().hand_result().has_value();
}

bool MatchViewModel::passAvailable() const noexcept {
  const auto &game = m_match.current_hand();
  return !game.board().blocks().empty() && !game.hand_result();
}

bool MatchViewModel::canPlayAt(BoardEnd end) const {
  return m_selected_index >= 0 && !openingAvailable() &&
      m_match.current_hand().can_play({static_cast<std::size_t>(m_selected_index), end});
}

bool MatchViewModel::canPlayLeft() const { return canPlayAt(BoardEnd::left); }
bool MatchViewModel::canPlayRight() const { return canPlayAt(BoardEnd::right); }

bool MatchViewModel::toggleSelection(int index) {
  if (handFinished() || index < 0 || index >= m_hand.rowCount()) {
    return false;
  }

  m_selected_index = m_selected_index == index ? -1 : index;
  emit selectionChanged();
  if (openingAvailable()) {
    setFeedback(QStringLiteral("Select a tile, then click Play to open the hand."));
  } else if (m_selected_index < 0) {
    setFeedback(QStringLiteral("Select a tile, then click its half of the board."));
  } else if (canPlayLeft() || canPlayRight()) {
    setFeedback(m_board.hasTurns()
        ? QStringLiteral("Click the top or bottom half to choose an end.")
        : QStringLiteral("Click the left or right half to choose an end."));
  } else {
    setFeedback(QStringLiteral("This tile has no legal end. Select another tile."));
  }
  return true;
}

bool MatchViewModel::playOpening() {
  if (handFinished()) {
    setFeedback(QStringLiteral("The hand is finished."));
    return false;
  }
  if (!openingAvailable()) {
    setFeedback(QStringLiteral("The hand is already open."));
    return false;
  }
  return submitSelected(std::nullopt);
}

bool MatchViewModel::playLeft() { return playAt(BoardEnd::left); }
bool MatchViewModel::playRight() { return playAt(BoardEnd::right); }

bool MatchViewModel::passTurn() {
  const auto player = currentPlayerName();
  const auto action = m_match.pass();
  if (action == ActionResult::rejected) {
    setFeedback(handFinished() ? QStringLiteral("The hand is finished.")
                              : QStringLiteral("Open the hand before passing."));
    return false;
  }

  // Passing changes the player/hand projection, not the board geometry.
  refreshHand();
  if (action == ActionResult::hand_ended) {
    const auto reason = m_match.current_hand().hand_result()->reason;
    if (reason == HandEndReason::invalid_pass) {
      setFeedback(QStringLiteral("Invalid pass: %1's team loses the hand.").arg(player));
    } else if (reason == HandEndReason::blocked) {
      setFeedback(QStringLiteral("The hand is blocked: no player can move."));
    } else {
      setFeedback(QStringLiteral("The hand is finished."));
    }
  } else {
    setFeedback(QStringLiteral("%1 passed. %2's turn.").arg(player, currentPlayerName()));
  }
  return true;
}

bool MatchViewModel::playAt(BoardEnd end) {
  if (handFinished()) {
    setFeedback(QStringLiteral("The hand is finished."));
    return false;
  }
  if (openingAvailable()) {
    setFeedback(QStringLiteral("Click Play to open the hand first."));
    return false;
  }
  return submitSelected(end);
}

bool MatchViewModel::submitSelected(std::optional<BoardEnd> end) {
  if (m_selected_index < 0) {
    setFeedback(QStringLiteral("Select a tile before playing."));
    return false;
  }

  const auto result = m_match.play({static_cast<std::size_t>(m_selected_index), end});
  if (result == ActionResult::rejected) {
    setFeedback(end ? QStringLiteral("The selected tile cannot be played at that end.")
                    : QStringLiteral("The selected tile cannot be played."));
    return false;
  }

  refreshState();
  if (result == ActionResult::hand_ended) {
    setFeedback(QStringLiteral("The hand is finished."));
  } else {
    setFeedback(QStringLiteral("%1's turn. Select a tile, then its half of the board.")
                    .arg(currentPlayerName()));
  }
  return true;
}

void MatchViewModel::refreshState() {
  const auto &game = m_match.current_hand();
  const auto &blocks = game.board().blocks();
  // Board uses deque storage; copy to contiguous storage for the snapshot API.
  const std::vector<Block> board_tiles{blocks.begin(), blocks.end()};
  m_board.set_tiles(board_tiles, game.board().opening_index());
  refreshHand();
}

void MatchViewModel::refreshHand() {
  const auto &game = m_match.current_hand();
  m_hand.set_tiles(m_match.players()[game.current_player_index()].hand());
  emit gameStateChanged();
}

void MatchViewModel::setFeedback(QString message) {
  if (message != m_feedback) {
    m_feedback = std::move(message);
    emit feedbackChanged();
  }
}

} // namespace dominoes::gui
