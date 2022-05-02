#include "ChessQueen.h"
#include "Board.h"

ChessQueen::ChessQueen(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 5;
	} else {
		this->pieceType = 11;
	}
}

bool ChessQueen::isMoveLegal(int& x, int& y) {
	// Check if a friendly piece is on the x and y coordinates
	if (isWhite) {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType <= 6) {
			return false;
		}
	} else {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType >= 7) {
			return false;
		}
	}

	// Check that the move is a valid queen move
	if (abs(x - this->position_x) == abs(y - this->position_y)) {
		return true;
	} else {
		return false;
	}
	
	// Check if a piece is in the way
	if (x == this->position_x) {
		if (y > this->position_y) {
			for (int i = this->position_y + 1; i < y; i++) {
				if (Board::board[x][i]->pieceType != 0) {
					return false;
				}
			}
		} else {
			for (int i = this->position_y - 1; i > y; i--) {
				if (Board::board[x][i]->pieceType != 0) {
					return false;
				}
			}
		}
	} else if (y == this->position_y) {
		if (x > this->position_x) {
			for (int i = this->position_x + 1; i < x; i++) {
				if (Board::board[i][y]->pieceType != 0) {
					return false;
				}
			}
		} else {
			for (int i = this->position_x - 1; i > x; i--) {
				if (Board::board[i][y]->pieceType != 0) {
					return false;
				}
			}
		}
	} else if (x > this->position_x && y > this->position_y) {
		for (int i = this->position_x + 1, j = this->position_y + 1; i < x && j < y; i++, j++) {
			if (Board::board[i][j]->pieceType != 0) {
				return false;
			}
		}
	} else if (x > this->position_x && y < this->position_y) {
		for (int i = this->position_x + 1, j = this->position_y - 1; i < x && j > y; i++, j--) {
			if (Board::board[i][j]->pieceType != 0) {
				return false;
			}
		}
	} else if (x < this->position_x && y > this->position_y) {
		for (int i = this->position_x - 1, j = this->position_y + 1; i > x && j < y; i--, j++) {
			if (Board::board[i][j]->pieceType != 0) {
				return false;
			}
		}
	} else if (x < this->position_x && y < this->position_y) {
		for (int i = this->position_x - 1, j = this->position_y - 1; i > x && j > y; i--, j--) {
			if (Board::board[i][j]->pieceType != 0) {
				return false;
			}
		}
	}
	return true;
}

