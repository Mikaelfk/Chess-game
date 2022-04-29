#include "ChessKing.h"
#include <cmath>

ChessKing::ChessKing(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 6;
	} else {
		this->pieceType = 12;
	}
}

bool ChessKing::isMoveLegal(int& x, int& y) {
	// Check if friendly piece is on x, y position
	if (isWhite) {
		if (Board::board[x][y] != 0 && Board::board[x][y] <= 6) {
			return false;
		}
	} else {
		if (Board::board[x][y] != 0 && Board::board[x][y] >= 7) {
			return false;
		}
	}
	
	// Check if the move is valid
	if (std::abs(x - this->position_x) > 1 || abs(y - this->position_y) > 1) {
		return false;
	}

	return true;
}
