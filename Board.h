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
	static bool isCheckMate;
	static bool whiteToMove;
	static bool canEnPassant;
	
	static Board& getInstance();
	static void printBoard();
	static std::tuple<int, int> getKingPosition(bool isWhite);
	static bool isCheck(bool isWhite);
	static void isCheckmate(bool isWhite);
	static void isStalemate(bool isWhite);
	static void canEnPassantCheck();

	// Delete copy constructor and assignment operator so that we can't copy the singleton
	Board(Board const&) = delete;
	void operator=(Board const&) = delete;
};

