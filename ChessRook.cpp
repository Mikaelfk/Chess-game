#include "ChessRook.h"

ChessRook::ChessRook(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 4;
	} else {
		this->pieceType = 10;
	}
	Board::board[x][y] = this->pieceType;
}

bool ChessRook::isMoveLegal(int& x, int& y) {
	// Check if friendly piece is in the position
	if (isWhite) {
		if (Board::board[x][y] <= 6 && Board::board[x][y] >= 1) {
			return false;
		}
	} else {
		if (Board::board[x][y] >= 7) {
			return false;
		}
	}
	// Check if a piece is in the way
	if (x == this->position_x) {
		if (y > this->position_y) {
			for (int i = this->position_y + 1; i < y; i++) {
				if (Board::board[x][i] != 0) {
					return false;
				}
			}
		} else {
			for (int i = this->position_y - 1; i > y; i--) {
				if (Board::board[x][i] != 0) {
					return false;
				}
			}
		}
	} else if (y == this->position_y) {
		if (x > this->position_x) {
			for (int i = this->position_x + 1; i < x; i++) {
				if (Board::board[i][y] != 0) {
					return false;
				}
			}
		} else {
			for (int i = this->position_x - 1; i > x; i--) {
				if (Board::board[i][y] != 0) {
					return false;
				}
			}
		}
	} else {
		return false;
	}
	return true;
}

