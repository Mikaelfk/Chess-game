#include "ChessApplication.h"

#include <QDialog>
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
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
#include <set>

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
    boardInput->installEventFilter(this);
    installEventFilter(this);
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
    selectedSquare.reset();
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
    resetDrag();
    for (auto* item : pieceItems) delete item;
    for (auto* item : overlays) delete item;
    for (auto* label : captureLabels) delete label;
    pieceItems.clear(); overlays.clear(); captureLabels.clear();
    if (selectedSquare) {
        const QPoint square = displaySquare(selectedSquare->y(),selectedSquare->x());
        auto* selection = scene.addRect(square.x()*62.5,square.y()*62.5,62.5,62.5,
                                       Qt::NoPen,QColor(246,196,65));
        selection->setOpacity(0.7);
        selection->setZValue(-0.5); // Above the board, beneath pieces and check warnings.
        overlays.push_back(selection);
    }
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
    std::set<std::pair<int,int>> hintedSquares;
    for (const auto& move : selectedMoves) {
        if (!hintedSquares.emplace(move.toRow,move.toCol).second) continue;
        const QPoint square = displaySquare(move.toRow,move.toCol);
        const bool capture = game.squares[move.toRow][move.toCol] != chess::Empty
            || (chess::kind(game.squares[move.fromRow][move.fromCol]) == chess::Pawn && move.fromCol != move.toCol);
        auto* hint = capture
            ? scene.addEllipse(square.x()*62.5+4,square.y()*62.5+4,54.5,54.5,QPen(QColor(160,35,35),4),Qt::NoBrush)
            : scene.addEllipse(square.x()*62.5+21,square.y()*62.5+21,20,20,Qt::NoPen,Qt::black);
        hint->setOpacity(capture ? 0.85 : 0.3);
        hint->setZValue(2);
        overlays.push_back(hint);
    }
    std::map<int,int> captures;
    for (int piece : capturedPieces) ++captures[piece];
    for (const auto& [piece,count] : captures) {
        auto* group = new QWidget(this);
        group->setObjectName(QString("capturedPiece%1").arg(piece));
        auto* layout = new QHBoxLayout(group);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(3);
        if (count > 1) {
            auto* number = new QLabel(QString("%1x").arg(count),group);
            layout->addWidget(number);
        }
        auto* icon = new QLabel(group);
        icon->setPixmap(piecePixmap(piece));
        icon->setFixedSize(30,30);
        icon->setScaledContents(true);
        layout->addWidget(icon);
        group->setFixedSize(group->sizeHint());
        (chess::isWhite(piece) == humanWhite ? topCaptures : bottomCaptures)->addWidget(group);
        captureLabels.push_back(group);
    }
    // Count material still on the board so promotions are included as well.
    const int values[] = {0,1,3,3,5,9,0};
    int balance = 0;
    for (const auto& row : game.squares) for (int piece : row)
        if (piece) balance += (chess::isWhite(piece) ? 1 : -1)*values[chess::kind(piece)];
    if (balance != 0) {
        const bool whiteLeads = balance > 0;
        auto* advantage = new QLabel(QString("+%1").arg(std::abs(balance)),this);
        advantage->setObjectName(whiteLeads ? "whiteMaterialAdvantage" : "blackMaterialAdvantage");
        advantage->setToolTip("Material advantage: pawn 1, knight/bishop 3, rook 5, queen 9");
        auto font = advantage->font();
        font.setBold(true);
        advantage->setFont(font);
        (whiteLeads == humanWhite ? bottomCaptures : topCaptures)->addWidget(advantage);
        captureLabels.push_back(advantage);
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
    if (!inputAllowed()) return;
    const QPoint square = boardSquare(boardInput->mapFromGlobal(QCursor::pos()));
    if (square.x() < 0) return;
    if (!moveTo(square)) selectSquare(square);
}

bool ChessApplication::inputAllowed() const {
    return !gameOver && !thinking && !moveAnimating() && (!againstComputer || game.whiteToMove == humanWhite);
}

