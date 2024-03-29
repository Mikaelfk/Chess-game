#include "ChessApplication.h"

#include <QDebug>
#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QVBoxLayout>
#include <algorithm>

#include "../ChessBackend/Board.h"

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
    QGraphicsProxyWidget* proxy = scene.addWidget(pushButton);

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
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != nullptr) {
                scene.removeItem(board[i][j]);
                delete board[i][j];
                board[i][j] = nullptr;
            }
        }
    }
    // Create the chess pieces
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (Board::board[i][j] != nullptr) {
                int pieceType = Board::board[i][j]->getPieceType();
                if (pieceType == 1) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Pawn.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 2) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Knight.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 3) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Bishop.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 4) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Rook.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 5) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Queen.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 6) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White King.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 7) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Pawn.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 8) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Knight.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 9) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Bishop.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 10) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Rook.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 11) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Queen.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                } else if (pieceType == 12) {
                    board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black King.png"));
                    scene.addItem(board[i][j]);
                    board[i][j]->setPos(j * 62.5, i * 62.5);
                }
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

    // Checks if a piece is chosen and if the pair is legal
    if (pieceChosen && (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end())) {
        // Check if the move is a pawn promotion
        if ((Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() == 1 || Board::board[activePiecePosition.first][activePiecePosition.second]->getPieceType() == 7) && (row == 0 || row == 7)) {
            // Create a dialog box to choose the piece to promote to
            QDialog* dialog = new QDialog();
            dialog->setWindowTitle("Pawn Promotion");
            QVBoxLayout* layout = new QVBoxLayout();
            dialog->setLayout(layout);
            QPushButton* queen = new QPushButton("Queen");
            QPushButton* rook = new QPushButton("Rook");
            QPushButton* bishop = new QPushButton("Bishop");
            QPushButton* knight = new QPushButton("Knight");
            layout->addWidget(queen);
            layout->addWidget(rook);
            layout->addWidget(bishop);
            layout->addWidget(knight);
            dialog->show();
            connect(queen, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(rook, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(bishop, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(knight, SIGNAL(clicked()), dialog, SLOT(accept()));

            connect(queen, SIGNAL(clicked()), this, SLOT(on_queenClicked()));
            connect(rook, SIGNAL(clicked()), this, SLOT(on_rookClicked()));
            connect(bishop, SIGNAL(clicked()), this, SLOT(on_bishopClicked()));
            connect(knight, SIGNAL(clicked()), this, SLOT(on_knightClicked()));
            dialog->exec();
        }
        bool moveWasPerformed = Board::board[activePiecePosition.first][activePiecePosition.second]->move(row, column, promotionPieceType);
        promotionPieceType = 0;
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

void ChessApplication::addTakenPiece(int& pieceType) {
    QLabel* label = new QLabel();
    if (pieceType == 1) {
        label->setPixmap(QPixmap(":/Images/White Pawn.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 2) {
        label->setPixmap(QPixmap(":/Images/White Knight.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 3) {
        label->setPixmap(QPixmap(":/Images/White Bishop.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 4) {
        label->setPixmap(QPixmap(":/Images/White Rook.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 5) {
        label->setPixmap(QPixmap(":/Images/White Queen.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 6) {
        label->setPixmap(QPixmap(":/Images/White King.png"));
        takenWhitePieces.push_back(label);
    } else if (pieceType == 7) {
        label->setPixmap(QPixmap(":/Images/Black Pawn.png"));
        takenBlackPieces.push_back(label);
    } else if (pieceType == 8) {
        label->setPixmap(QPixmap(":/Images/Black Knight.png"));
        takenBlackPieces.push_back(label);
    } else if (pieceType == 9) {
        label->setPixmap(QPixmap(":/Images/Black Bishop.png"));
        takenBlackPieces.push_back(label);
    } else if (pieceType == 10) {
        label->setPixmap(QPixmap(":/Images/Black Rook.png"));
        takenBlackPieces.push_back(label);
    } else if (pieceType == 11) {
        label->setPixmap(QPixmap(":/Images/Black Queen.png"));
        takenBlackPieces.push_back(label);
    } else if (pieceType == 12) {
        label->setPixmap(QPixmap(":/Images/Black King.png"));
        takenBlackPieces.push_back(label);
    }
    label->setMaximumSize(30, 30);
    label->setScaledContents(true);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    if (pieceType >= 7) {
        playerInfoLayout1->addWidget(label);
    } else {
        playerInfoLayout2->addWidget(label);
    }
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

void ChessApplication::on_queenClicked() {
    qDebug() << "Queen clicked: ";
    promotionPieceType = 0;
}

void ChessApplication::on_rookClicked() {
    promotionPieceType = 1;
}

void ChessApplication::on_bishopClicked() {
    promotionPieceType = 2;
}

void ChessApplication::on_knightClicked() {
    promotionPieceType = 3;
}
