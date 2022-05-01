#include <iostream>
#include "Board.h"
#include "ChessPawn.h"
#include "ChessKnight.h"
#include "ChessBishop.h"
#include "ChessRook.h"
#include "ChessQueen.h"
#include "ChessKing.h"	


int Board::board[8][8] = {};
std::vector<std::vector<ChessPiece>> Board::piecesBoard(8);

Board::Board() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = 0;
		}
	}
	for(int i = 0; i < 8; i++) {
		//Create empyy arrays for each row with size of 8
		piecesBoard.at(i) = std::vector<ChessPiece>(8);
	}
	// Create all pieces in piecesBoard
	for (int i = 0; i < 8; i++) {
		piecesBoard[1][i] = ChessPawn(1, i, 0);
		piecesBoard[6][i] = ChessPawn(6, i, 1);
	}

	// Should probably refactor this later :)
	piecesBoard[0][0] = ChessRook(0, 0, 0);
	piecesBoard[0][1] = ChessKnight(0, 1, 0);
	piecesBoard[0][2] = ChessBishop(0, 2, 0);
	piecesBoard[0][3] = ChessQueen(0, 3, 0);
	piecesBoard[0][4] = ChessKing(0, 4, 0);
	piecesBoard[0][5] = ChessBishop(0, 5, 0);
	piecesBoard[0][6] = ChessKnight(0, 6, 0);
	piecesBoard[0][7] = ChessRook(0, 7, 0);

	piecesBoard[7][0] = ChessRook(7, 0, 1);
	piecesBoard[7][1] = ChessKnight(7, 1, 1);
	piecesBoard[7][2] = ChessBishop(7, 2, 1);
	piecesBoard[7][3] = ChessQueen(7, 3, 1);
	piecesBoard[7][4] = ChessKing(7, 4, 1);
	piecesBoard[7][5] = ChessBishop(7, 5, 1);
	piecesBoard[7][6] = ChessKnight(7, 6, 1);
	piecesBoard[7][7] = ChessRook(7, 7, 1);
}

Board& Board::getInstance() {
	static Board instance;
	return instance;
}

void Board::printBoard() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			std::cout << board[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

// color is true if white, false if black
bool Board::isCheck(bool color) {
	if (color) {
		std::cout << "Check on White" << std::endl;
		return true;
	}

	std::cout << "Check on Black" << std::endl;
	return true;
}

bool Board::isCheckmate() {
	std::cout << "Checkmate" << std::endl;
	return true;
}

