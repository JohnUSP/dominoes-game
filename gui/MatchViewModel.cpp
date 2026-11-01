#include "MatchViewModel.hpp"

#include <utility>

namespace dominoes::gui {

MatchViewModel::MatchViewModel(std::array<std::string, 4> player_names,
                             std::mt19937 random_engine, QObject *parent)
    : QObject(parent), m_engine(std::move(random_engine)),
      m_match(std::move(player_names), m_engine), m_hand(this) {
  QObject::connect(&m_hand, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
    if (m_selected_index != -1) {
      m_selected_index = -1;
      emit selectionChanged();
    }
  });

  const auto player = m_match.current_hand().current_player_index();
  m_hand.set_tiles(m_match.players()[player].hand());
}

QString MatchViewModel::currentPlayerName() const {
  const auto player = m_match.current_hand().current_player_index();
  return QString::fromStdString(m_match.players()[player].name());
}

TileListModel *MatchViewModel::hand() noexcept { return &m_hand; }

int MatchViewModel::selectedHandIndex() const noexcept { return m_selected_index; }

bool MatchViewModel::toggleSelection(int index) {
  if (index < 0 || index >= m_hand.rowCount()) {
    return false;
  }

  m_selected_index = m_selected_index == index ? -1 : index;
  emit selectionChanged();
  return true;
}

} // namespace dominoes::gui
