#include "../Android/AndroidGame.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QThread>
#include <QThreadPool>
#include <cstdlib>
#include <iostream>

void require(bool ok, const char* message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}

void move(AndroidGame& game, int from, int to) {
    const auto before = game.position().repetitionKey();
    game.selectSquare(from); game.selectSquare(to);
    require(before != game.position().repetitionKey() || game.promotionPending(), "Mobile move was rejected");
}

void processFor(int milliseconds) {
    QElapsedTimer timer; timer.start();
    while (timer.elapsed() < milliseconds) { QCoreApplication::processEvents(); QThread::msleep(2); }
}

void waitForComputer(AndroidGame& game) {
    QElapsedTimer timer; timer.start();
    while (game.thinking() && timer.elapsed() < 6000) processFor(5);
    require(!game.thinking(), "Mobile AI did not finish");
}

void testSelectionAndOrientation() {
    AndroidGame game;
    game.newGame(true, false, 0);
    require(game.board().size() == 64 && !game.gameOver(), "Mobile board is not playable");
    game.selectSquare(52); // e2
    require(game.board()[52].toMap()["selected"].toBool(), "Mobile selection is missing");
    require(game.board()[36].toMap()["legal"].toBool(), "Mobile legal move hint is missing");
    game.selectSquare(28); // Illegal e2-e5.
    require(game.position().squares == chess::Position::initial().squares, "Illegal tap changed the board");
    game.selectSquare(-1); game.selectSquare(64);
    move(game, 52, 36); move(game, 11, 27); move(game, 36, 27);
    require(game.position().squares[3][3] == chess::Pawn && game.material() == "White +1", "Mobile capture failed");
    game.newGame(false, false, 0);
    require(game.board()[0].toMap()["index"].toInt() == 63 && game.board()[0].toMap()["piece"].toInt() == chess::Rook,
            "Black board orientation is incorrect");
    move(game, 52, 36);
    require(game.board()[27].toMap()["piece"].toInt() == chess::Pawn, "Rotated board did not display the move");
}

void testPromotion() {
    for (int piece : {chess::Queen, chess::Rook, chess::Bishop, chess::Knight}) {
        AndroidGame game; game.newGame(true, false, 0);
        for (const auto& step : {std::pair{48,32}, {15,31}, {32,24}, {31,39}, {24,16}, {39,47}, {16,9}, {47,54}})
            move(game, step.first, step.second);
        move(game, 9, 0);
        require(game.promotionPending() && game.position().squares[1][1] == chess::Pawn, "Promotion did not wait for a choice");
        game.promote(chess::King);
        require(game.promotionPending(), "Invalid promotion was accepted");
        game.cancelPromotion();
        require(!game.promotionPending() && game.position().whiteToMove, "Cancelling promotion changed the turn");
        game.selectSquare(0); // The pawn remains selected after cancellation.
        require(game.promotionPending(), "Promotion could not be retried");
        game.promote(piece);
        require(!game.promotionPending() && game.position().squares[0][0] == piece && !game.position().whiteToMove,
                "Mobile promotion produced the wrong piece or turn");
    }
}

void testResultsReplayAndPersistence() {
    QTemporaryDir folder;
    require(folder.isValid(), "Cannot create mobile save test directory");
    const QString path = folder.path() + "/nested/game.json";
    AndroidGame game(path); game.newGame(true, false, 1);
    for (const auto& step : {std::pair{53,45}, {12,28}, {54,38}, {3,39}}) move(game, step.first, step.second);
    require(game.gameOver() && game.result() == "Black wins by checkmate", "Mobile checkmate result is incorrect");
    const auto finished = game.savedState();
    game.selectSquare(52); game.selectSquare(36);
    require(game.savedState() == finished, "Mobile input changed a finished game");
    game.startReplay();
    require(game.replaying() && game.replayIndex() == 0 && game.replayLength() == 4, "Mobile replay did not start");
    require(game.board()[53].toMap()["piece"].toInt() == chess::Pawn, "Replay did not show the initial board");
    game.seekReplay(999);
    require(game.replayIndex() == 4, "Replay navigation was not bounded");
    game.seekReplay(-1); game.selectSquare(52); game.selectSquare(36);
    require(game.savedState() == finished, "Replay input changed the finished game");
    game.exitReplay();
    require(!game.replaying() && game.gameOver(), "Exiting replay did not restore the finished game");
    AndroidGame restored(path);
    require(restored.savedState() == finished && restored.gameOver() && restored.saveError().isEmpty(), "Saved game did not restore");
    restored.startReplay();
    require(restored.replayLength() == 4, "Saved game lost its replay");
    restored.newGame(false, false, 2);
    require(!restored.replaying() && !restored.gameOver() && restored.replayLength() == 0, "New game retained replay or result");
    AndroidGame settings(path);
    require(!settings.humanWhite() && !settings.againstComputer() && settings.difficulty() == 2, "Saved settings did not restore");
    const auto before = settings.savedState();
    require(!settings.restoreState("invalid JSON") && settings.savedState() == before, "Invalid save modified live state");
    auto corrupt = QJsonDocument::fromJson(before).object();
    corrupt["moves"] = QJsonArray{QJsonArray{6, 4, -1, 4, 5}};
    require(!settings.restoreState(QJsonDocument(corrupt).toJson()) && settings.savedState() == before, "Illegal saved move was accepted");
    corrupt["moves"] = QJsonArray{QJsonArray{6.5, 4, 4, 4, 5}};
    require(!settings.restoreState(QJsonDocument(corrupt).toJson()), "Fractional saved square was accepted");
    game.newGame(true, false, 0);
    for (int repeat = 0; repeat < 2; ++repeat)
        for (const auto& step : {std::pair{62,45}, {6,21}, {45,62}, {21,6}}) move(game, step.first, step.second);
    require(game.gameOver() && game.result().contains("threefold"), "Mobile repetition draw is missing");
    AndroidGame repetition(path);
    require(repetition.gameOver() && repetition.result().contains("threefold"), "Restore lost repetition history");
}

void testComputerAndBackgroundCancellation() {
    AndroidGame game;
    game.newGame(false, true, 0);
    require(game.thinking(), "Black game did not schedule White's AI move");
    const auto before = game.savedState();
    game.selectSquare(52); game.selectSquare(36);
    require(game.savedState() == before, "Player moved during the AI turn");
    waitForComputer(game);
    require(!game.position().whiteToMove && game.replayLength() == 1, "AI did not hand over Black's turn");
    game.newGame(false, true, 2);
    game.setApplicationActive(false);
    processFor(300);
    require(!game.thinking() && game.replayLength() == 0, "AI continued while app was in the background");
    game.setApplicationActive(true);
    require(game.thinking(), "AI did not resume when app became active");
    processFor(210);
    game.newGame(true, false, 0);
    processFor(500);
    require(game.replayLength() == 0 && game.position().squares == chess::Position::initial().squares,
            "Cancelled AI changed the new game");
    game.newGame(true, true, 0);
    move(game, 52, 36);
    require(game.thinking(), "AI reply was not scheduled");
    waitForComputer(game);
    require(game.position().whiteToMove && game.replayLength() == 2, "AI reply did not finish");
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    testSelectionAndOrientation();
    testPromotion();
    testResultsReplayAndPersistence();
    testComputerAndBackgroundCancellation();
    QThreadPool::globalInstance()->waitForDone();
}
