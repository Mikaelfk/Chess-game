#include <cmath>
#include "ChessKing.h"
#include "Board.h"

ChessKing::ChessKing(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 6;
	} else {
		this->pieceType = 12;
	}
}

bool ChessKing::isMoveLegal(int x, int y) {
	// Out of bounds checking
	if (x < 0 || x > 7 || y < 0 || y > 7) {
		return false;
	}

	// Check if friendly piece is on x, y position
	if (isWhite) {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType <= 6) {
			return false;
		}
	} else {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType >= 7) {
			return false;
		}
	}

	// Check if the move is valid:
	if (std::abs(x - this->position_x) > 1 || abs(y - this->position_y) > 1) {
		return false;
	}

	// Check if any opposing pieces have line of sight to the x, y position.
	if (this->isWhite) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (Board::board[i][j]->pieceType >= 7 && Board::board[i][j]->isMoveLegal(x, y)) {
					return false;
				}
			}
		}
	} else {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (Board::board[i][j]->pieceType >= 1 && Board::board[i][j]->pieceType <= 6 && Board::board[i][j]->isMoveLegal(x, y)) {
					return false;
				}
			}
		}

	}
	return true;
}

std::vector<std::pair<int, int>> ChessKing::getLegalMoves() {
	// Get all legal king moves
	std::vector<std::pair<int, int>> legalMoves;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (i == 0 && j == 0) {
				continue;
			}
			if (canMoveBePerformed(this->position_x + i, this->position_y + j)) {
				legalMoves.push_back(std::make_pair(this->position_x + i, this->position_y + j));
			}
		}
	}
	return legalMoves;
}
