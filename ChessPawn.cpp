#include "ChessPawn.h"
#include "Board.h"

ChessPawn::ChessPawn(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 1;
	} else {
		this->pieceType = 7;
	}
	Board::board[x][y] = this->pieceType;
}

bool ChessPawn::isMoveLegal(int& x1, int& y1, int& x2, int& y2) {
	// Check if pawn move is legal
	if (isWhite) {
		if (x1 == x2 + 1 && y1 == y2 && Board::board[x2][y2] == 0) {
			return true;
		} else if (x1 == x2 + 2 && y1 == y2 && x1 == 6 && Board::board[x2][y2] == 0) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 + 1 && Board::board[x2][y2] >= 7) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 - 1 && Board::board[x2][y2] >= 7) {
			return true;
		} else {
			return false;
		}
	} else {
		if (x1 == x2 - 1 && y1 == y2 && Board::board[x2][y2] == 0) {
			return true;
		} else if (x1 == x2 - 2 && y1 == y2 && x1 == 1 && Board::board[x2][y2] == 0) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 + 1 && Board::board[x2][y2] > 0 && Board::board[x2][y2] <= 6) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 - 1 && Board::board[x2][y2] > 0 && Board::board[x2][y2] <= 6) {
			return true;
		} else {
			return false;
		}
	}
}
