#include "MatchViewModel.hpp"

#include <utility>
#include <vector>

namespace dominoes::gui {

MatchViewModel::MatchViewModel(std::array<std::string, 4> player_names,
                             std::mt19937 random_engine, QObject *parent)
    : QObject(parent), m_engine(std::move(random_engine)),
      m_match(std::move(player_names), m_engine), m_hand(this), m_board(this) {
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

bool MatchViewModel::toggleSelection(int index) {
  if (index < 0 || index >= m_hand.rowCount()) {
    return false;
  }

  m_selected_index = m_selected_index == index ? -1 : index;
  emit selectionChanged();
  return true;
}

bool MatchViewModel::playOpening() {
  if (!openingAvailable()) {
    setFeedback(QStringLiteral("The hand is already open."));
    return false;
  }
  if (m_selected_index < 0) {
    setFeedback(QStringLiteral("Select a tile before playing."));
    return false;
  }

  const auto result = m_match.play({static_cast<std::size_t>(m_selected_index)});
  if (result == ActionResult::rejected) {
    setFeedback(QStringLiteral("The selected tile cannot be played."));
    return false;
  }

  refreshState();
  setFeedback(QStringLiteral("Opening played. %1's turn.").arg(currentPlayerName()));
  return true;
}

void MatchViewModel::refreshState() {
  const auto &game = m_match.current_hand();
  const auto &blocks = game.board().blocks();
  // Board uses deque storage; copy to contiguous storage for the snapshot API.
  const std::vector<Block> board_tiles{blocks.begin(), blocks.end()};
  m_board.set_tiles(board_tiles, game.board().opening_index());
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
