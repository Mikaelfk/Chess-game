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
    // Out of bound check
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return false;
    }

    // Check if a friendly piece is on the x and y coordinates
    if (isWhite) {
        if (Board::board[x][y]->getPieceType() != 0 && Board::board[x][y]->getPieceType() <= 6) {
            return false;
        }
    } else {
        if (Board::board[x][y]->getPieceType() != 0 && Board::board[x][y]->getPieceType() >= 7) {
            return false;
        }
    }

    // Check if a piece is in the way of the queen
    if (x == this->position_x) {
        if (y > this->position_y) {
            for (int i = this->position_y + 1; i < y; i++) {
                if (Board::board[x][i]->getPieceType() != 0) {
                    return false;
                }
            }
        } else {
            for (int i = this->position_y - 1; i > y; i--) {
                if (Board::board[x][i]->getPieceType() != 0) {
                    return false;
                }
            }
        }
    } else if (y == this->position_y) {
        if (x > this->position_x) {
            for (int i = this->position_x + 1; i < x; i++) {
                if (Board::board[i][y]->getPieceType() != 0) {
                    return false;
                }
            }
        } else {
            for (int i = this->position_x - 1; i > x; i--) {
                if (Board::board[i][y]->getPieceType() != 0) {
                    return false;
                }
            }
        }
    } else if (abs(x - this->position_x) == abs(y - this->position_y)) {
        if (x > this->position_x) {
            if (y > this->position_y) {
                for (int i = 1; i < abs(x - this->position_x); i++) {
                    if (Board::board[this->position_x + i][this->position_y + i]->getPieceType() != 0) {
                        return false;
                    }
                }
            } else {
                for (int i = 1; i < abs(x - this->position_x); i++) {
                    if (Board::board[this->position_x + i][this->position_y - i]->getPieceType() != 0) {
                        return false;
                    }
                }
            }
        } else {
            if (y > this->position_y) {
                for (int i = 1; i < abs(x - this->position_x); i++) {
                    if (Board::board[this->position_x - i][this->position_y + i]->getPieceType() != 0) {
                        return false;
                    }
                }
            } else {
                for (int i = 1; i < abs(x - this->position_x); i++) {
                    if (Board::board[this->position_x - i][this->position_y - i]->getPieceType() != 0) {
                        return false;
                    }
                }
            }
        }
    } else {
        return false;
    }

    return true;
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
