#pragma once
#include <iostream>
#include <vector>
#include <tuple>
#include "ChessPiece.h"

typedef std::vector<std::vector<ChessPiece*>> ChessPieceMatrix;

class Board {
private:
	Board();
public:
	static ChessPieceMatrix board;
	static bool isCheckOnWhite;
	static bool isCheckOnBlack;
	static bool whiteToMove;
	
	static Board& getInstance();
	static void printBoard();
	static std::tuple<int, int> getKingPosition(bool isWhite);
	static bool isCheck(bool isWhite);
	static bool isCheckmate();

	// Delete copy constructor and assignment operator so that we can't copy the singleton
	Board(Board const&) = delete;
	void operator=(Board const&) = delete;
};

