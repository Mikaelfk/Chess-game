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
		return;
	}
	Board::board[position_x][position_y] = 0;
	this->position_x = x;
	this->position_y = y;
	Board::board[position_x][position_y] = this->pieceType;
	Board::isCheck(!this->isWhite);
}