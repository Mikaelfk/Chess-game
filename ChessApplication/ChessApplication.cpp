#include "ChessApplication.h"

#include <QDialog>
#include <QFrame>
#include <QProgressBar>
#include <QStyle>
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

int choosePromotionPiece(QWidget* parent);

// Replay owns a copy of the timeline and never changes the live game.
class GameReplay : public QDialog {
    std::vector<chess::Position> positions;
    std::vector<chess::Move> moves;
    const std::vector<chess::Position> originalPositions;
    const std::vector<chess::Move> originalMoves;
    std::optional<int> branchRoot;
    std::optional<QPoint> selected;
    QGraphicsView* view;
    QProgressBar* evaluation;
    QLabel* suggestion;
    QPushButton* restore;
    std::shared_ptr<std::atomic_bool> searchCancelled;
    unsigned searchGeneration = 0;
    bool white;
    int index = 0;
    QGraphicsScene scene;
    QTimer timer;
    QVariantAnimation animation;
    QLabel* progress;
    QPushButton *first, *previous, *play, *next, *last;

    QPointF point(int row,int col) const {
        return white ? QPointF(col*62.5,row*62.5) : QPointF((7-col)*62.5,(7-row)*62.5);
    }
    void paint(double t = 1) {
        scene.clear();
        auto* board=scene.addPixmap(QPixmap(":/Images/ChessBoard.png").scaled(500,500,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
        board->setZValue(-1);
        board->setTransformOriginPoint(250,250);
        board->setRotation(white ? 0 : 180);
        const bool moving = t < 1 && index > 0;
        const auto& position=positions[moving ? index-1 : index];
        if(selected && !moving) {
            auto p=point(selected->y(),selected->x());
            scene.addRect(p.x(),p.y(),62.5,62.5,Qt::NoPen,QColor(246,196,65))->setZValue(-0.5);
            for(auto m:position.legalMoves()) if(m.fromRow==selected->y() && m.fromCol==selected->x()) {
                auto to=point(m.toRow,m.toCol);
                scene.addEllipse(to.x()+23,to.y()+23,16,16,Qt::NoPen,QColor(40,140,180))->setZValue(1);
            }
        }
        for(int row=0;row<8;++row) for(int col=0;col<8;++col) {
            const int piece=position.squares[row][col];
            if(!piece) continue;
            auto* item=scene.addPixmap(piecePixmap(piece));
            QPointF location=point(row,col);
            if(moving) {
                const auto move=moves[index-1];
                if(row==move.fromRow && col==move.fromCol) {
                    location+=(point(move.toRow,move.toCol)-location)*t;
                    item->setZValue(2);
                } else if(chess::kind(position.squares[move.fromRow][move.fromCol])==chess::King
                          && std::abs(move.toCol-move.fromCol)==2 && row==move.fromRow
                          && col==(move.toCol==6 ? 7 : 0)) {
                    location+=(point(row,move.toCol==6 ? 5 : 3)-location)*t;
                }
            }
            item->setPos(location);
        }
    }
    void updateAnalysis() {
        if(searchCancelled) searchCancelled->store(true);
        const unsigned generation=++searchGeneration;
        const auto snapshot=positions[index];
        const int values[]={0,1,3,3,5,9,0};
        int balance=0;
        for(const auto& row:snapshot.squares) for(int p:row)
            if(p) balance+=(chess::isWhite(p)?1:-1)*values[chess::kind(p)];
        evaluation->setValue(std::clamp(100+balance*5,0,200));
        evaluation->setFormat(balance==0 ? "Equal material" : QString("%1 +%2 (material)").arg(balance>0?"White":"Black").arg(std::abs(balance)));
        suggestion->setText("Finding a move for " + QString(snapshot.whiteToMove?"White…":"Black…"));
        suggestion->setProperty("ready",false);
        searchCancelled=std::make_shared<std::atomic_bool>(false);
        const auto token=searchCancelled;
        auto* watcher=new QFutureWatcher<chess::SearchResult>(this);
        connect(watcher,&QFutureWatcher<chess::SearchResult>::finished,this,[this,watcher,generation,snapshot] {
            const auto result=watcher->result(); watcher->deleteLater();
            if(generation!=searchGeneration) return;
            suggestion->setProperty("ready",true);
            if(!result.move) {
                const bool mate=snapshot.outcome()==chess::Outcome::Checkmate;
                evaluation->setValue(mate ? (snapshot.whiteToMove?0:200) : 100);
                evaluation->setFormat(mate ? (snapshot.whiteToMove?"Black wins — checkmate":"White wins — checkmate") : "Draw");
                suggestion->setText("No move suggestion — game over."); return;
            }
            if(result.whiteEvaluation) {
                const int score=*result.whiteEvaluation;
                evaluation->setValue(int(100+100*std::tanh(score/600.0)));
                evaluation->setFormat(std::abs(score)>90000 ? (score>0?"White — forced mate":"Black — forced mate")
                    : score==0 ? "Equal (engine)" : QString("%1 +%2 (engine)").arg(score>0?"White":"Black").arg(std::abs(score)/100.0,0,'f',2));
            }
            const auto m=*result.move;
            QString text=QString("Suggested for %1: %2%3 → %4%5")
                .arg(snapshot.whiteToMove?"White":"Black")
                .arg(QChar('a'+m.fromCol)).arg(8-m.fromRow).arg(QChar('a'+m.toCol)).arg(8-m.toRow);
            if(chess::kind(snapshot.squares[m.fromRow][m.fromCol])==chess::Pawn && (m.toRow==0 || m.toRow==7)) {
                const char* names[]={"","pawn","knight","bishop","rook","queen","king"};
                text+=" = "+QString(names[m.promotion]);
            }
            suggestion->setText(text+QString(" (depth %1)").arg(result.completedDepth));
        });
        watcher->setFuture(QtConcurrent::run([snapshot,token] { return chess::chooseMove(snapshot,chess::Difficulty::Hard,*token,42); }));
    }
    bool eventFilter(QObject* watched,QEvent* event) override {
        if(watched!=view->viewport() || event->type()!=QEvent::MouseButtonRelease) return QDialog::eventFilter(watched,event);
        auto* mouse=static_cast<QMouseEvent*>(event);
        if(mouse->button()!=Qt::LeftButton || animation.state()==QAbstractAnimation::Running) return true;
        pause();
        auto p=view->mapToScene(mouse->position().toPoint());
        if(p.x()<0 || p.y()<0 || p.x()>=500 || p.y()>=500) return true;
        int col=int(p.x()/62.5),row=int(p.y()/62.5);
        if(!white) { row=7-row; col=7-col; }
        if(selected) for(auto move:positions[index].legalMoves()) {
            if(move.fromRow!=selected->y() || move.fromCol!=selected->x() || move.toRow!=row || move.toCol!=col) continue;
            if(chess::kind(positions[index].squares[move.fromRow][move.fromCol])==chess::Pawn && (row==0 || row==7)) {
                const int types[]={chess::Queen,chess::Rook,chess::Bishop,chess::Knight};
                move.promotion=types[choosePromotionPiece(this)];
            }
            auto position=positions[index];
            if(!position.play(move)) return true;
            if(!branchRoot) branchRoot=index;
            positions.resize(index+1); moves.resize(index);
            moves.push_back(move); positions.push_back(position);
            go(index+1,true); return true;
        }
        selected.reset();
        const int piece=positions[index].squares[row][col];
        if(piece && chess::isWhite(piece)==positions[index].whiteToMove) selected=QPoint(col,row);
        paint(); return true;
    }
    void pause() { timer.stop(); play->setText("Play"); }
    void go(int target,bool animate=false) {
        animation.stop();
        selected.reset();
        index=std::clamp(target,0,int(moves.size()));
        QString text=QString("Move %1 of %2").arg(index).arg(moves.size());
        if(index) {
            const auto move=moves[index-1];
            text+=QString(" — %1: %2%3 → %4%5")
                .arg(positions[index-1].whiteToMove ? "White" : "Black")
                .arg(QChar('a'+move.fromCol)).arg(8-move.fromRow)
                .arg(QChar('a'+move.toCol)).arg(8-move.toRow);
        } else text+=" — Starting position";
        if(branchRoot) text="Analysis variation · "+text;
        progress->setText(text);
        restore->setEnabled(branchRoot.has_value());
        updateAnalysis();
        first->setEnabled(index>0); previous->setEnabled(index>0);
        next->setEnabled(index<int(moves.size())); last->setEnabled(index<int(moves.size()));
        if(index==int(moves.size())) pause();
        if(animate && index>0) { paint(0); animation.start(); } else paint();
    }
public:
    GameReplay(QWidget* parent,const std::vector<chess::Position>& positions,
               const std::vector<chess::Move>& moves,bool white)
        : QDialog(parent),positions(positions),moves(moves),originalPositions(positions),originalMoves(moves),white(white) {
        setObjectName("gameReplayDialog"); setWindowTitle("Game replay");
        setAttribute(Qt::WA_DeleteOnClose);
        auto* layout=new QVBoxLayout(this);
        progress=new QLabel(this); progress->setObjectName("replayProgress");
        progress->setAlignment(Qt::AlignCenter); layout->addWidget(progress);
        evaluation=new QProgressBar(this); evaluation->setObjectName("replayEvaluation");
        evaluation->setRange(0,200); evaluation->setAlignment(Qt::AlignCenter);
        evaluation->setToolTip("Built-in engine evaluation in pawn units; material balance while searching. Not a win probability. Repetition history is not evaluated.");
        layout->addWidget(evaluation);
        view=new QGraphicsView(&scene,this);
        view->viewport()->installEventFilter(this);
        view->setFixedSize(502,502); scene.setSceneRect(0,0,500,500);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        layout->addWidget(view);
        auto* controls=new QHBoxLayout(); layout->addLayout(controls);
        auto button=[&](const char* text,const char* name) {
            auto* b=new QPushButton(text,this); b->setObjectName(name); controls->addWidget(b); return b;
        };
        first=button("First","replayFirst"); previous=button("Previous","replayPrevious");
        play=button("Play","replayPlay"); next=button("Next","replayNext"); last=button("Last","replayLast");
        for(auto entry : {std::pair{first,QStyle::SP_MediaSkipBackward}, {previous,QStyle::SP_MediaSeekBackward},
                          {next,QStyle::SP_MediaSeekForward}, {last,QStyle::SP_MediaSkipForward}}) {
            entry.first->setToolTip(entry.first->text()); entry.first->setAccessibleName(entry.first->text());
            entry.first->setIcon(style()->standardIcon(entry.second)); entry.first->setText("");
        }
        suggestion=new QLabel(this); suggestion->setObjectName("replaySuggestion");
        suggestion->setToolTip("Best move found by the built-in engine at Hard difficulty; not a guarantee of perfect play.");
        suggestion->setWordWrap(true); layout->addWidget(suggestion);
        auto* help=new QLabel("Click a piece, then a legal square to try an alternative move.",this);
        layout->addWidget(help);
        restore=new QPushButton("Return to recorded game",this); restore->setObjectName("replayRestore"); layout->addWidget(restore);
        connect(restore,&QPushButton::clicked,this,[this]{
            pause(); const int target=branchRoot.value_or(index);
            this->positions=originalPositions; this->moves=originalMoves; branchRoot.reset(); go(target);
        });
        connect(first,&QPushButton::clicked,this,[this]{ pause(); go(0); });
        connect(previous,&QPushButton::clicked,this,[this]{ pause(); go(index-1); });
        connect(next,&QPushButton::clicked,this,[this]{ pause(); go(index+1,true); });
        connect(last,&QPushButton::clicked,this,[this]{ pause(); go(int(this->moves.size())); });
        connect(play,&QPushButton::clicked,this,[this]{
            if(timer.isActive()) { pause(); return; }
            if(index==int(this->moves.size())) go(0);
            timer.start(900); play->setText("Pause"); go(index+1,true);
        });
        connect(&timer,&QTimer::timeout,this,[this]{ go(index+1,true); });
        animation.setDuration(moveDurationMs); animation.setStartValue(0.0); animation.setEndValue(1.0);
        animation.setEasingCurve(QEasingCurve::InOutCubic);
        connect(&animation,&QVariantAnimation::valueChanged,this,[this](const QVariant& value){ paint(value.toDouble()); });
        connect(this,&QDialog::finished,this,[this]{ pause(); animation.stop(); if(searchCancelled) searchCancelled->store(true); ++searchGeneration; });
        go(0);
    }
    ~GameReplay() override { if(searchCancelled) searchCancelled->store(true); }
};

int choosePromotionPiece(QWidget* parent) {
    QDialog dialog(parent);
    dialog.setObjectName("promotionDialog");
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
    topCaptures->setAlignment(Qt::AlignLeft);
    bottomCaptures->setAlignment(Qt::AlignLeft);
    topPlayer = new QLabel(this);
    bottomPlayer = new QLabel(this);
    topPlayer->setFixedHeight(30);
    bottomPlayer->setFixedHeight(30);
    topCaptures->addWidget(topPlayer);
    bottomCaptures->addWidget(bottomPlayer);
    for (auto* row : {topCaptures,bottomCaptures}) {
        row->setSpacing(8);
        auto* divider = new QFrame(this);
        divider->setFrameShape(QFrame::VLine);
        divider->setFrameShadow(QFrame::Plain);
        divider->setFixedHeight(20);
        row->addWidget(divider,0,Qt::AlignVCenter);
    }
    ui.boardLayout->addLayout(topCaptures);
    ui.boardLayout->addWidget(&view, 0, Qt::AlignHCenter);
    ui.boardLayout->addLayout(bottomCaptures);
    status = new QLabel(this);
    status->setObjectName("gameStatus");
    ui.statusBar->addWidget(status,1);
    replayButton = new QPushButton("Replay game",this);
    replayButton->setObjectName("replayGame");
    ui.statusBar->addPermanentWidget(replayButton);
    connect(replayButton,&QPushButton::clicked,this,&ChessApplication::showReplay);
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
    if (resultDialog) resultDialog->close();
    if (replayDialog) replayDialog->close();
    resultShown = false;
    if (cancelled) cancelled->store(true);
    ++generation;
    stopMoveAnimation();
    thinking = false;
    game = position;
    history = {game};
    moveHistory.clear();
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
    pieceItems.clear(); overlays.clear();
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
    updateCaptureDisplay();
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
    replayButton->setVisible(gameOver && !moveHistory.empty());
    if (gameOver && !resultShown) {
        resultShown = true;
        const unsigned current = generation;
        QTimer::singleShot(0,this,[this,current,result] {
            if (current == generation && gameOver) showGameResult(result);
        });
    }
}

void ChessApplication::showGameResult(chess::Outcome result) {
    auto* dialog = new QDialog(this);
    resultDialog = dialog;
    dialog->setObjectName("gameResultDialog");
    dialog->setWindowTitle("Game over");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMinimumWidth(340);
    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(28,24,28,24);
    layout->setSpacing(16);
    QString title = "Draw";
    if (result == chess::Outcome::Checkmate) {
        title = againstComputer ? (game.whiteToMove == humanWhite ? "Computer wins" : "You win!")
                                : (game.whiteToMove ? "Black wins" : "White wins");
    }
    auto* heading = new QLabel(title,dialog);
    heading->setObjectName("gameResultTitle");
    heading->setAlignment(Qt::AlignCenter);
    auto font = heading->font();
    font.setPointSize(24);
    font.setBold(true);
    heading->setFont(font);
    layout->addWidget(heading);
    auto* detail = new QLabel(status->text(),dialog);
    detail->setObjectName("gameResultDetail");
    detail->setAlignment(Qt::AlignCenter);
    detail->setWordWrap(true);
    layout->addWidget(detail);
    auto* buttons = new QHBoxLayout();
    auto* review = new QPushButton("View board",dialog);
    review->setObjectName("viewFinalBoard");
    auto* again = new QPushButton("Play again",dialog);
    again->setObjectName("playAgain");
    again->setDefault(true);
    auto* replay = new QPushButton("Replay game",dialog);
    replay->setObjectName("replayResult");
    replay->setEnabled(!moveHistory.empty());
    buttons->addWidget(replay);
    connect(replay,&QPushButton::clicked,this,[this,dialog] { dialog->accept(); showReplay(); });
    buttons->addWidget(review);
    buttons->addWidget(again);
    layout->addLayout(buttons);
    connect(review,&QPushButton::clicked,dialog,&QDialog::accept);
    connect(again,&QPushButton::clicked,this,[this] { newGame(); });
    dialog->open();
}

void ChessApplication::showReplay() {
    if (!gameOver || moveHistory.empty()) return;
    if (replayDialog) { replayDialog->raise(); return; }
    replayDialog = new GameReplay(this,history,moveHistory,humanWhite);
    replayDialog->open();
}

void ChessApplication::updateCaptureDisplay() {
    // Count material still on the board so promotions are included as well.
    const int values[] = {0,1,3,3,5,9,0};
    int balance = 0;
    for (const auto& row : game.squares) for (int piece : row)
        if (piece) balance += (chess::isWhite(piece) ? 1 : -1)*values[chess::kind(piece)];
    if (displayedBalance == balance && displayedCaptures == capturedPieces && displayedHumanWhite == humanWhite) return;
    displayedBalance = balance;
    displayedCaptures = capturedPieces;
    displayedHumanWhite = humanWhite;
    for (auto* widget : captureLabels) delete widget;
    captureLabels.clear();
    std::map<int,int> captures;
    for (int piece : capturedPieces) ++captures[piece];
    for (const auto& [piece,count] : captures) {
        auto* group = new QWidget(this);
        group->setObjectName(QString("capturedPiece%1").arg(piece));
        auto* layout = new QHBoxLayout(group);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        if (count > 1) {
            auto* number = new QLabel(QString("%1x").arg(count),group);
            number->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
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
    history.push_back(game);
    moveHistory.push_back(move);
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
