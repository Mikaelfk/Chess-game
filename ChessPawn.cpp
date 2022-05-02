#include "ChessPawn.h"
#include "Board.h"

ChessPawn::ChessPawn(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
	if (isWhite) {
		this->pieceType = 1;
	} else {
		this->pieceType = 7;
	}
}

bool ChessPawn::isMoveLegal(int& x, int& y) {
	// Check if pawn move is legal
	if (isWhite) {
		if (this->position_x == x + 1 && this->position_y == y && Board::board[x][y]->pieceType == 0) {
			return true;
		} else if (this->position_x == x + 2 && this->position_y == y && this->position_x == 6 && Board::board[x][y]->pieceType == 0) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y + 1 && Board::board[x][y]->pieceType >= 7) {
			return true;
		} else if (this->position_x == x + 1 && this->position_y == y - 1 && Board::board[x][y]->pieceType >= 7) {
			return true;
		} else {
			return false;
		}
	} else {
		if (this->position_x == x - 1 && this->position_y == y && Board::board[x][y]->pieceType == 0) {
			return true;
		} else if (this->position_x == x - 2 && this->position_y == y && this->position_x == 1 && Board::board[x][y]->pieceType == 0) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y + 1 && Board::board[x][y]->pieceType > 0 && Board::board[x][y]->pieceType <= 6) {
			return true;
		} else if (this->position_x == x - 1 && this->position_y == y - 1 && Board::board[x][y]->pieceType > 0 && Board::board[x][y]->pieceType <= 6) {
			return true;
		} else {
			return false;
		}
	}
}
