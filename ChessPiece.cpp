#include "ChessPiece.h"
#include "Board.h"


ChessPiece::ChessPiece() {
	this->position_x = 0;
	this->position_y = 0;
	this->isWhite = true;
	this->pieceType = 0;

}

ChessPiece::ChessPiece(int x, int y, bool isWhite) {
	this->position_x = x;
	this->position_y = y;
	this->isWhite = isWhite;
	this->pieceType = 0;
}

void ChessPiece::move(int x, int y) {
	if (Board::whiteToMove != isWhite || Board::isCheckmate) {
		return;
	}
	if (!isMoveLegal(x, y)) {
		return;
	}

	// Moves the piece to the new position
	ChessPiece* temp = Board::board[x][y];

	Board::board[x][y] = this;
	Board::board[position_x][position_y] = new ChessPiece();
	// Check if the move puts the friendly king in check
	if (Board::isCheck(this->isWhite)) {
		// undo move
		delete Board::board[this->position_x][this->position_y];
		Board::board[this->position_x][this->position_y] = this;
		Board::board[x][y] = temp;
		std::cout << "Move is not legal" << std::endl;
		return;
	}

	// Change the position of the piece and delete the old one
	delete temp;
	this->position_x = x;
	this->position_y = y;

	// Check if the move puts the enemy king in check
	Board::isCheck(!this->isWhite);

	// Check if the move puts the enemy king in checkmate
	Board::isCheckmateFunc(!this->isWhite);

	// Change who's turn it is
	Board::whiteToMove = !Board::whiteToMove;
}

bool ChessPiece::willMovePutFriendlyKingInCheck(int x, int y) {
	bool check = false;

	// Saves the piece that is currently in the new position
	ChessPiece* temp = Board::board[x][y];

	// Moves the piece to the new position
	Board::board[x][y] = this;

	// Check if the move puts the friendly king in check
	if (Board::isCheck(this->isWhite)) {
		check = true;
	}
	// Undo move
	Board::board[this->position_x][this->position_y] = this;
	Board::board[x][y] = temp;

	return check;
}

bool ChessPiece::canMoveBePerformed(int x, int y) {
	// Out of bound check
	if (x < 0 || x > 7 || y < 0 || y > 7) {
		return false;
	}
	if (ChessPiece::willMovePutFriendlyKingInCheck(x, y)) {
		return false;
	}
	if(!this->isMoveLegal(x, y)) {
		return false;
	}
	return true;
}