#include "ChessApplication.h"

#include <QDebug>
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
	connect(pushButton, &QPushButton::released, this, &ChessApplication::on_pushButton_clicked);

	// Show the board
	updateBoard();

	view.setMaximumSize(500, 500);
	view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


	// Add the board widget to the layout
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




	ui.boardLayout->addWidget(playerInfo1);
	ui.boardLayout->addWidget(&view);
	ui.boardLayout->addWidget(playerInfo2);


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
				int* pieceType = &Board::board[i][j]->pieceType;
				if (*pieceType == 1) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Pawn.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 2) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Knight.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 3) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Bishop.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 4) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Rook.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 5) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White Queen.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 6) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/White King.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 7) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Pawn.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 8) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Knight.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 9) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Bishop.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 10) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Rook.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 11) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black Queen.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				} else if (*pieceType == 12) {
					board[i][j] = new QGraphicsPixmapItem(QPixmap(":/Images/Black King.png"));
					scene.addItem(board[i][j]);
					board[i][j]->setPos(j * 62.5, i * 62.5);
				}
			}
		}
	}
}

void ChessApplication::on_pushButton_clicked() {
	// Get position of mouse and convert to board position
	QPoint _Position = pushButton->mapFromGlobal(QCursor::pos());
	int row = _Position.y() / 62.5;
	int column = _Position.x() / 62.5;

	// Saves the current move in a pair of ints
	std::pair<int, int> move = std::make_pair(row, column);

	// Checks if a piece is chosen and if the pair is legal
	if (pieceChosen && (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end())) {
		int takenPieceType = Board::board[row][column]->pieceType;
		Board::board[activePiecePosition.first][activePiecePosition.second]->move(row, column);
		if (takenPieceType != 0) {
			// Add the taken piece to the taken piece widget
			showTakenPiece(takenPieceType);
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
		if ((Board::board[activePiecePosition.first][activePiecePosition.second]->pieceType >= 7 && (!Board::whiteToMove)) || (Board::board[activePiecePosition.first][activePiecePosition.second]->pieceType <= 6 && Board::whiteToMove)) {
			for (auto move : legalMoves) {
				moveHints.push_back(new QGraphicsEllipseItem(move.second * 62.5 + 21, move.first * 62.5 + 21, 20, 20));
				moveHints.back()->setBrush(Qt::gray);
				moveHints.back()->setOpacity(0.3);
				moveHints.back()->setZValue(1);
				scene.addItem(moveHints.back());
			}
		}
	} else {
		pieceChosen = false;
	}

	updateBoard();

	// add warnings if any of the kings are in check
	scene.removeItem(kingInCheckWarning);
	delete kingInCheckWarning;
	kingInCheckWarning = nullptr;

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

void ChessApplication::showTakenPiece(int &pieceType) {

	QLabel *label = new QLabel();
	if (pieceType == 1) {
		label->setPixmap(QPixmap(":/Images/White Pawn.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 2) {
		label->setPixmap(QPixmap(":/Images/White Knight.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 3) {
		label->setPixmap(QPixmap(":/Images/White Bishop.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 4) {
		label->setPixmap(QPixmap(":/Images/White Rook.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 5) {
		label->setPixmap(QPixmap(":/Images/White Queen.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 6) {
		label->setPixmap(QPixmap(":/Images/White King.png"));
		playerInfoLayout2->addWidget(label);
	} else if (pieceType == 7) {
		label->setPixmap(QPixmap(":/Images/Black Pawn.png"));
		playerInfoLayout1->addWidget(label);
	} else if (pieceType == 8) {
		label->setPixmap(QPixmap(":/Images/Black Knight.png"));
		playerInfoLayout1->addWidget(label);
	} else if (pieceType == 9) {
		label->setPixmap(QPixmap(":/Images/Black Bishop.png"));
		playerInfoLayout1->addWidget(label);
	} else if (pieceType == 10) {
		label->setPixmap(QPixmap(":/Images/Black Rook.png"));
		playerInfoLayout1->addWidget(label);
	} else if (pieceType == 11) {
		label->setPixmap(QPixmap(":/Images/Black Queen.png"));
		playerInfoLayout1->addWidget(label);
	} else if (pieceType == 12) {
		label->setPixmap(QPixmap(":/Images/Black King.png"));
		playerInfoLayout1->addWidget(label);
	}
	label->setMaximumSize(30, 30);
	label->setScaledContents(true);
	label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}
