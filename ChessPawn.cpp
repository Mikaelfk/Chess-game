#include "ChessPawn.h"
#include "Board.h"

ChessPawn::ChessPawn(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {}

bool ChessPawn::isMoveLegal(int& x1, int& y1, int& x2, int& y2)
{
	return true;
}
