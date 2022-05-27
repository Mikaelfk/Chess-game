#include "../ChessBackend/Board.h"

int main() {
	Board::getInstance();
	if(Board::board.size() != 8) {
		std::cout << "Board does not have 8 rows" << std::endl;
		return 1;
	}
	for (int i = 0; i < 8; i++) {
		if (Board::board[i].size() != 8) {
			std::cout << "Board does not have 8 columns" << std::endl;
			return 1;
		}
	}
	return 0;
}