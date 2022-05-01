#include "ChessPiece.h"
#include "Board.h"


ChessPiece::ChessPiece() {
	this->position_x = 0;
	this->position_y = 0;
	this->isWhite = true;
	this->pieceType = 0;

}

ChessPiece::ChessPiece(int x, int y, bool isWhite)
{
	this->position_x = x;
	this->position_y = y;
	this->isWhite = isWhite;
	this->pieceType = 0;
}

void ChessPiece::move(int x, int y) {
	if (!isMoveLegal(x, y)) {
		return;
	}
	if (Board::isCheck(this->isWhite)) {
		// Make logic for only allowing moves that will get the king out of check
		return;
	}

	// make logic for checking if the new moves puts the friendly king in check before allowing it.

	//Moves the piece to the new position
	
	// Move the unique pointer to the new position
	
	

	//Board::board[x][y] = this;
	Board::board[position_x][position_y] = std::make_unique<ChessPiece>();

	Board::isCheck(!this->isWhite);
}