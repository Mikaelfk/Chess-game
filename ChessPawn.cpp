#include "ChessPawn.h"

ChessPawn::ChessPawn(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 1;
	} else {
		this->pieceType = 7;
	}
	Board::board[x][y] = this->pieceType;
}

bool ChessPawn::isMoveLegal(int& x2, int& y2) {
	// Check if pawn move is legal
	if (isWhite) {
		if (this->position_x == x2 + 1 && this->position_y == y2 && Board::board[x2][y2] == 0) {
			return true;
		} else if (this->position_x == x2 + 2 && this->position_y == y2 && this->position_x == 6 && Board::board[x2][y2] == 0) {
			return true;
		} else if (this->position_x == x2 + 1 && this->position_y == y2 + 1 && Board::board[x2][y2] >= 7) {
			return true;
		} else if (this->position_x == x2 + 1 && this->position_y == y2 - 1 && Board::board[x2][y2] >= 7) {
			return true;
		} else {
			return false;
		}
	} else {
		if (this->position_x == x2 - 1 && this->position_y == y2 && Board::board[x2][y2] == 0) {
			return true;
		} else if (this->position_x == x2 - 2 && this->position_y == y2 && this->position_x == 1 && Board::board[x2][y2] == 0) {
			return true;
		} else if (this->position_x == x2 - 1 && this->position_y == y2 + 1 && Board::board[x2][y2] > 0 && Board::board[x2][y2] <= 6) {
			return true;
		} else if (this->position_x == x2 - 1 && this->position_y == y2 - 1 && Board::board[x2][y2] > 0 && Board::board[x2][y2] <= 6) {
			return true;
		} else {
			return false;
		}
	}
}
