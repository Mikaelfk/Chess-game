#pragma once
class Board {
public:
	Board();
	static int board[8][8];
	static void printBoard();
	bool isCheck();
	bool isCheckmate();
};

