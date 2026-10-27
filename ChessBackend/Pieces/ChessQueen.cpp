#include "ChessQueen.h"

#include "../Board.h"

ChessQueen::ChessQueen(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
    if (isWhite) {
        this->pieceType = 5;
    } else {
        this->pieceType = 11;
    }
}

bool ChessQueen::isMoveLegal(int x, int y) {
    return isSlidingMoveLegal(x, y, true, true);
}

std::vector<std::pair<int, int>> ChessQueen::getLegalMoves() {
    // Get all legal queen moves
    std::vector<std::pair<int, int>> legalMoves;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (canMoveBePerformed(i, j)) {
                legalMoves.push_back(std::make_pair(i, j));
            }
        }
    }
    return legalMoves;
}
