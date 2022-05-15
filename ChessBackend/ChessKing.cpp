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
	Board::whiteCastledKingSide = false;
	Board::whiteCastledQueenSide = false;
	Board::blackCastledKingSide = false;
	Board::blackCastledQueenSide = false;


	// Out of bounds checking
	if (x < 0 || x > 7 || y < 0 || y > 7) {
		return false;
	}

	// Check if friendly piece is on the path to the x, y position
	if (isWhite) {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType <= 6) {
			return false;
		}
	} else {
		if (Board::board[x][y]->pieceType != 0 && Board::board[x][y]->pieceType >= 7) {
			return false;
		}
	}

	// Check if the move is castling
	if (isWhite) {
		if (Board::canWhiteCastleKingSide && x == 7 && y == 6 && Board::board[7][6]->pieceType == 0 && Board::board[7][5]->pieceType == 0) {
			// Check if opponent has line of sight to the 7,6 or 7,5 positions
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					if (Board::board[i][j]->pieceType >= 7 && Board::board[i][j]->isMoveLegal(7, 6)) {
						return false;
					}
					if (Board::board[i][j]->pieceType >= 7 && Board::board[i][j]->isMoveLegal(7, 5)) {
						return false;
					}
				}
			}
			Board::whiteCastledKingSide = true;
			return true;
		} else if (Board::canWhiteCastleQueenSide && x == 7 && y == 2 && Board::board[7][2]->pieceType == 0 && Board::board[7][3]->pieceType == 0 && Board::board[7][4]->pieceType == 0) {
			// Check if opponent has line of sight to the 7,1 or 7,2 or 7,3 positions
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					if (Board::board[i][j]->pieceType >= 7 && Board::board[i][j]->pieceType >= 1) {
						if (Board::board[i][j]->isMoveLegal(7, 1) || Board::board[i][j]->isMoveLegal(7, 2) || Board::board[i][j]->isMoveLegal(7, 3)) {
							return false;
						}
					}
				}
			}
			Board::whiteCastledQueenSide = true;
			return true;
		}
	} else {
		if (Board::canBlackCastleKingSide && x == 0 && y == 6 && Board::board[0][6]->pieceType == 0 && Board::board[0][5]->pieceType == 0) {
			// Check if opponent has line of sight to the 7,6 or 7,5 positions
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					if (Board::board[i][j]->pieceType <= 6 && Board::board[i][j]->pieceType >= 1 && Board::board[i][j]->isMoveLegal(0, 6)) {
						return false;
					}
					if (Board::board[i][j]->pieceType <= 6 && Board::board[i][j]->pieceType >= 1 && Board::board[i][j]->isMoveLegal(0, 5)) {
						return false;
					}
				}
			}
			Board::blackCastledKingSide = true;
			return true;
		} else if (Board::canBlackCastleQueenSide && x == 0 && y == 2 && Board::board[0][2]->pieceType == 0 && Board::board[0][3]->pieceType == 0 && Board::board[0][4]->pieceType == 0) {
			// Check if opponent has line of sight to the 7,1 or 7,2 or 7,3 positions
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					if (Board::board[i][j]->pieceType <= 6 && Board::board[i][j]->pieceType >= 1 && Board::board[i][j]->pieceType >= 1) {
						if (Board::board[i][j]->isMoveLegal(0, 1) || Board::board[i][j]->isMoveLegal(0, 2) || Board::board[i][j]->isMoveLegal(0, 3)) {
							return false;
						}
					}
				}
			}
			Board::blackCastledQueenSide = true;
			return true;
		}
	}


	// Check if the move is valid:
	if (std::abs(x - this->position_x) > 1 || abs(y - this->position_y) > 1) {
		return false;
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
