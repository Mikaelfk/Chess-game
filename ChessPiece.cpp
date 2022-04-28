#include "ChessPiece.h"
#include "Board.h"

ChessPiece::ChessPiece(int x, int y, bool isWhite)
{
	this->position_x = x;
	this->position_y = y;
	this->isWhite = isWhite;
	this->pieceType = 0;
}

void ChessPiece::move(int x1, int y1, int x2, int y2) {
	if (!isMoveLegal(x1, y1, x2, y2)) {
		return;
	}
	Board::board[position_x][position_y] = 0;
	this->position_x = x2;
	this->position_y = y2;
	Board::board[position_x][position_y] = this->pieceType;
}