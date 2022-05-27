#include "../ChessBackend/Board.h"

int main() {
	Board::getInstance();

	// Fastest possible stalemate
	Board::board[6][4]->move(5, 4);
	Board::board[1][0]->move(3, 0);

	Board::board[7][3]->move(3, 7);
	Board::board[0][0]->move(2, 0);

	Board::board[3][7]->move(3, 0);
	Board::board[1][7]->move(3, 7);
	
	Board::board[3][0]->move(1, 2);
	Board::board[2][0]->move(2, 7);

	Board::board[6][7]->move(4, 7);
	Board::board[1][5]->move(2, 5);

	Board::board[1][2]->move(1, 3);
	Board::board[0][4]->move(1, 5);

	Board::board[1][3]->move(1, 1);
	Board::board[0][3]->move(5, 3);
	
	Board::board[1][1]->move(0, 1);
	Board::board[5][3]->move(1, 7);

	Board::board[0][1]->move(0, 2);
	Board::board[1][5]->move(2, 6);
	
	Board::board[0][2]->move(2, 4);

	Board::isCheckmateFunc(false);

	if (!Board::isStalemate) {
		std::cout << "Stalemate variable not set correctly" << std::endl;
		return 1;
	}
}