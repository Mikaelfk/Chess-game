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
    auto* animated = visualItem(window->contentItem(), "movingPiece");
    require(animated, "Move animation was not created");
    const auto initialY = animated->y();
    QTest::qWait(80);
    require(animated->y() < initialY, "Pawn animation did not travel toward e4");
    QTest::qWait(180);
    require(!visualItem(window->contentItem(), "movingPiece"), "Move animation did not finish");
    game.newGame(true, true, 0); QTest::qWait(30);
    const auto boardPosition = board->mapToScene(QPointF());
    game.selectSquare(52); game.selectSquare(36);
    QTest::qWait(100);
    require(game.thinking() && game.replayLength() == 1, "Computer did not pause before replying");
    require(board->mapToScene(QPointF()) == boardPosition, "Thinking indicator shifted the board");
    for (int i = 0; i < 100 && game.thinking(); ++i) QTest::qWait(50);
    require(!game.thinking() && game.replayLength() == 2, "Computer did not reply");
    require(board->mapToScene(QPointF()) == boardPosition, "Board shifted after computer reply");
    require(visualItem(window->contentItem(), "movingPiece"), "Computer reply must still animate");
    QTest::qWait(250);
    game.newGame(false, false, 0); QTest::qWait(20);
    for (int square : {52, 36}) {
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, squareCenter(window, square));
        QTest::qWait(20);
    }
    require(game.position().squares[4][4] == chess::Pawn, "Rotated mobile tap coordinates are incorrect");
    QTest::qWait(250);
    auto* evaluation = window->findChild<QQuickItem*>("replayEvaluation");
    require(evaluation && !evaluation->isVisible(), "Evaluation must be hidden during live games");
    for (bool white : {true, false}) {
        game.newGame(white, false, 0); QTest::qWait(30);
        const auto from = squareCenter(window, 52);
        const auto illegal = squareCenter(window, 28);
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(window, illegal, 30);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, illegal);
        require(game.replayLength() == 0, "Illegal drag changed the position");
        const auto to = squareCenter(window, 36);
        QTest::touchEvent(window, touch).press(0, from, window);
        QTest::touchEvent(window, touch).move(0, to, window);
        QTest::qWait(30);
        QTest::touchEvent(window, touch).release(0, to, window);
        require(game.position().squares[4][4] == chess::Pawn, "Touch drag failed on rotated or normal board");
        require(!visualItem(window->contentItem(), "movingPiece"), "Drag drop replayed the move animation");
        QTest::qWait(250);
    }
    game.newGame(true, false, 0);
    for (auto pair : {std::pair{48,32}, {15,31}, {32,24}, {31,39}, {24,16}, {39,47}, {16,9}, {47,54}}) {
        game.selectSquare(pair.first); game.selectSquare(pair.second);
    }
    QTest::qWait(250);
    const auto promotionFrom = squareCenter(window, 9);
    const auto promotionTo = squareCenter(window, 0);
    QTest::touchEvent(window, touch).press(0, promotionFrom, window);
    QTest::touchEvent(window, touch).move(0, promotionTo, window);
    QTest::qWait(30);
    QTest::touchEvent(window, touch).release(0, promotionTo, window);
    require(game.promotionPending(), "Dragged promotion did not open the choice");
    game.promote(chess::Queen);
    require(game.position().squares[0][0] == chess::Queen, "Dragged promotion did not complete");
    require(!visualItem(window->contentItem(), "movingPiece"), "Dragged promotion replayed its animation");
    game.newGame(true, false, 0);
    for (auto pair : {std::pair{53,45}, std::pair{12,28}, std::pair{54,38}, std::pair{3,39}}) {
        game.selectSquare(pair.first); game.selectSquare(pair.second);
    }
    QTest::qWait(250);
    game.startReplay(); QTest::qWait(30);
    require(evaluation->isVisible(), "Replay evaluation is missing");
    for (int i = 0; i < 100 && !game.evaluationText().contains("engine"); ++i) QTest::qWait(50);
    require(game.evaluationText().contains("engine"), "Replay engine evaluation never arrived");
    game.seekReplay(game.replayLength());
    require(game.evaluation() == 0 && game.evaluationText().contains("checkmate"), "Replay checkmate evaluation is incorrect");
    game.exitReplay(); QTest::qWait(30);
    require(!evaluation->isVisible(), "Replay evaluation remained on the live board");
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
