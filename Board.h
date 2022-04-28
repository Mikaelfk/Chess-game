#pragma once
class Board {
private:
	Board();
public:
	static Board& getInstance();
	static int board[8][8];
	static void printBoard();
	bool isCheck();
	bool isCheckmate();

	// Delete copy constructor and assignment operator so that we can't copy the singleton
	Board(Board const&) = delete;
	void operator=(Board const&) = delete;
};

