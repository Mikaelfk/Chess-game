#include "Board.h"
#include <iostream>

int Board::board[8][8] = {};

Board::Board() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = 0;
		}
	}
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

bool Board::isCheck() {
	std::cout << "Check" << std::endl;
	return true;
}

bool Board::isCheckmate() {
	std::cout << "Checkmate" << std::endl;
	return true;
}

