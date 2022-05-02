#include <iostream>
#include <tuple>
#include "Board.h"
#include "ChessPawn.h"
#include "ChessKnight.h"
#include "ChessBishop.h"
#include "ChessRook.h"
#include "ChessQueen.h"
#include "ChessKing.h"	


std::vector<std::vector<ChessPiece*>> Board::board(8);
bool Board::isCheckOnWhite = false;
bool Board::isCheckOnBlack = false;
bool Board::whiteToMove = true;

Board::Board() {
	for (int i = 0; i < 8; i++) {
		//Create empyy arrays for each row with size of 8
		board.at(i) = std::vector<ChessPiece*>(8);
	}
	
	// Initialize with empty pieces
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = new ChessPiece();
		}
	}

	// Create all pieces in board
	for (int i = 0; i < 8; i++) {
		delete board[1][i];
		delete board[6][i];
		board[1][i] = new ChessPawn(1, i, 0);
		board[6][i] = new ChessPawn(6, i, 1);
	}

	for (int i = 0; i < 8; i++) {
		delete board[0][i];
		delete board[7][i];
	}
	// Should probably refactor this later :)
	board[0][0] = new ChessRook(0, 0, 0);
	board[0][1] = new ChessKnight(0, 1, 0);
	board[0][2] = new ChessBishop(0, 2, 0);
	board[0][3] = new ChessQueen(0, 3, 0);
	board[0][4] = new ChessKing(0, 4, 0);
	board[0][5] = new ChessBishop(0, 5, 0);
	board[0][6] = new ChessKnight(0, 6, 0);
	board[0][7] = new ChessRook(0, 7, 0);

	board[7][0] = new ChessRook(7, 0, 1);
	board[7][1] = new ChessKnight(7, 1, 1);
	board[7][2] = new ChessBishop(7, 2, 1);
	board[7][3] = new ChessQueen(7, 3, 1);
	board[7][4] = new ChessKing(7, 4, 1);
	board[7][5] = new ChessBishop(7, 5, 1);
	board[7][6] = new ChessKnight(7, 6, 1);
	board[7][7] = new ChessRook(7, 7, 1);
}

Board& Board::getInstance() {
	static Board instance;
	return instance;
}

void Board::printBoard() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			std::cout << board[i][j]->pieceType << " ";
		}
		std::cout << std::endl;
	}
}

std::tuple<int, int> Board::getKingPosition(bool isWhite) {
	if (isWhite) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (Board::board[i][j]->pieceType == 6) {
					return std::make_tuple(i, j);
				}
			}
		}
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (Board::board[i][j]->pieceType == 12) {
				return std::make_tuple(i, j);
			}
		}
	}
	return std::make_tuple(-1, -1);
}

// color is true if white, false if black
bool Board::isCheck(bool color) {
	if (color) {
		// Check if white king is in check
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (board[i][j]->pieceType >= 7) {
					// Check if piece can move to king
					int kingPosition_x, kingPosition_y;
					std::tie(kingPosition_x, kingPosition_y) = Board::getKingPosition(true);
					if (board[i][j]->isMoveLegal(kingPosition_x, kingPosition_y)) {
						std::cout << "Check on White" << std::endl;
						Board::isCheckmate(true);
						return true;
					}
				}
			}
		}
		// White king is not in check
		return false;
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (board[i][j]->pieceType >= 1 && board[i][j]->pieceType <= 6) {
				// Check if piece can move to king
				int kingPosition_x, kingPosition_y;
				std::tie(kingPosition_x, kingPosition_y) = Board::getKingPosition(false);
				if (board[i][j]->isMoveLegal(kingPosition_x, kingPosition_y)) {
					std::cout << "Check on Black" << std::endl;
					Board::isCheckmate(false);
					return true;
				}
			}
		}
	}
	// Black king is not in check
	return false;
}

// Check if king is in checkmate, if isWhite is true, check white king, else check black king
bool Board::isCheckmate(bool isWhite) {
	int kingPosition_x, kingPosition_y;
	std::tie(kingPosition_x, kingPosition_y) = Board::getKingPosition(isWhite);
	// Check if king can move to any square
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if(Board::board[i][j]->position_x == kingPosition_x && Board::board[i][j]->position_y == kingPosition_y) {
				if (!(Board::board[i][j]->isMoveLegal(kingPosition_x + 1, kingPosition_y + 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x + 1, kingPosition_y - 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x - 1, kingPosition_y + 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x - 1, kingPosition_y - 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x, kingPosition_y + 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x, kingPosition_y - 1) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x + 1, kingPosition_y) ||
					Board::board[i][j]->isMoveLegal(kingPosition_x - 1, kingPosition_y))) {
					std::cout << "Checkmate" << std::endl;
					return true;
				}
			}
		}
	}
	return false;
}

