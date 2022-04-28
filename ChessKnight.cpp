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
bool ChessKnight::isMoveLegal(int& x1, int& y1, int& x2, int& y2) {
	// Function for checking if a move is legal for a knight
	// I hate this so much :)
	if (isWhite && (Board::board[x2][y2] == 0 || Board::board[x2][y2] >= 7)) {
		if (x1 == x2 - 2 && y1 == y2 - 1) {
			return true;
		} else if (x1 == x2 - 2 && y1 == y2 + 1) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 - 2) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 + 2) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 - 2) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 + 2) {
			return true;
		} else if (x1 == x2 + 2 && y1 == y2 - 1) {
			return true;
		} else if (x1 == x2 + 2 && y1 == y2 + 1) {
			return true;
		} else {
			return false;
		}
	} else if (!isWhite && (Board::board[x2][y2] == 0 || Board::board[x2][y2] <= 6)) {
		if (x1 == x2 - 2 && y1 == y2 - 1) {
			return true;
		} else if (x1 == x2 - 2 && y1 == y2 + 1) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 - 2) {
			return true;
		} else if (x1 == x2 - 1 && y1 == y2 + 2) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 - 2) {
			return true;
		} else if (x1 == x2 + 1 && y1 == y2 + 2) {
			return true;
		} else if (x1 == x2 + 2 && y1 == y2 - 1) {
			return true;
		} else if (x1 == x2 + 2 && y1 == y2 + 1) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}


