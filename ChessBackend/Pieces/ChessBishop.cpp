#include "ChessBishop.h"

#include "../Board.h"

ChessBishop::ChessBishop(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
    if (isWhite) {
        this->pieceType = 3;
    } else {
        this->pieceType = 9;
    }
}

bool ChessBishop::isMoveLegal(int x, int y) {
    return isSlidingMoveLegal(x, y, false, true);
}

std::vector<std::pair<int, int>> ChessBishop::getLegalMoves() {
    // Get all legal bishop moves
    std::vector<std::pair<int, int>> legalMoves;
    for (int i = 1; i < 8; i++) {
        if (canMoveBePerformed(this->position_x + i, this->position_y + i)) {
            legalMoves.push_back(std::make_pair(this->position_x + i, this->position_y + i));
        }
        if (canMoveBePerformed(this->position_x + i, this->position_y - i)) {
            legalMoves.push_back(std::make_pair(this->position_x + i, this->position_y - i));
        }
        if (canMoveBePerformed(this->position_x - i, this->position_y + i)) {
            legalMoves.push_back(std::make_pair(this->position_x - i, this->position_y + i));
        }
        if (canMoveBePerformed(this->position_x - i, this->position_y - i)) {
            legalMoves.push_back(std::make_pair(this->position_x - i, this->position_y - i));
        }
    }
    return legalMoves;
}
