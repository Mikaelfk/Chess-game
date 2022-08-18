#include "../ChessBackend/Board.h"

int main() {
    Board::getInstance();

    Board::board[6][4]->move(4, 4);
    if (Board::board[4][4]->getPieceType() != 1) {
        return 1;
    }
}