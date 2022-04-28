#include "Board.h"
#include <iostream>

//create constructor for board class

Board::Board() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = 0;
		}
	}
}

void Board::printBoard() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			std::cout << board[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

