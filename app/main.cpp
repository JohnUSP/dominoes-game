#include "Game.hpp"
#include "Match.hpp"
#include "Player.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>

namespace {

void print_block(const dominoes::Block &block) {
  std::cout << '[' << dominoes::to_value(block.left()) << '|'
            << dominoes::to_value(block.right()) << ']';
}

void print_commands(bool opening) {
  std::cout << "Play: play INDEX [left|right]\n";
  if (opening) {
    std::cout << "Opening accepts any tile; the end may be omitted.\n";
    std::cout << "Use transfer to offer opening to your partner; one return is allowed.\n";
  } else {
    std::cout << "End optional for one legal end or equal board ends (uses right).\n"
              << "If both ends accept the tile and show different numbers, choose an end.\n";
  }
  std::cout << "Other commands: pass, quit. Indices start at 0.\n";
}

void print_turn(const dominoes::Game &game,
                const std::array<dominoes::Player, 4> &players) {
  std::cout << "\nBoard: ";
  if (game.board().blocks().empty()) {
    std::cout << "empty";
  }
  for (const auto &block : game.board().blocks()) {
    print_block(block);
    std::cout << ' ';
  }

  const std::size_t index = game.current_player_index();
  const auto &player = players[index];
  std::cout << "\nTurn: " << player.name() << " (team "
            << (index % 2 == 0 ? 'A' : 'B') << ")\nHand: ";
  for (std::size_t tile = 0; tile < player.hand().size(); ++tile) {
    std::cout << tile << ':';
    print_block(player.hand()[tile]);
    std::cout << ' ';
  }
  std::cout << '\n';
  print_commands(game.board().blocks().empty());
}

bool at_end(std::istringstream &input) {
  input >> std::ws;
  return input.eof();
}

// Syntax errors are handled here; the engine validates and scores actions.
std::optional<dominoes::ActionResult>
submit_command(const std::string &command, std::istringstream &input,
               dominoes::Match &match) {
  if (command == "pass" && at_end(input)) {
    return match.pass();
  }
  if (command == "transfer" && at_end(input)) {
    return match.transfer_opening();
  }
  if (command != "play") {
    return std::nullopt;
  }

  int index;
  if (!(input >> index) || index < 0) {
    return std::nullopt;
  }

  if (at_end(input)) {
    return match.play({static_cast<std::size_t>(index)});
  }

  std::string end;
  if (!(input >> end) || (end != "left" && end != "right") || !at_end(input)) {
    return std::nullopt;
  }

  return match.play({static_cast<std::size_t>(index),
                    end == "left" ? dominoes::BoardEnd::left
                                  : dominoes::BoardEnd::right});
}

void print_result(const dominoes::HandResult &result,
                  const dominoes::Match &match) {
  std::cout << "\nHand ended: ";
  switch (result.reason) {
  case dominoes::HandEndReason::domination:
    std::cout << "domination";
    break;
  case dominoes::HandEndReason::blocked:
    std::cout << "blocked";
    break;
  case dominoes::HandEndReason::invalid_pass:
    std::cout << "passing while a legal move was available";
    break;
  case dominoes::HandEndReason::timeout:
    std::cout << "timeout";
    break;
  }
  std::cout << ".\n";

  if (!result.winning_team) {
    std::cout << "Null hand. No points awarded.\n";
    return;
  }

  const std::size_t team = *result.winning_team;
  std::cout << "Winner: " << match.team_names()[team] << "\nAwarded points: "
            << result.awarded_points << '\n';
}

} // namespace

int main() {

  std::cout << "Welcome to Double Nine Dominoes Game" << std::endl;

  std::mt19937 engine{std::random_device{}()};

  // Playing order; teammates occupy seats 0/2 and 1/3.
  dominoes::Match match{{"John", "Kyle", "Jane", "Kate"}, engine};
  const auto &players = match.players();
  const std::size_t opening_team = match.current_hand().current_player_index();

  std::cout << "Team A: " << match.team_names()[0]
            << "\nTeam B: " << match.team_names()[1]
            << "\nOpening team: " << (opening_team == 0 ? 'A' : 'B')
            << "; " << players[opening_team].name() << " receives opening.\n"
            << "Clocks are not active.\n"
            << "Passing with a legal move loses the hand.\n";

  std::size_t hand_number = 1;
  while (!match.winning_team()) {
    const dominoes::Game &game = match.current_hand();
    std::cout << "\nHand " << hand_number << ": "
              << players[game.current_player_index()].name()
              << " receives opening.\n";

    while (!game.hand_result()) {
      print_turn(game, players);
      std::cout << "> " << std::flush;

      std::string line;
      if (!std::getline(std::cin, line)) {
        std::cout << "\nInput closed. Match unfinished.\n";
        return 0;
      }

      std::istringstream input{line};
      std::string command;
      input >> command;
      if (command == "quit" && at_end(input)) {
        std::cout << "Match unfinished.\n";
        return 0;
      }

      const auto result = submit_command(command, input, match);
      if (!result) {
        std::cout << "Invalid command. No action taken.\n";
      } else if (*result == dominoes::ActionResult::rejected) {
        std::cout << "Action rejected. The turn is unchanged.\n";
      } else if (*result == dominoes::ActionResult::accepted) {
        std::cout << "Action accepted.\n";
      }
    }

    print_result(*game.hand_result(), match);
    std::cout << "\nMatch scores (target " << dominoes::Match::target_score
              << "):\n";
    for (std::size_t team = 0; team < match.scores().size(); ++team) {
      std::cout << match.team_names()[team] << ": " << match.scores()[team]
                << '\n';
    }

    if (const auto winner = match.winning_team()) {
      std::cout << "Match winner: " << match.team_names()[*winner] << '\n';
    } else if (match.start_next_hand(engine)) {
      ++hand_number;
    } else {
      std::cerr << "Cannot start the next hand.\n";
      return 1;
    }
  }

  return 0;
}
