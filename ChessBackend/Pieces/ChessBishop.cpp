#include "ChessBishop.h"

#include <cmath>

#include "../Board.h"

ChessBishop::ChessBishop(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
    if (isWhite) {
        this->pieceType = 3;
    } else {
        this->pieceType = 9;
    }
}

bool ChessBishop::isMoveLegal(int x, int y) {
    // Out of bound check
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return false;
    }

    // Check if the move is diagonal
    int xDiff = x - this->position_x;
    int yDiff = y - this->position_y;
    if (std::abs(xDiff) != std::abs(yDiff)) {
        return false;
    }

    // Check if friendly piece in place
    if (isWhite) {
        if (Board::board[x][y]->getPieceType() <= 6 && Board::board[x][y]->getPieceType() > 0) {
            return false;
        }
    } else {
        if (Board::board[x][y]->getPieceType() >= 7 && Board::board[x][y]->getPieceType() <= 12) {
            return false;
        }
    }

    // Check if a piece is in the way
    if (xDiff > 0) {
        if (yDiff > 0) {
            for (int i = 1; i < xDiff; i++) {
                if (Board::board[this->position_x + i][this->position_y + i]->getPieceType() != 0) {
                    return false;
                }
            }
        } else {
            for (int i = 1; i < xDiff; i++) {
                if (Board::board[this->position_x + i][this->position_y - i]->getPieceType() != 0) {
                    return false;
                }
            }
        }
    } else {
        if (yDiff > 0) {
            for (int i = 1; i < -xDiff; i++) {
                if (Board::board[this->position_x - i][this->position_y + i]->getPieceType() != 0) {
                    return false;
                }
            }
        } else {
            for (int i = 1; i < -xDiff; i++) {
                if (Board::board[this->position_x - i][this->position_y - i]->getPieceType() != 0) {
                    return false;
                }
            }
        }
    }

    return true;
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
