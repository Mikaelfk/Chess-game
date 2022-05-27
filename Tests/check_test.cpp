#include "../ChessBackend/Board.h"

int main() {
	Board::getInstance();

	// White should not be in check
	if (Board::isCheck(true) || Board::isCheckOnWhite) {
		return 1;
	}

	Board::board[6][4]->move(4, 4);
	Board::board[1][5]->move(3, 5);
	Board::board[7][3]->move(3, 7);
	
	// Black should be in check
	if(!Board::isCheck(false) && !Board::isCheckOnBlack) {
		return 1;
	}

	Board::board[1][6]->move(2, 6);
	// Black should be out of check
	if (Board::isCheck(false) || Board::isCheckOnBlack) {
		return 1;
	}
}
