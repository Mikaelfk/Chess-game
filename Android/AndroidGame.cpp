#include "AndroidGame.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <random>

AndroidGame::AndroidGame(const QString& stateFile, QObject* parent)
    : QObject(parent), stateFile(stateFile) {
    history = {current};
    repetitions[current.repetitionKey()] = 1;
    if (!stateFile.isEmpty()) {
        QFile file(stateFile);
        if (file.open(QIODevice::ReadOnly) && !restoreState(file.readAll()))
            storageError = "The saved game could not be loaded. A new game is ready.";
    }
}

AndroidGame::~AndroidGame() { cancelSearch(); }

const chess::Position& AndroidGame::displayedPosition() const {
    return replay ? history[replayStep] : current;
}

bool AndroidGame::gameOver() const {
    const auto count = repetitions.find(current.repetitionKey());
    return current.outcome() != chess::Outcome::Playing
        || (count != repetitions.end() && count->second >= 3);
}

bool AndroidGame::inputAllowed() const {
    return active && !replay && !searching && !pending && !gameOver()
        && (!computer || current.whiteToMove == playerWhite);
}

QVariantList AndroidGame::board() const {
    const auto& p = displayedPosition();
    const auto legal = inputAllowed() && selected >= 0 ? p.legalMoves() : std::vector<chess::Move>{};
    static const char* names[] = {"", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
    QVariantList squares;
    for (int screen = 0; screen < 64; ++screen) {
        const int square = playerWhite ? screen : 63 - screen;
        const int row = square / 8, col = square % 8, piece = p.squares[row][col];
        const bool target = std::any_of(legal.begin(), legal.end(), [&](const chess::Move& move) {
            return move.fromRow * 8 + move.fromCol == selected && move.toRow == row && move.toCol == col;
        });
        const bool capture = target && (piece != chess::Empty
            || (chess::kind(p.squares[selected / 8][selected % 8]) == chess::Pawn && col != selected % 8));
        const QString name = piece ? QString("%1 %2").arg(chess::isWhite(piece) ? "White" : "Black", names[chess::kind(piece)]) : QString();
        squares.append(QVariantMap{
            {"index", square}, {"piece", piece}, {"name", name},
            {"image", piece ? "qrc:/Images/" + name + ".png" : QString()},
            {"selected", !replay && selected == square}, {"legal", target}, {"capture", capture},
            {"check", chess::kind(piece) == chess::King && p.inCheck(chess::isWhite(piece))},
            {"coordinate", QString(QChar('a' + col)) + QString::number(8 - row)}
        });
    }
    return squares;
}

QString AndroidGame::result() const {
    switch (current.outcome()) {
    case chess::Outcome::Checkmate:
        return current.whiteToMove ? "Black wins by checkmate" : "White wins by checkmate";
    case chess::Outcome::Stalemate: return "Draw by stalemate";
    case chess::Outcome::FiftyMoveDraw: return "Draw by the fifty-move rule";
    case chess::Outcome::InsufficientMaterial: return "Draw by insufficient material";
    case chess::Outcome::Playing: break;
    }
    const auto count = repetitions.find(current.repetitionKey());
    return count != repetitions.end() && count->second >= 3 ? "Draw by threefold repetition" : QString();
}

QString AndroidGame::status() const {
    if (replay) return QString("Replay: move %1 of %2").arg(replayStep).arg(moves.size());
    if (gameOver()) return result();
    if (pending) return "Choose a promotion piece";
    if (!active) return "Game paused";
    if (searching) return "Computer is thinking…";
    const QString turn = computer ? (current.whiteToMove == playerWhite ? "Your turn" : "Computer's turn")
                                  : (current.whiteToMove ? "White to move" : "Black to move");
    return turn + (current.inCheck(current.whiteToMove) ? " — Check!" : "");
}

QString AndroidGame::material() const {
    constexpr int values[] = {0, 1, 3, 3, 5, 9, 0};
    int balance = 0;
    for (const auto& row : displayedPosition().squares) for (int piece : row)
        if (piece) balance += (chess::isWhite(piece) ? 1 : -1) * values[chess::kind(piece)];
    return balance == 0 ? "Equal material" : QString("%1 +%2").arg(balance > 0 ? "White" : "Black").arg(std::abs(balance));
}

void AndroidGame::selectSquare(int index) {
    if (index < 0 || index >= 64 || !inputAllowed()) return;
    if (selected >= 0) for (const auto& move : current.legalMoves()) {
        if (move.fromRow * 8 + move.fromCol != selected || move.toRow * 8 + move.toCol != index) continue;
        if (chess::kind(current.squares[move.fromRow][move.fromCol]) == chess::Pawn && (move.toRow == 0 || move.toRow == 7)) {
            pending = move;
            emit stateChanged();
        } else applyMove(move);
        return;
    }
    const int piece = current.squares[index / 8][index % 8];
    selected = piece && chess::isWhite(piece) == current.whiteToMove ? index : -1;
    emit stateChanged();
}

void AndroidGame::promote(int piece) {
    if (!pending || (piece != chess::Queen && piece != chess::Rook && piece != chess::Bishop && piece != chess::Knight)) return;
    auto move = *pending;
    move.promotion = piece;
    pending.reset();
    applyMove(move);
}

void AndroidGame::cancelPromotion() {
    pending.reset();
    emit stateChanged();
}

void AndroidGame::applyMove(const chess::Move& move) {
    const bool castle = chess::kind(current.squares[move.fromRow][move.fromCol]) == chess::King
        && std::abs(move.toCol - move.fromCol) == 2;
    if (!current.play(move)) return;
    selected = -1;
    moves.push_back(move);
    history.push_back(current);
    ++repetitions[current.repetitionKey()];
    save();
    emit stateChanged();
    emit moveMade(move.fromRow * 8 + move.fromCol, move.toRow * 8 + move.toCol,
                  castle ? move.fromRow * 8 + (move.toCol > move.fromCol ? 7 : 0) : -1,
                  castle ? move.fromRow * 8 + (move.toCol > move.fromCol ? 5 : 3) : -1);
    queueComputerMove();
}

void AndroidGame::cancelSearch() {
    if (cancelled) cancelled->store(true);
    ++generation;
    searching = false;
}

void AndroidGame::newGame(bool white, bool computerOpponent, int difficulty) {
    if (difficulty < 0 || difficulty > 2) return;
    cancelSearch();
    playerWhite = white; computer = computerOpponent; level = difficulty;
    current = chess::Position::initial();
    history = {current}; moves.clear(); repetitions.clear();
    repetitions[current.repetitionKey()] = 1;
    pending.reset(); selected = -1; replay = false; replayStep = 0;
    save();
    emit stateChanged();
    queueComputerMove();
}

void AndroidGame::queueComputerMove() {
    if (!active || replay || pending || searching || !computer || gameOver() || current.whiteToMove == playerWhite) return;
    searching = true;
    cancelled = std::make_shared<std::atomic_bool>(false);
    const auto token = cancelled;
    const auto snapshot = current;
    const auto difficulty = static_cast<chess::Difficulty>(level);
    const unsigned request = generation;
    emit stateChanged();
    // Let the board and thinking status paint before doing background work.
    QTimer::singleShot(500, this, [this, token, snapshot, difficulty, request] {
        if (token->load() || request != generation) return;
        auto* watcher = new QFutureWatcher<chess::SearchResult>(this);
        connect(watcher, &QFutureWatcher<chess::SearchResult>::finished, this, [this, watcher, token, request] {
            const auto result = watcher->result();
            watcher->deleteLater();
            if (token->load() || request != generation) return;
            searching = false;
            if (result.move) applyMove(*result.move);
            else emit stateChanged();
        });
        watcher->setFuture(QtConcurrent::run([snapshot, difficulty, token] {
            return chess::chooseMove(snapshot, difficulty, *token, std::random_device{}());
        }));
    });
}

void AndroidGame::setApplicationActive(bool value) {
    active = value;
    if (!active) { cancelSearch(); save(); }
    emit stateChanged();
    if (active) queueComputerMove();
}

void AndroidGame::startReplay() {
    if (!gameOver() || moves.empty()) return;
    replay = true; replayStep = 0; selected = -1;
    emit stateChanged();
}

void AndroidGame::seekReplay(int step) {
    if (!replay) return;
    replayStep = std::clamp(step, 0, int(moves.size()));
    emit stateChanged();
}

void AndroidGame::exitReplay() {
    replay = false;
    emit stateChanged();
}

QByteArray AndroidGame::savedState() const {
    QJsonArray timeline;
    for (const auto& move : moves) timeline.append(QJsonArray{move.fromRow, move.fromCol, move.toRow, move.toCol, move.promotion});
    return QJsonDocument(QJsonObject{{"version", 1}, {"white", playerWhite}, {"computer", computer},
                                   {"difficulty", level}, {"moves", timeline}}).toJson(QJsonDocument::Compact);
}

bool AndroidGame::restoreState(const QByteArray& data) {
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) return false;
    const auto object = document.object();
    const int difficulty = object.value("difficulty").toInt(-1);
    if (object.value("version").toInt() != 1 || !object.value("white").isBool()
        || !object.value("computer").isBool() || !object.value("moves").isArray()
        || difficulty < 0 || difficulty > 2 || object.value("moves").toArray().size() > 10000) return false;
    auto position = chess::Position::initial();
    std::vector<chess::Position> restoredHistory{position};
    std::vector<chess::Move> restoredMoves;
    std::map<std::string, int> counts{{position.repetitionKey(), 1}};
    for (const auto& value : object.value("moves").toArray()) {
        if (!value.isArray() || value.toArray().size() != 5) return false;
        const auto fields = value.toArray();
        for (const auto& field : fields) if (!field.isDouble() || field.toDouble() != field.toInt(-100)) return false;
        const chess::Move move{fields[0].toInt(), fields[1].toInt(), fields[2].toInt(), fields[3].toInt(), fields[4].toInt()};
        if (position.outcome() != chess::Outcome::Playing || counts[position.repetitionKey()] >= 3 || !position.play(move)) return false;
        restoredMoves.push_back(move); restoredHistory.push_back(position);
        ++counts[position.repetitionKey()];
    }
    cancelSearch();
    current = position; moves = std::move(restoredMoves); history = std::move(restoredHistory); repetitions = std::move(counts);
    playerWhite = object.value("white").toBool(); computer = object.value("computer").toBool(); level = difficulty;
    pending.reset(); selected = -1; replay = false; replayStep = 0; storageError.clear();
    emit stateChanged();
    queueComputerMove();
    return true;
}

void AndroidGame::save() {
    if (stateFile.isEmpty()) return;
    QSaveFile file(stateFile);
    if (!QDir().mkpath(QFileInfo(stateFile).absolutePath()) || !file.open(QIODevice::WriteOnly)) {
        storageError = "Could not save the game on this device.";
        return;
    }
    const auto data = savedState();
    if (file.write(data) != data.size() || !file.commit()) storageError = "Could not save the game on this device.";
    else storageError.clear();
}
