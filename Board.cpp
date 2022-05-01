#include <iostream>
#include <tuple>
#include "Board.h"
#include "ChessPawn.h"
#include "ChessKnight.h"
#include "ChessBishop.h"
#include "ChessRook.h"
#include "ChessQueen.h"
#include "ChessKing.h"	


std::vector<std::vector<std::unique_ptr<ChessPiece>>> Board::board(8);

Board::Board() {

	for (int i = 0; i < 8; i++) {
		//Create empyy arrays for each row with size of 8
		board.at(i) = std::vector<std::unique_ptr<ChessPiece>>(8);
	}
	
	// Initialize with empty pieces
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = std::make_unique<ChessPiece>();
		}
	}

	// Create all pieces in board
	for (int i = 0; i < 8; i++) {
		board[1][i] = std::make_unique<ChessPawn>(1, i, 0);
		board[6][i] = std::make_unique<ChessPawn>(6, i, 1);
	}

	// Should probably refactor this later :)
	board[0][0] = std::make_unique<ChessRook>(0, 0, 0);
	board[0][1] = std::make_unique<ChessKnight>(0, 1, 0);
	board[0][2] = std::make_unique<ChessBishop>(0, 2, 0);
	board[0][3] = std::make_unique<ChessQueen>(0, 3, 0);
	board[0][4] = std::make_unique<ChessKing>(0, 4, 0);
	board[0][5] = std::make_unique<ChessBishop>(0, 5, 0);
	board[0][6] = std::make_unique<ChessKnight>(0, 6, 0);
	board[0][7] = std::make_unique<ChessRook>(0, 7, 0);

	board[7][0] = std::make_unique<ChessRook>(7, 0, 1);
	board[7][1] = std::make_unique<ChessKnight>(7, 1, 1);
	board[7][2] = std::make_unique<ChessBishop>(7, 2, 1);
	board[7][3] = std::make_unique<ChessQueen>(7, 3, 1);
	board[7][4] = std::make_unique<ChessKing>(7, 4, 1);
	board[7][5] = std::make_unique<ChessBishop>(7, 5, 1);
	board[7][6] = std::make_unique<ChessKnight>(7, 6, 1);
	board[7][7] = std::make_unique<ChessRook>(7, 7, 1);
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
						Board::isCheckmate();
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
					Board::isCheckmate();
					return true;
				}
			}
		}
	}
	// Black king is not in check
	return false;
}

bool Board::isCheckmate() {
	std::cout << "Checkmate" << std::endl;
	return true;
}

