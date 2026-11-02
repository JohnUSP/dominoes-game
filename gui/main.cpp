#include "MatchViewModel.hpp"
#include "BoardPreviewTiles.hpp"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QVariant>

#include <random>

int main(int argc, char *argv[]) {
  QGuiApplication app{argc, argv};
  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addOption({"tile-gallery", "Preview all 55 tile faces."});
  parser.addOption({"board-preview", "Preview a connected snake board."});
  parser.addOption({"preview-count", "Number of board-preview tiles (1-40).", "count", "40"});
  parser.process(app);
  bool valid_count = false;
  const int preview_count = parser.value("preview-count").toInt(&valid_count);
  if (!valid_count || preview_count < 1 || preview_count > 40 ||
      (parser.isSet("tile-gallery") && parser.isSet("board-preview"))) {
    qCritical("Use one preview mode and a preview count from 1 through 40.");
    return 1;
  }

  // The QML engine is destroyed first, followed by the presentation and match.
  dominoes::gui::MatchViewModel match{{"John", "Kyle", "Jane", "Kate"},
                                    std::mt19937{std::random_device{}()}};
  dominoes::gui::BoardTileModel preview_board;
  if (parser.isSet("board-preview")) {
    const auto preview = dominoes::gui::board_preview_chain(
        static_cast<std::size_t>(preview_count));
    preview_board.set_tiles(preview.tiles, preview.opening_index);
  }
  QQmlApplicationEngine engine;

  engine.setInitialProperties({
      {"matchViewModel", QVariant::fromValue(&match)},
      {"previewBoard", QVariant::fromValue(&preview_board)},
      {"showTileGallery", parser.isSet("tile-gallery")},
      {"showBoardPreview", parser.isSet("board-preview")},
  });

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
  engine.loadFromModule("DominoesGui", "Main");

  return app.exec();
}
