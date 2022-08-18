#include "ChessPiece.h"

#include "../Board.h"
#include "ChessBishop.h"
#include "ChessKnight.h"
#include "ChessQueen.h"
#include "ChessRook.h"

ChessPiece::ChessPiece() {
    this->position_x = 0;
    this->position_y = 0;
    this->isWhite = true;
    this->pieceType = 0;
}

ChessPiece::ChessPiece(int x, int y, bool isWhite) {
    this->position_x = x;
    this->position_y = y;
    this->isWhite = isWhite;
    this->pieceType = 0;
}

int ChessPiece::getPieceType() {
    return this->pieceType;
}

bool ChessPiece::getCanBeTakenByEnPassant() {
    return this->canBeTakenByEnPassant;
}

bool ChessPiece::move(int x, int y, int promote) {
    if (Board::whiteToMove != isWhite) {
        return false;
    }
    if (!canMoveBePerformed(x, y)) {
        return false;
    }
    int x_diff = abs(x - position_x);

    Board& boardInstance = Board::getInstance();
    // Moves the piece to the new position
    Board::lastTakenPieceType = boardInstance.board[x][y]->pieceType;
    boardInstance.board[x][y] = this;
    boardInstance.board[position_x][position_y] = new ChessPiece();

    bool enPassantMove = Board::enPassantHappened;
    bool whiteCastledKingSide = Board::whiteCastledKingSide;
    bool whiteCastledQueenSide = Board::whiteCastledQueenSide;
    bool blackCastledKingSide = Board::blackCastledKingSide;
    bool blackCastledQueenSide = Board::blackCastledQueenSide;

    // Check if the move makes castling impossible;
    if (this->pieceType == 6) {
        Board::canWhiteCastleKingSide = false;
        Board::canWhiteCastleQueenSide = false;
    } else if (this->pieceType == 12) {
        Board::canBlackCastleKingSide = false;
        Board::canBlackCastleQueenSide = false;
    } else if (this->pieceType == 4 && this->position_x == 7 && this->position_y == 7) {
        Board::canWhiteCastleKingSide = false;
    } else if (this->pieceType == 4 && this->position_x == 7 && this->position_y == 0) {
        Board::canWhiteCastleQueenSide = false;
    } else if (this->pieceType == 10 && this->position_x == 0 && this->position_y == 0) {
        Board::canBlackCastleQueenSide = false;
    } else if (this->pieceType == 10 && this->position_x == 0 && this->position_y == 7) {
        Board::canBlackCastleKingSide = false;
    }

    // Change the position of the piece
    this->position_x = x;
    this->position_y = y;
    if (enPassantMove) {
        if (isWhite) {
            delete boardInstance.board[this->position_x + 1][this->position_y];
            boardInstance.board[this->position_x + 1][this->position_y] = new ChessPiece();
            Board::lastTakenPieceType = 7;
        } else {
            delete boardInstance.board[this->position_x - 1][this->position_y];
            boardInstance.board[this->position_x - 1][this->position_y] = new ChessPiece();
            Board::lastTakenPieceType = 1;
        }
    }
    // Loop through and make sure every pawn has canBeTakenByEnPassant set to false
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (boardInstance.board[i][j]->pieceType == 1 || boardInstance.board[i][j]->pieceType == 7) {
                boardInstance.board[i][j]->canBeTakenByEnPassant = false;
            }
        }
    }

    // Check if the move was castling
    if (whiteCastledKingSide) {
        // Move rook to position 7,5
        delete boardInstance.board[7][5];
        boardInstance.board[7][5] = boardInstance.board[7][7];
        boardInstance.board[7][5]->position_x = 7;
        boardInstance.board[7][5]->position_y = 5;
        boardInstance.board[7][7] = new ChessPiece();

    } else if (whiteCastledQueenSide) {
        // Move rook to position 7,3
        delete boardInstance.board[7][3];
        boardInstance.board[7][3] = boardInstance.board[7][0];
        boardInstance.board[7][3]->position_x = 7;
        boardInstance.board[7][3]->position_y = 3;
        boardInstance.board[7][0] = new ChessPiece();
    } else if (blackCastledKingSide) {
        // Move rook to position 0,5
        delete boardInstance.board[0][5];
        boardInstance.board[0][5] = boardInstance.board[0][7];
        boardInstance.board[0][5]->position_x = 0;
        boardInstance.board[0][5]->position_y = 5;
        boardInstance.board[0][7] = new ChessPiece();
    } else if (blackCastledQueenSide) {
        // Move rook to position 0,3
        delete boardInstance.board[0][3];
        boardInstance.board[0][3] = boardInstance.board[0][0];
        boardInstance.board[0][3]->position_x = 0;
        boardInstance.board[0][3]->position_y = 3;
        boardInstance.board[0][0] = new ChessPiece();
    }

    // Check if move is promotion
    bool selfDestruct = false;
    if ((this->pieceType == 1 && this->position_x == 0) || (this->pieceType == 7 && this->position_x == 7)) {
        if (promote == 1) {
            boardInstance.board[this->position_x][this->position_y] = new ChessRook(this->position_x, this->position_y, this->isWhite);
        } else if (promote == 2) {
            boardInstance.board[this->position_x][this->position_y] = new ChessBishop(this->position_x, this->position_y, this->isWhite);
        } else if (promote == 3) {
            boardInstance.board[this->position_x][this->position_y] = new ChessKnight(this->position_x, this->position_y, this->isWhite);
        } else {
            boardInstance.board[this->position_x][this->position_y] = new ChessQueen(this->position_x, this->position_y, this->isWhite);
        }
        selfDestruct = true;
    }

    Board::isCheckOnBlack = false;
    Board::isCheckOnWhite = false;
    // Check if the move puts the enemy king in check
    if (boardInstance.isCheck(!this->isWhite)) {
        // Check if the move puts the enemy king in checkmate
        if (this->isWhite) {
            Board::isCheckOnBlack = true;
        } else {
            Board::isCheckOnWhite = true;
        }
        boardInstance.isCheckmateFunc(!this->isWhite);
    }

    // Check if the move makes en passant possible
    if ((this->pieceType == 1 && x_diff == 2) || (this->pieceType == 7 && x_diff == 2)) {
        Board::canEnPassant = true;
        this->canBeTakenByEnPassant = true;
    } else {
        Board::canEnPassant = false;
        this->canBeTakenByEnPassant = false;
    }

    // Change who's turn it is
    Board::whiteToMove = !Board::whiteToMove;

    // If the move was a capture, add the piece type to the takenPieces vector
    if (Board::lastTakenPieceType != 0) {
        Board::takenPieces.push_back(Board::lastTakenPieceType);
    }

    // If 30 pieces have been taken, it is a draw.
    if (Board::takenPieces.size() >= 30) {
        Board::isStalemate = true;
    }

    // Commit suicide
    if (selfDestruct) {
        delete this;
    }
    return true;
}

