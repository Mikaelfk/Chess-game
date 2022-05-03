#include "ChessRook.h"
#include "Board.h"

ChessRook::ChessRook(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 4;
	} else {
		this->pieceType = 10;
	}
}

bool ChessRook::isMoveLegal(int x, int y) {
	// Out of bound check
	if (x < 0 || x > 7 || y < 0 || y > 7) {
		return false;
	}

	// Check if friendly piece is in the position
	if (isWhite) {
		if (Board::board[x][y]->pieceType <= 6 && Board::board[x][y]->pieceType >= 1) {
			return false;
		}
	} else {
		if (Board::board[x][y]->pieceType >= 7) {
			return false;
		}
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
	} else {
		return false;
	}
	return true;
}

std::vector<std::pair<int, int>> ChessRook::getLegalMoves() {
	// Get all legal rook moves
	std::vector<std::pair<int, int>> legalMoves;
	for (int i = 0; i < 8; i++) {
		if (isMoveLegal(this->position_x, i)) {
			legalMoves.push_back(std::make_pair(this->position_x, i));
		}
		if (isMoveLegal(i, this->position_y)) {
			legalMoves.push_back(std::make_pair(i, this->position_y));
		}
	}
	return legalMoves;
}
