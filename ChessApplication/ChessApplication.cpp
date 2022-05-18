#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <algorithm>
#include "ChessApplication.h"
#include "../ChessBackend/Board.h"

ChessApplication::ChessApplication(QWidget *parent)
    : QMainWindow(parent)
{
    // Initiate the board
    Board::getInstance();

    ui.setupUi(this);


    ui.pushButton->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
}

void ChessApplication::updateBoard() {
	
}

void ChessApplication::on_pushButton_clicked() {

    // Get position of mouse and convert to board position
    QPoint _Position = ui.pushButton->mapFromGlobal(QCursor::pos());
    int row = _Position.y() / 75;
    int column = _Position.x() / 75;
    qDebug() << row << " " << column;

    // Get the legal moves for given piece
    qDebug() << legalMoves.size();
    std::pair<int, int> move = std::make_pair(row, column);

    if (pieceChosen && (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end())) {
        qDebug() << "Piece chosen";
        Board::board[activePiecePosition.first][activePiecePosition.second]->move(row, column);
    }
	
    legalMoves = Board::board[row][column]->getLegalMoves();

    if (legalMoves.size() > 0){
        pieceChosen = true;
        activePiecePosition = std::make_pair(row, column);
    } else {
        pieceChosen = false;
    }

    Board::printBoard();
}
