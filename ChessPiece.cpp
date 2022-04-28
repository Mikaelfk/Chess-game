#include "ChessPiece.h"

ChessPiece::ChessPiece(int x, int y, bool isWhite)
{
	this->position_x = x;
	this->position_y = y;
	this->isWhite = isWhite;
}

void ChessPiece::move(int& x1, int& y1, int& x2, int& y2) {
	if (!isMoveLegal(x1, y1, x2, y2)) {
		return;
	}
	position_x = x2;
	position_y = y2;
}