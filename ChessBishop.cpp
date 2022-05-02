#include <cmath> 
#include "ChessBishop.h"
#include "Board.h"

ChessBishop::ChessBishop(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
	this->pieceType = 3;
} else {
	this->pieceType = 9;
	}
}

bool ChessBishop::isMoveLegal(int& x, int& y) {
	// Check if the move is diagonal
	int xDiff = x - this->position_x;
	int yDiff = y - this->position_y;
	if (std::abs(xDiff) != std::abs(yDiff)) {
		return false;
	}	

	// Check if friendly piece in place
	if (isWhite) {
		if (Board::board[x][y]->pieceType <= 6 && Board::board[x][y]->pieceType > 0) {
			return false;
		}
	} else {
		if (Board::board[x][y]->pieceType >= 7 && Board::board[x][y]->pieceType <= 12) {
			return false;
		}
	}

	// Check if a piece is in the way
	if (xDiff > 0) {
		if (yDiff > 0) {
			for (int i = 1; i < xDiff; i++) {
				if (Board::board[this->position_x + i][this->position_y + i]->pieceType != 0) {
					return false;
				}
			}
		} else {
			for (int i = 1; i < xDiff; i++) {
				if (Board::board[this->position_x + i][this->position_y - i]->pieceType != 0) {
					return false;
				}
			}
		}
	} else {
		if (yDiff > 0) {
			for (int i = 1; i < -xDiff; i++) {
				if (Board::board[this->position_x - i][this->position_y + i]->pieceType != 0) {
					return false;
				}
			}
		} else {
			for (int i = 1; i < -xDiff; i++) {
				if (Board::board[this->position_x - i][this->position_y - i]->pieceType != 0) {
					return false;
				}
			}
		}
	}

	return true;
}