bool ChessPiece::willMovePutFriendlyKingInCheck(int x, int y) {
    Board& boardInstance = Board::getInstance();
    // Saves the piece that is currently in the new position
    ChessPiece* temp = boardInstance.board[x][y];

    // Moves the piece to the new position
    boardInstance.board[x][y] = this;
    boardInstance.board[this->position_x][this->position_y] = new ChessPiece();
    int temp_x = this->position_x;
    int temp_y = this->position_y;
    this->position_x = x;
    this->position_y = y;

    // Check if the move puts the friendly king in check
    bool check = boardInstance.isCheck(this->isWhite);

    // Undo move
    this->position_x = temp_x;
    this->position_y = temp_y;
    delete boardInstance.board[this->position_x][this->position_y];
    boardInstance.board[this->position_x][this->position_y] = this;
    boardInstance.board[x][y] = temp;

    return check;
}

bool ChessPiece::canMoveBePerformed(int x, int y) {
    // Out of bound check
    if (x < 0 || x > 7 || y < 0 || y > 7) {
        return false;
    }
    if (Board::isCheckmate || Board::isStalemate) {
        return false;
    }
    if (ChessPiece::willMovePutFriendlyKingInCheck(x, y)) {
        return false;
    }
    if (!this->isMoveLegal(x, y)) {
        return false;
    }

    return true;
}