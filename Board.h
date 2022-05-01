#pragma once
#include <iostream>
#include <vector>
#include "ChessPiece.h"

typedef std::vector<std::vector<ChessPiece>> ChessBoard;

class Board {
private:
	Board();
public:
	static Board& getInstance();
	static int board[8][8];
	static ChessBoard piecesBoard;
	static void printBoard();
	static bool isCheck(bool isWhite);
	static bool isCheckmate();

	// Delete copy constructor and assignment operator so that we can't copy the singleton
	Board(Board const&) = delete;
	void operator=(Board const&) = delete;
};

