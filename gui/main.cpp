#include "MatchViewModel.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <QVariant>

#include <random>

int main(int argc, char *argv[]) {
  QGuiApplication app{argc, argv};
  // The QML engine is destroyed first, followed by the presentation and match.
  dominoes::gui::MatchViewModel match{{"John", "Kyle", "Jane", "Kate"},
                                    std::mt19937{std::random_device{}()}};
  QQmlApplicationEngine engine;

  engine.setInitialProperties({
      {"matchViewModel", QVariant::fromValue(&match)},
      {"showTileGallery", app.arguments().contains("--tile-gallery")},
  });

  QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                   &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
  engine.loadFromModule("DominoesGui", "Main");

  return app.exec();
}
