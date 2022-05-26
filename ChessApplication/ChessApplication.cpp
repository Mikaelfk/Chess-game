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


}

void ChessApplication::updateBoard() {
	
}

void ChessApplication::on_pushButton_clicked() {

    // Get position of mouse and convert to board position
    QPoint _Position = ui.pushButton->mapFromGlobal(QCursor::pos());
    int row = _Position.y() / 75;
    int column = _Position.x() / 75;

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
    if (legalMoves.size() > 0){
        pieceChosen = true;
        activePiecePosition = std::make_pair(row, column);
    } else {
        pieceChosen = false;
    }

    // clear the console
    system("clear");
    Board::printBoard();
}
