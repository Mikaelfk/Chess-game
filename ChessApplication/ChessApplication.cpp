#include "ChessApplication.h"

#include <QDialog>
#include <QFutureWatcher>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QStatusBar>
#include <QTimer>
#include <QVariantAnimation>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <cmath>
#include <random>

namespace {
constexpr int moveDurationMs = 220;
constexpr int computerMovePauseMs = 450;

QPixmap piecePixmap(int pieceType) {
    static const char* paths[] = {
        "", ":/Images/White Pawn.png", ":/Images/White Knight.png",
        ":/Images/White Bishop.png", ":/Images/White Rook.png",
        ":/Images/White Queen.png", ":/Images/White King.png",
        ":/Images/Black Pawn.png", ":/Images/Black Knight.png",
        ":/Images/Black Bishop.png", ":/Images/Black Rook.png",
        ":/Images/Black Queen.png", ":/Images/Black King.png"
    };
    if (pieceType < 1 || pieceType > 12) {
        return {};
    }
    return QPixmap(paths[pieceType]);
}

int choosePromotionPiece(QWidget* parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Pawn Promotion");
    auto* layout = new QVBoxLayout(&dialog);
    const char* names[] = {"Queen", "Rook", "Bishop", "Knight"};
    int selection = 0; // Closing the dialog keeps the default queen promotion.
    for (int type = 0; type < 4; ++type) {
        auto* button = new QPushButton(names[type], &dialog);
        layout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, &dialog, [&, type] {
            selection = type;
            dialog.accept();
        });
    }
    dialog.exec();
    return selection;
}
} // namespace

