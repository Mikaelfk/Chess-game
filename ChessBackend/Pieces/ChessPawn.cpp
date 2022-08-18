#include "ChessPawn.h"

#include "../Board.h"

ChessPawn::ChessPawn(int x, int y, bool isWhite) : ChessPiece(x, y, isWhite) {
    if (isWhite) {
        this->pieceType = 1;
    } else {
        this->pieceType = 7;
    }
}

bool ChessPawn::isMoveLegal(int x, int y) {
    Board::enPassantHappened = false;
    // Out of bound check
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return false;
    }

    // Check if pawn move is legal
    if (isWhite) {
        if (this->position_x == x + 1 && this->position_y == y && Board::board[x][y]->getPieceType() == 0) {
            return true;
        } else if (this->position_x == x + 2 && this->position_y == y && this->position_x == 6 && Board::board[x][y]->getPieceType() == 0 && Board::board[x + 1][y]->getPieceType() == 0) {
            return true;
        } else if (this->position_x == x + 1 && this->position_y == y + 1 && Board::board[x][y]->getPieceType() >= 7) {
            return true;
        } else if (this->position_x == x + 1 && this->position_y == y - 1 && Board::board[x][y]->getPieceType() >= 7) {
            return true;
        } else if (this->position_x == x + 1 && (this->position_y == y + 1 || this->position_y == y - 1) && Board::board[this->position_x][y]->getPieceType() == 7 && Board::canEnPassant) {
            Board::enPassantHappened = true;
            return true;
        } else {
            return false;
        }
    } else {
        if (this->position_x == x - 1 && this->position_y == y && Board::board[x][y]->getPieceType() == 0) {
            return true;
        } else if (this->position_x == x - 2 && this->position_y == y && this->position_x == 1 && Board::board[x][y]->getPieceType() == 0 && Board::board[x - 1][y]->getPieceType() == 0) {
            return true;
        } else if (this->position_x == x - 1 && this->position_y == y + 1 && Board::board[x][y]->getPieceType() > 0 && Board::board[x][y]->getPieceType() <= 6) {
            return true;
        } else if (this->position_x == x - 1 && this->position_y == y - 1 && Board::board[x][y]->getPieceType() > 0 && Board::board[x][y]->getPieceType() <= 6) {
            return true;
        } else if (this->position_x == x - 1 && (this->position_y == y + 1 || this->position_y == y - 1) && Board::board[this->position_x][y]->getPieceType() == 1 && Board::canEnPassant) {
            Board::enPassantHappened = true;
            return true;
        } else {
            return false;
        }
    }
}

std::vector<std::pair<int, int>> ChessPawn::getLegalMoves() {
    // Get all legal pawn moves
    std::vector<std::pair<int, int>> legalMoves;
    // Check what pawn moves are legal
    if (this->canMoveBePerformed(position_x + 1, position_y)) {
        legalMoves.push_back(std::make_pair(position_x + 1, position_y));
    }
    if (this->canMoveBePerformed(position_x + 2, position_y)) {
        legalMoves.push_back(std::make_pair(position_x + 2, position_y));
    }
    if (this->canMoveBePerformed(position_x - 1, position_y)) {
        legalMoves.push_back(std::make_pair(position_x - 1, position_y));
    }
    if (this->canMoveBePerformed(position_x - 2, position_y)) {
        legalMoves.push_back(std::make_pair(position_x - 2, position_y));
    }
    if (this->canMoveBePerformed(position_x + 1, position_y + 1)) {
        legalMoves.push_back(std::make_pair(position_x + 1, position_y + 1));
    }
    if (this->canMoveBePerformed(position_x + 1, position_y - 1)) {
        legalMoves.push_back(std::make_pair(position_x + 1, position_y - 1));
    }
    if (this->canMoveBePerformed(position_x - 1, position_y + 1)) {
        legalMoves.push_back(std::make_pair(position_x - 1, position_y + 1));
    }
    if (this->canMoveBePerformed(position_x - 1, position_y - 1)) {
        legalMoves.push_back(std::make_pair(position_x - 1, position_y - 1));
    }
    return legalMoves;
}
