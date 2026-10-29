#include "../Android/AndroidGame.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTest>
#include <QThreadPool>
#include <cstdlib>
#include <iostream>

void require(bool ok, const char* message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}

QQuickItem* visualItem(QQuickItem* root, const QString& name) {
    if (root->objectName() == name) return root;
    for (auto* child : root->childItems()) if (auto* found = visualItem(child, name)) return found;
    return nullptr;
}

QPoint squareCenter(QQuickWindow* window, int index) {
    auto* square = visualItem(window->contentItem(), QString("square%1").arg(index));
    require(square, "Mobile QML square was not created");
    return square->mapToScene(QPointF(square->width()/2, square->height()/2)).toPoint();
}

int main(int argc, char** argv) {
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    AndroidGame game;
    game.newGame(true, false, 0);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("game", &game);
    engine.load(QUrl("qrc:/mobile/Main.qml"));
    require(engine.rootObjects().size() == 1, "Mobile QML interface failed to load");
    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    require(window, "Mobile QML root is not a window");
    window->resize(320, 568); window->show();
    QTest::qWait(100);
    auto* board = window->findChild<QQuickItem*>("boardGrid");
    require(board && board->width() <= 320 && board->width() >= 280 && board->height() == board->width(),
            "Mobile board did not fit a narrow phone");
    auto* touch = QTest::createTouchDevice();
    for (int square : {52, 36}) {
        const auto point = squareCenter(window, square);
        require(window->contentItem()->contains(point), "Phone move target is outside the screen");
        QTest::touchEvent(window, touch).press(0, point, window);
        QTest::touchEvent(window, touch).release(0, point, window);
        QTest::qWait(20);
    }
    require(game.position().squares[4][4] == chess::Pawn, "Touch input did not play e2-e4");
    game.newGame(false, false, 0); QTest::qWait(20);
    for (int square : {52, 36}) {
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, squareCenter(window, square));
        QTest::qWait(20);
    }
    require(game.position().squares[4][4] == chess::Pawn, "Rotated mobile tap coordinates are incorrect");
    auto* newGame = window->findChild<QQuickItem*>("newGameButton");
    require(newGame, "Mobile new-game control is missing");
    QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
                      newGame->mapToScene(QPointF(newGame->width()/2, newGame->height()/2)).toPoint());
    QTest::qWait(50);
    auto* settings = window->findChild<QObject*>("newGameSettings");
    require(settings && settings->property("visible").toBool(), "Phone settings did not open");
    require(settings->property("width").toDouble() <= 320, "Phone settings are too wide");
    QMetaObject::invokeMethod(settings, "close");
    window->resize(800, 360); QTest::qWait(50);
    require(board->width() <= 540 && board->height() == board->width(), "Landscape board is not square or bounded");
    const auto screenshot = qEnvironmentVariable("CHESS_MOBILE_SCREENSHOT");
    if (!screenshot.isEmpty()) {
        window->resize(390, 820); game.newGame(true, false, 0); QTest::qWait(100);
        require(window->grabWindow().save(screenshot), "Could not save mobile preview screenshot");
    }
    QThreadPool::globalInstance()->waitForDone();
}
