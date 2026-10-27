#include "ChessRook.h"

#include "../Board.h"

ChessRook::ChessRook(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
    if (isWhite) {
        this->pieceType = 4;
    } else {
        this->pieceType = 10;
    }
}

bool ChessRook::isMoveLegal(int x, int y) {
    return isSlidingMoveLegal(x, y, true, false);
}

std::vector<std::pair<int, int>> ChessRook::getLegalMoves() {
    // Get all legal rook moves
    std::vector<std::pair<int, int>> legalMoves;
    for (int i = 0; i < 8; i++) {
        if (canMoveBePerformed(this->position_x, i)) {
            legalMoves.push_back(std::make_pair(this->position_x, i));
        }
        if (canMoveBePerformed(i, this->position_y)) {
            legalMoves.push_back(std::make_pair(i, this->position_y));
        }
    }
    return legalMoves;
}
