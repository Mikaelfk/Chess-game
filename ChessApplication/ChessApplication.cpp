#include <QLabel>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <algorithm>
#include "ChessApplication.h"
#include "../ChessBackend/Board.h"

ChessApplication::ChessApplication(QWidget* parent)
	: QMainWindow(parent), view(&scene) {
	// Initiate the board
	Board::getInstance();

	//or set the layout and the view in the designer if using Qt Creator

	//EDIT: add background like this first
	QGraphicsPixmapItem* background = new QGraphicsPixmapItem(QPixmap(":/Images/ChessBoard.png"));
	scene.addItem(background);
	background->setPos(0, 0); //position it to cover all the scene so at 0,0 which is the origin point
	background->setScale(0.5); //scale the image to the scene rectangle to fill it
	background->setZValue(-0.1); //to ensure it is always at the back

	// Add transparent QPushButton on top of the board
	pushButton = new QPushButton();
	// Make pushbutton transparent
	pushButton->setStyleSheet("background-color: transparent");
	// Set size of the push button
	pushButton->setFixedSize(QSize(500, 500));
	QGraphicsProxyWidget* proxy = scene.addWidget(pushButton);

	connect(pushButton, &QPushButton::released, this, &ChessApplication::on_pushButton_clicked);

	updateBoard();

	setCentralWidget(&view);
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
				char name[] = "";
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
	qDebug() << _Position.x() << _Position.y();
	int row = _Position.y() / 62.5;
	int column = _Position.x() / 62.5;

	// Saves the current move in a pair of ints
	std::pair<int, int> move = std::make_pair(row, column);

	// Checks if a piece is chosen and if the pair is legal
	if (pieceChosen && (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end())) {
		Board::board[activePiecePosition.first][activePiecePosition.second]->move(row, column);

		// Empty legalMoves
		legalMoves.clear();
	} else {
		// Saves the legal moves on the newly clicked piece.
		legalMoves = Board::board[row][column]->getLegalMoves();
	}

	// Check if the clicked square has any legal moves
	if (legalMoves.size() > 0) {
		pieceChosen = true;
		activePiecePosition = std::make_pair(row, column);
	} else {
		pieceChosen = false;
	}

	updateBoard();
}
