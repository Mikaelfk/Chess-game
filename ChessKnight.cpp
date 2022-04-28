#include "ChessKnight.h"

ChessKnight::ChessKnight(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 2;
	} else {
		this->pieceType = 8;
	}
	Board::board[x][y] = this->pieceType;
}

// Check if the move is valid
bool ChessKnight::isMoveLegal(int& x, int& y) {
	// Function for checking if a move is legal for a knight
	// I hate this so much :)
	if (isWhite && (Board::board[x][y] == 0 || Board::board[x][y] >= 7)) {
		if (this->position_x == x - 2 && this->position_y == y - 1) {
			return true;
		} else if (this->position_x == x - 2 && this->position_y == y + 1) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y - 2) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y + 2) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y - 2) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y + 2) {
			return true;
		} else if (this->position_x == x + 2 && this->position_y == y - 1) {
			return true;
		} else if (this->position_x == x + 2 && this->position_y == y + 1) {
			return true;
		} else {
			return false;
		}
	} else if (!isWhite && (Board::board[x][y] == 0 || Board::board[x][y] <= 6)) {
		if (this->position_x == x - 2 && this->position_y == y - 1) {
			return true;
		} else if (this->position_x == x - 2 && this->position_y == y + 1) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y - 2) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y + 2) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y - 2) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y + 2) {
			return true;
		} else if (this->position_x == x + 2 && this->position_y == y - 1) {
			return true;
		} else if (this->position_x == x + 2 && this->position_y == y + 1) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}


