#include "../ChessBackend/Board.h"

int main() {
	Board::getInstance();
	if(Board::board.size() != 8) {
		std::cout << "Board size is not 64" << std::endl;
		return 1;
	}
	for (int i = 0; i < 8; i++) {
		if (Board::board[i].size() != 8) {
			std::cout << "Board size is not 64" << std::endl;
			return 1;
		}
	}
	return 0;
}