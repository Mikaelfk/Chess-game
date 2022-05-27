#include "../ChessBackend/Board.h"

int main() {
	Board::getInstance();
	
	// Checkmate on white in two moves
	Board::board[6][5]->move(5, 5);
	Board::board[1][4]->move(3, 4);
	Board::board[6][6]->move(4, 6);
	Board::board[0][3]->move(4, 7);

	if (!Board::isCheckmate) {
		std::cout << "Checkmate variable has not been correctly set" << std::endl;
		return 1;
	}
}