ChessApplication::ChessApplication(QWidget* parent, bool computerOpponent, chess::Position initial)
    : QMainWindow(parent), view(&scene) {
    ui.setupUi(this);
    setWindowTitle("Chess");
    auto* controls = new QHBoxLayout();
    opponentChoice = new QComboBox(this);
    opponentChoice->setObjectName("opponentChoice");
    opponentChoice->addItems({"Computer", "Local two-player"});
    opponentChoice->setCurrentIndex(computerOpponent ? 0 : 1);
    colourChoice = new QComboBox(this);
    colourChoice->setObjectName("colourChoice");
    colourChoice->addItems({"White", "Black"});
    difficultyChoice = new QComboBox(this);
    difficultyChoice->setObjectName("difficultyChoice");
    difficultyChoice->addItems({"Easy", "Medium", "Hard"});
    difficultyChoice->setCurrentIndex(1);
    auto* restart = new QPushButton("New game", this);
    restart->setObjectName("newGame");
    controls->addWidget(opponentChoice);
    controls->addWidget(new QLabel("Your side:", this));
    controls->addWidget(colourChoice);
    controls->addWidget(new QLabel("Difficulty:", this));
    controls->addWidget(difficultyChoice);
    controls->addWidget(restart);
    ui.boardLayout->addLayout(controls);
    auto* help = new QLabel("Choose a side and difficulty, then click New game.", this);
    ui.boardLayout->addWidget(help);
    difficultyChoice->setToolTip("Easy: relaxed play. Medium: balanced play. Hard: a stronger opponent with longer thinking time.");
    colourChoice->setToolTip("Your pieces appear at the bottom of the board.");
    connect(restart, &QPushButton::clicked, this, &ChessApplication::newGame);
    connect(opponentChoice, &QComboBox::currentIndexChanged, this, [this](int index) {
        difficultyChoice->setEnabled(index == 0);
    });
    difficultyChoice->setEnabled(computerOpponent);

    background = scene.addPixmap(QPixmap(":/Images/ChessBoard.png").scaled(500,500,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    background->setZValue(-1);
    background->setTransformOriginPoint(background->boundingRect().center());
    boardInput = new QPushButton();
    boardInput->setFlat(true);
    boardInput->setStyleSheet("QPushButton { background: transparent; border: 0px; }");
    boardInput->setFixedSize(500, 500);
    auto* proxy = scene.addWidget(boardInput);
    proxy->setZValue(3);
    connect(boardInput, &QPushButton::released, this, &ChessApplication::on_pushButtonClicked);
    scene.setSceneRect(0,0,500,500);
    view.setFixedSize(502,502);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    topCaptures = new QHBoxLayout();
    bottomCaptures = new QHBoxLayout();
    topPlayer = new QLabel(this);
    bottomPlayer = new QLabel(this);
    topCaptures->addWidget(topPlayer);
    bottomCaptures->addWidget(bottomPlayer);
    ui.boardLayout->addLayout(topCaptures);
    ui.boardLayout->addWidget(&view, 0, Qt::AlignHCenter);
    ui.boardLayout->addLayout(bottomCaptures);
    status = new QLabel(this);
    status->setObjectName("gameStatus");
    ui.statusBar->addWidget(status,1);
    startGame(initial);
}

ChessApplication::~ChessApplication() {
    if (cancelled) cancelled->store(true);
    stopMoveAnimation();
}

void ChessApplication::stopMoveAnimation() {
    if (!moveAnimation) return;
    moveAnimation->stop();
    delete moveAnimation;
    moveAnimation = nullptr;
}

QPoint ChessApplication::displaySquare(int row, int column) const {
    return humanWhite ? QPoint(column,row) : QPoint(7-column,7-row);
}

void ChessApplication::newGame() {
    startGame(chess::Position::initial());
}

void ChessApplication::startGame(chess::Position position) {
    if (cancelled) cancelled->store(true);
    ++generation;
    stopMoveAnimation();
    thinking = false;
    game = position;
    humanWhite = colourChoice->currentIndex() == 0;
    againstComputer = opponentChoice->currentIndex() == 0;
    difficulty = static_cast<chess::Difficulty>(difficultyChoice->currentIndex());
    selectedMoves.clear();
    capturedPieces.clear();
    repetitions.clear();
    repetitions[game.repetitionKey()] = 1;
    background->setRotation(humanWhite ? 0 : 180);
    topPlayer->setText(QString("%1 (%2)").arg(againstComputer ? "Computer" : "Player 2", humanWhite ? "Black" : "White"));
    bottomPlayer->setText(QString("%1 (%2)").arg(againstComputer ? "You" : "Player 1", humanWhite ? "White" : "Black"));
    render();
    queueComputerMove();
}

void ChessApplication::render() {
    for (auto* item : pieceItems) delete item;
    for (auto* item : overlays) delete item;
    for (auto* label : captureLabels) delete label;
    pieceItems.clear(); overlays.clear(); captureLabels.clear();
    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col) {
        const int piece = game.squares[row][col];
        if (!piece) continue;
        auto* item = scene.addPixmap(piecePixmap(piece));
        const QPoint square = displaySquare(row,col);
        item->setPos(square.x()*62.5,square.y()*62.5);
        pieceItems.push_back(item);
        if (chess::kind(piece) == chess::King && game.inCheck(chess::isWhite(piece))) {
            auto* warning = scene.addRect(square.x()*62.5,square.y()*62.5,62.5,62.5,Qt::NoPen,Qt::red);
            warning->setOpacity(0.4);
            warning->setZValue(1);
            overlays.push_back(warning);
        }
    }
    for (const auto& move : selectedMoves) {
        const QPoint square = displaySquare(move.toRow,move.toCol);
        auto* hint = scene.addEllipse(square.x()*62.5+21,square.y()*62.5+21,20,20,Qt::NoPen,Qt::black);
        hint->setOpacity(0.3);
        hint->setZValue(2);
        overlays.push_back(hint);
    }
    auto captures = capturedPieces;
    std::sort(captures.begin(),captures.end());
    for (int piece : captures) {
        auto* label = new QLabel(this);
        label->setPixmap(piecePixmap(piece));
        label->setFixedSize(30,30);
        label->setScaledContents(true);
        (chess::isWhite(piece) == humanWhite ? topCaptures : bottomCaptures)->addWidget(label);
        captureLabels.push_back(label);
    }
    const auto result = game.outcome();
    const bool repetition = repetitions[game.repetitionKey()] >= 3;
    gameOver = result != chess::Outcome::Playing || repetition;
    if (result == chess::Outcome::Checkmate) {
        if (againstComputer) status->setText(game.whiteToMove == humanWhite ? "Checkmate - Computer wins." : "Checkmate - You win!");
        else status->setText(game.whiteToMove ? "Checkmate - Black wins." : "Checkmate - White wins.");
    } else if (result == chess::Outcome::Stalemate) status->setText("Draw - stalemate.");
    else if (result == chess::Outcome::FiftyMoveDraw) status->setText("Draw - fifty-move rule.");
    else if (result == chess::Outcome::InsufficientMaterial) status->setText("Draw - insufficient material.");
    else if (repetition) status->setText("Draw - threefold repetition.");
    else if (thinking) status->setText("Computer is thinking...");
    else {
        const QString turn = againstComputer ? "Your turn" : (game.whiteToMove ? "White to move" : "Black to move");
        status->setText(turn + (game.inCheck(game.whiteToMove) ? " - Check!" : ""));
    }
    boardInput->setEnabled(!gameOver && !thinking && !moveAnimating() && (!againstComputer || game.whiteToMove == humanWhite));
}

void ChessApplication::on_pushButtonClicked() {
    if (gameOver || thinking || moveAnimating() || (againstComputer && game.whiteToMove != humanWhite)) return;
    const QPoint point = boardInput->mapFromGlobal(QCursor::pos());
    if (point.x() < 0 || point.y() < 0 || point.x() >= 500 || point.y() >= 500) return;
    const int col = humanWhite ? int(point.x()/62.5) : 7-int(point.x()/62.5);
    const int row = humanWhite ? int(point.y()/62.5) : 7-int(point.y()/62.5);
    const auto found = std::find_if(selectedMoves.begin(),selectedMoves.end(),[&](const chess::Move& move) {
        return move.toRow == row && move.toCol == col;
    });
    if (found != selectedMoves.end()) {
        chess::Move move = *found;
        if (chess::kind(game.squares[move.fromRow][move.fromCol]) == chess::Pawn && (row == 0 || row == 7)) {
            const unsigned current = generation;
            const int choices[] = {chess::Queen,chess::Rook,chess::Bishop,chess::Knight};
            move.promotion = choices[choosePromotionPiece(this)];
            if (current != generation) return;
        }
        applyMove(move);
        return;
    }
    selectedMoves.clear();
    for (const auto& move : game.legalMoves()) if (move.fromRow == row && move.fromCol == col) selectedMoves.push_back(move);
    render();
}

void ChessApplication::applyMove(const chess::Move& move) {
    if (moveAnimating()) return;
    const int piece = game.squares[move.fromRow][move.fromCol];
    int captured = game.squares[move.toRow][move.toCol];
    if (chess::kind(piece) == chess::Pawn && move.fromCol != move.toCol && !captured) {
        captured = game.squares[move.fromRow][move.toCol];
    }
    if (!game.play(move)) return;
    if (captured) capturedPieces.push_back(captured);
    ++repetitions[game.repetitionKey()];
    selectedMoves.clear();
    for (auto* item : overlays) delete item;
    overlays.clear();

    // Keep the previous board visible until the pieces reach their destinations.
    // Captures disappear and promoted pawns change appearance on arrival.
    boardInput->setEnabled(false);
    status->setText("Moving...");
    moveAnimation = new QParallelAnimationGroup(this);
    auto animatePiece = [&](int fromRow, int fromCol, int toRow, int toCol) {
        const QPoint from = displaySquare(fromRow,fromCol);
        const QPoint to = displaySquare(toRow,toCol);
        for (auto* item : pieceItems) {
            if (item->pos() != QPointF(from.x()*62.5,from.y()*62.5)) continue;
            item->setZValue(2); // Slide above stationary pieces during captures.
            auto* motion = new QVariantAnimation();
            motion->setDuration(moveDurationMs);
            motion->setStartValue(item->pos());
            motion->setEndValue(QPointF(to.x()*62.5,to.y()*62.5));
            motion->setEasingCurve(QEasingCurve::InOutCubic);
            connect(motion,&QVariantAnimation::valueChanged,this,[item](const QVariant& value) {
                item->setPos(value.toPointF());
            });
            moveAnimation->addAnimation(motion);
            break;
        }
    };
    animatePiece(move.fromRow,move.fromCol,move.toRow,move.toCol);
    if (chess::kind(piece) == chess::King && std::abs(move.toCol-move.fromCol) == 2) {
        animatePiece(move.fromRow,move.toCol == 6 ? 7 : 0,move.toRow,move.toCol == 6 ? 5 : 3);
    }
    auto* animation = moveAnimation;
    if (animation->animationCount() == 0) {
        stopMoveAnimation();
        render();
        queueComputerMove();
        return;
    }
    const unsigned current = generation;
    connect(animation,&QParallelAnimationGroup::finished,this,[this,animation,current] {
        if (current != generation) return;
        moveAnimation = nullptr;
        animation->deleteLater();
        render();
        queueComputerMove();
    });
    animation->start();
}

void ChessApplication::queueComputerMove() {
    if (!againstComputer || gameOver || game.whiteToMove == humanWhite || thinking || moveAnimating()) return;
    thinking = true;
    cancelled = std::make_shared<std::atomic_bool>(false);
    const auto token = cancelled;
    const auto snapshot = game;
    const auto level = difficulty;
    const unsigned current = generation;
    render();
    // Give the just-played move and thinking indicator time to paint first.
    QTimer::singleShot(100,this,[this,token,snapshot,level,current] {
        if (token->load() || current != generation) return;
        auto* watcher = new QFutureWatcher<chess::SearchResult>(this);
        connect(watcher,&QFutureWatcher<chess::SearchResult>::finished,this,[this,watcher,current,token] {
            const auto result = watcher->result();
            watcher->deleteLater();
            if (current != generation || token->load()) return;
            if (!result.move) {
                thinking = false;
                render();
                return;
            }
            // Even a fast search gets a visible pause; never block the GUI thread.
            QTimer::singleShot(computerMovePauseMs,this,[this,current,token,move=*result.move] {
                if (current != generation || token->load()) return;
                thinking = false;
                applyMove(move);
            });
        });
        watcher->setFuture(QtConcurrent::run([snapshot,level,token] {
            return chess::chooseMove(snapshot,level,*token,std::random_device{}());
        }));
    });
}