QPoint ChessApplication::boardSquare(QPointF point) const {
    if (point.x() < 0 || point.y() < 0 || point.x() >= 500 || point.y() >= 500) return {-1,-1};
    const int col = int(point.x()/62.5), row = int(point.y()/62.5);
    return humanWhite ? QPoint(col,row) : QPoint(7-col,7-row);
}

void ChessApplication::resetDrag() {
    if (draggedPiece) {
        draggedPiece->setPos(dragOrigin);
        draggedPiece->setZValue(0);
    }
    draggedPiece = nullptr;
    boardPressed = dragging = false;
    boardInput->unsetCursor();
}

void ChessApplication::selectSquare(QPoint square) {
    selectedMoves.clear();
    selectedSquare.reset();
    const int piece = game.squares[square.y()][square.x()];
    if (piece && chess::isWhite(piece) == game.whiteToMove) selectedSquare = square;
    for (const auto& move : game.legalMoves())
        if (move.fromRow == square.y() && move.fromCol == square.x()) selectedMoves.push_back(move);
    render();
}

bool ChessApplication::eventFilter(QObject* watched, QEvent* event) {
    if ((event->type() == QEvent::WindowDeactivate)
        || (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)) {
        resetDrag();
    }
    if (watched != boardInput) return QMainWindow::eventFilter(watched,event);
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() != Qt::LeftButton) return true;
        resetDrag();
        if (!inputAllowed()) return true;
        const QPoint square = boardSquare(mouse->position());
        if (square.x() < 0) return true;
        const int piece = game.squares[square.y()][square.x()];
        if (piece && chess::isWhite(piece) == game.whiteToMove) {
            selectSquare(square);
            const QPoint displayed = displaySquare(square.y(),square.x());
            dragOrigin = QPointF(displayed.x()*62.5,displayed.y()*62.5);
            for (auto* item : pieceItems) if (item->pos() == dragOrigin) draggedPiece = item;
            dragOffset = mouse->position()-dragOrigin;
        }
        pressPosition = mouse->position();
        boardPressed = true;
        mouse->accept();
        return true;
    }
    if (event->type() == QEvent::MouseMove && boardPressed) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (draggedPiece && (mouse->position()-pressPosition).manhattanLength() >= QApplication::startDragDistance()) dragging = true;
        if (dragging) {
            draggedPiece->setZValue(4);
            draggedPiece->setPos(mouse->position()-dragOffset);
            boardInput->setCursor(Qt::ClosedHandCursor);
        }
        return true;
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() != Qt::LeftButton || !boardPressed) return true;
        const bool wasDragging = dragging;
        const auto start = dragging ? std::optional<QPointF>(draggedPiece->pos()) : std::nullopt;
        const QPoint square = boardSquare(mouse->position());
        resetDrag();
        if (!inputAllowed()) return true;
        if (square.x() >= 0 && moveTo(square,start)) return true;
        if (!wasDragging && square.x() >= 0) selectSquare(square);
        return true;
    }
    return QMainWindow::eventFilter(watched,event);
}

bool ChessApplication::moveTo(QPoint square, std::optional<QPointF> dragStart) {
    const int row = square.y(), col = square.x();
    const auto found = std::find_if(selectedMoves.begin(),selectedMoves.end(),[&](const chess::Move& move) {
        return move.toRow == row && move.toCol == col;
    });
    if (found != selectedMoves.end()) {
        chess::Move move = *found;
        if (chess::kind(game.squares[move.fromRow][move.fromCol]) == chess::Pawn && (row == 0 || row == 7)) {
            const unsigned current = generation;
            const int choices[] = {chess::Queen,chess::Rook,chess::Bishop,chess::Knight};
            move.promotion = choices[choosePromotionPiece(this)];
            if (current != generation) return true;
        }
        applyMove(move,dragStart);
        return true;
    }
    return false;
}

void ChessApplication::applyMove(const chess::Move& move, std::optional<QPointF> dragStart) {
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
    selectedSquare.reset();
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
            if (dragStart && fromRow == move.fromRow && fromCol == move.fromCol) item->setPos(*dragStart);
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
