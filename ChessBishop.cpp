#include <cmath> 
#include "ChessBishop.h"

ChessBishop::ChessBishop(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 3;
	} else {
		this->pieceType = 9;
	}
	Board::board[x][y] = this->pieceType;
}

bool ChessBishop::isMoveLegal(int& x, int& y) {
	// x = 3, y = 6
	// position_x = 5 , position_y = 4
	int xDiff = x - this->position_x;
	int yDiff = y - this->position_y;
	// Check if the move is diagonal
	if (std::abs(xDiff) != std::abs(yDiff)) {
		return false;
	}	

	// Check if friendly piece in place
	if (isWhite) {
		if (Board::board[x][y] <= 6 && Board::board[x][y] > 0) {
			return false;
		}
	} else {
		if (Board::board[x][y] >= 7 && Board::board[x][y] <= 12) {
			return false;
		}
	}

	// Check if a piece is in the way
	if (xDiff > 0) {
		if (yDiff > 0) {
			for (int i = 1; i < xDiff; i++) {
				if (Board::board[this->position_x + i][this->position_y + i] != 0) {
					return false;
				}
			}
		} else {
			for (int i = 1; i < xDiff; i++) {
				if (Board::board[this->position_x + i][this->position_y - i] != 0) {
					return false;
				}
			}
		}
	} else {
		if (yDiff > 0) {
			for (int i = 1; i < -xDiff; i++) {
				if (Board::board[this->position_x - i][this->position_y + i] != 0) {
					return false;
				}
			}
		} else {
			for (int i = 1; i < -xDiff; i++) {
				if (Board::board[this->position_x - i][this->position_y - i] != 0) {
					return false;
				}
			}
		}
	}

	return true;
}