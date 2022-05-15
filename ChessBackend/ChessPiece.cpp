#include "ChessPiece.h"
#include "Board.h"
#include "ChessQueen.h"
#include "ChessRook.h"
#include "ChessBishop.h"
#include "ChessKnight.h"


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

void ChessPiece::move(int x, int y, int promote) {
	if (Board::whiteToMove != isWhite || Board::isCheckmate) {
		return;
	}
	if (!canMoveBePerformed(x, y)) {
		return;
	}
	int x_diff = abs(x - position_x);

	// Moves the piece to the new position
	Board::board[x][y] = this;
	Board::board[position_x][position_y] = new ChessPiece();

	bool enPassantMove = Board::enPassantHappened;
	bool whiteCastledKingSide = Board::whiteCastledKingSide;
	bool whiteCastledQueenSide = Board::whiteCastledQueenSide;
	bool blackCastledKingSide = Board::blackCastledKingSide;
	bool blackCastledQueenSide = Board::blackCastledQueenSide;
	
	// Check if the move makes castling impossible;
	if (this->pieceType == 6) {
		Board::canWhiteCastleKingSide = false;
		Board::canWhiteCastleQueenSide = false;
	} else if (this->pieceType == 12) {
		Board::canBlackCastleKingSide = false;
		Board::canBlackCastleQueenSide = false;
	} else if (this->pieceType == 4 && this->position_x == 7 && this->position_y == 7) {
		Board::canWhiteCastleKingSide = false;
	} else if (this->pieceType == 4 && this->position_x == 7 && this->position_y == 0) {
		Board::canWhiteCastleQueenSide = false;
	} else if (this->pieceType == 10 && this->position_x == 0 && this->position_y == 0) {
		Board::canBlackCastleQueenSide = false;
	} else if (this->pieceType == 10 && this->position_x == 0 && this->position_y == 7) {
		Board::canBlackCastleKingSide = false;
	}

	// Change the position of the piece
	this->position_x = x;
	this->position_y = y;
	if (enPassantMove) {
		if (isWhite) {
			delete Board::board[this->position_x + 1][this->position_y];
			Board::board[this->position_x + 1][this->position_y] = new ChessPiece();
		} else {
			delete Board::board[this->position_x - 1][this->position_y];
			Board::board[this->position_x - 1][this->position_y] = new ChessPiece();
		}
	}

	// Check if the move was castling
	if (whiteCastledKingSide) {
		// Move rook to position 7,5
		delete Board::board[7][5];
		Board::board[7][5] = Board::board[7][7];
		Board::board[7][7] = new ChessPiece();
		
	} else if (whiteCastledQueenSide) {
		// Move rook to position 7,3
		delete Board::board[7][3];
		Board::board[7][3] = Board::board[7][0];
		Board::board[7][0] = new ChessPiece();
	} else if (blackCastledKingSide) {
		// Move rook to position 0,5
		delete Board::board[0][5];
		Board::board[0][5] = Board::board[0][7];
		Board::board[0][7] = new ChessPiece();
	} else if (blackCastledQueenSide) {
		// Move rook to position 0,3
		delete Board::board[0][3];
		Board::board[0][3] = Board::board[0][0];
		Board::board[0][0] = new ChessPiece();
	}

	// Check if move is promotion
	bool selfDestruct = false;
	if ((this->pieceType == 1 && this->position_x == 0) || (this->pieceType == 7 && this->position_x == 7)) {
		if (promote == 1) {
			Board::board[this->position_x][this->position_y] = new ChessRook(this->position_x, this->position_y, this->isWhite);
		} else if (promote == 2) {
			Board::board[this->position_x][this->position_y] = new ChessBishop(this->position_x, this->position_y, this->isWhite);
		} else if (promote == 3) {
			Board::board[this->position_x][this->position_y] = new ChessKnight(this->position_x, this->position_y, this->isWhite);
		} else {
			Board::board[this->position_x][this->position_y] = new ChessQueen(this->position_x, this->position_y, this->isWhite);
		}
		selfDestruct = true;
	} 
	// Check if the move puts the enemy king in check
	if (Board::isCheck(!this->isWhite)) {
		// Check if the move puts the enemy king in checkmate
		Board::isCheckmateFunc(!this->isWhite);
	}

	// Check if the move makes en passant possible
	if ((this->pieceType == 1 && x_diff == 2) || (this->pieceType == 7 && x_diff == 2)) {
		Board::canEnPassant = true;
	} else {
		Board::canEnPassant = false;
	}

	// Change who's turn it is
	Board::whiteToMove = !Board::whiteToMove;

	// Commit suicide
	if (selfDestruct) {
		delete this;
	}
}

bool ChessPiece::willMovePutFriendlyKingInCheck(int x, int y) {
	bool check = false;

	// Saves the piece that is currently in the new position
	ChessPiece* temp = Board::board[x][y];

	// Moves the piece to the new position
	Board::board[x][y] = this;
	Board::board[this->position_x][this->position_y] = new ChessPiece();
	int temp_x = this->position_x;
	int temp_y = this->position_y;
	this->position_x = x;
	this->position_y = y;

	// Check if the move puts the friendly king in check
	if (Board::isCheck(this->isWhite)) {
		check = true;
	}

	// Undo move
	this->position_x = temp_x;
	this->position_y = temp_y;
	delete Board::board[this->position_x][this->position_y];
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
	if (!this->isMoveLegal(x, y)) {
		return false;
	}
	return true;
}