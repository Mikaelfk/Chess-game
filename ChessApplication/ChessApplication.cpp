#include "ChessApplication.h"

#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QVBoxLayout>
#include <algorithm>

#include "../ChessBackend/Board.h"

namespace {
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

ChessApplication::ChessApplication(QWidget* parent)
    : QMainWindow(parent), view(&scene) {
    ui.setupUi(this);
    ui.centralWidget->setMinimumSize(600, 600);

    // Initiate the board
    Board::getInstance();

    QGraphicsPixmapItem* background = new QGraphicsPixmapItem(QPixmap(":/Images/ChessBoard.png"));
    scene.addItem(background);
    background->setPos(0, 0);
    background->setScale(0.5);
    background->setZValue(-0.1);

    // Add transparent QPushButton on top of the board
    pushButton = new QPushButton();
    // Make pushbutton transparent
    pushButton->setFlat(true);
    pushButton->setStyleSheet("QPushButton{background:transparent; background-color: rgba(255, 255, 255, 0); border: 0px;}");
    // Set size of the push button
    pushButton->setFixedSize(QSize(500, 500));
    scene.addWidget(pushButton);

    // Connect pushButton to the slot
    connect(pushButton, &QPushButton::released, this, &ChessApplication::on_pushButtonClicked);

    // Show the board
    updateBoard();

    // Set size for the chess board and remove scroll bars.
    view.setMaximumSize(500, 500);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Make player info widgets
    playerInfoLayout1 = new QHBoxLayout();
    playerInfoLayout2 = new QHBoxLayout();
    QWidget* playerInfo1 = new QWidget();
    QWidget* playerInfo2 = new QWidget();

    playerInfo1->setLayout(playerInfoLayout1);
    playerInfo2->setLayout(playerInfoLayout2);

    QLabel* player1 = new QLabel("Player 1");
    QLabel* player2 = new QLabel("Player 2");

    playerInfoLayout1->addWidget(player1);
    playerInfoLayout2->addWidget(player2);

    // Add the playerInfo and board widgets to the main layout
    ui.boardLayout->addWidget(playerInfo2);
    ui.boardLayout->addWidget(&view);
    ui.boardLayout->addWidget(playerInfo1);
}

void ChessApplication::updateBoard() {
    for (int row = 0; row < 8; ++row) {
        for (int column = 0; column < 8; ++column) {
            // Deleting a graphics item also removes it from its scene.
            delete board[row][column];
            board[row][column] = nullptr;
            auto* piece = Board::board[row][column];
            if (piece == nullptr) {
                continue;
            }
            const int type = piece->getPieceType();
            if (type != 0) {
                board[row][column] = scene.addPixmap(piecePixmap(type));
                board[row][column]->setPos(column * 62.5, row * 62.5);
            }
        }
    }
}

void ChessApplication::on_pushButtonClicked() {
    // Get position of mouse and convert to board position
    QPoint _Position = pushButton->mapFromGlobal(QCursor::pos());
    int row = _Position.y() / 62.5;
    int column = _Position.x() / 62.5;

    // Saves the current move in a pair of ints
    std::pair<int, int> move = std::make_pair(row, column);

    int promotionPieceType = 0;

    // Checks if a piece is chosen and if the pair is legal
    if (pieceChosen && (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end())) {
        // Check if the move is a pawn promotion
        if ((Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() == 1 || Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() == 7) && (row == 0 || row == 7)) {
            promotionPieceType = choosePromotionPiece(this);
        }
        bool moveWasPerformed = Board::board[activePiecePosition.first][activePiecePosition.second]->move(row, column, promotionPieceType);
        if (Board::lastTakenPieceType != 0 && moveWasPerformed) {
            // Update the taken pieces visual
            updateTakenPieces();
        }

        // Empty legalMoves
        legalMoves.clear();
    } else {
        // Saves the legal moves on the newly clicked piece.
        legalMoves = Board::board[row][column]->getLegalMoves();
    }

    // Empty moveHints vector
    for (auto hint : moveHints) {
        scene.removeItem(hint);
        delete hint;
    }
    moveHints.clear();

    // Check if the clicked square has any legal moves
    if (legalMoves.size() > 0) {
        pieceChosen = true;
        activePiecePosition = std::make_pair(row, column);
        // For each legal move, draw a semi transparent grey circle on the board
        if ((Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() >= 7 && (!Board::whiteToMove)) || (Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() <= 6 && Board::whiteToMove)) {
            for (auto move : legalMoves) {
                QGraphicsEllipseItem* hint = nullptr;
                if (Board::board[move.first][move.second]->getPieceType() != 0) {
                    hint = new QGraphicsEllipseItem(move.second * 62.5 + 1.25, move.first * 62.5 + 1.25, 60, 60);
                    hint->setBrush(Qt::NoBrush);
                    hint->setPen(QPen(Qt::black, 2));
                    hint->setOpacity(0.3);
                    hint->setZValue(1);
                } else {
                    hint = new QGraphicsEllipseItem(move.second * 62.5 + 21, move.first * 62.5 + 21, 20, 20);
                    hint->setBrush(Qt::black);
                    hint->setOpacity(0.3);
                    hint->setZValue(1);
                }
                scene.addItem(hint);
                moveHints.push_back(hint);
            }
        }
    } else {
        pieceChosen = false;
    }

    updateBoard();

    // add warnings if any of the kings are in check
    if (kingInCheckWarning != nullptr) {
        scene.removeItem(kingInCheckWarning);
        delete kingInCheckWarning;
        kingInCheckWarning = nullptr;
    }

    if (Board::isCheckOnBlack) {
        std::tuple<int, int> blackKingPosition = Board::getKingPosition(false);
        kingInCheckWarning = new QGraphicsRectItem(std::get<1>(blackKingPosition) * 62.5, std::get<0>(blackKingPosition) * 62.5, 62.5, 62.5);
    } else if (Board::isCheckOnWhite) {
        std::tuple<int, int> whiteKingPosition = Board::getKingPosition(true);
        kingInCheckWarning = new QGraphicsRectItem(std::get<1>(whiteKingPosition) * 62.5, std::get<0>(whiteKingPosition) * 62.5, 62.5, 62.5);
    } else {
        return;
    }
    kingInCheckWarning->setBrush(Qt::red);
    kingInCheckWarning->setOpacity(0.5);

    scene.addItem(kingInCheckWarning);
}

void ChessApplication::addTakenPiece(int pieceType) {
    if (pieceType < 1 || pieceType > 12) {
        return;
    }
    auto* label = new QLabel();
    label->setPixmap(piecePixmap(pieceType));
    auto& takenPieces = pieceType >= 7 ? takenBlackPieces : takenWhitePieces;
    takenPieces.push_back(label);
    label->setMaximumSize(30, 30);
    label->setScaledContents(true);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    auto* layout = pieceType >= 7 ? playerInfoLayout1 : playerInfoLayout2;
    layout->addWidget(label);
}

void ChessApplication::updateTakenPieces() {
    for (auto piece : takenWhitePieces) {
        delete piece;
    }
    takenWhitePieces.clear();
    for (auto piece : takenBlackPieces) {
        delete piece;
    }
    takenBlackPieces.clear();
    sort(Board::takenPieces.begin(), Board::takenPieces.end());
    for (auto piece : Board::takenPieces) {
        addTakenPiece(piece);
    }
}
