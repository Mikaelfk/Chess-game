#pragma once
#include <iostream>
#include <tuple>
#include <vector>

#include "Pieces/ChessPiece.h"

typedef std::vector<std::vector<ChessPiece*>> ChessPieceMatrix;

class Board {
   private:
    Board();

   public:
    ChessPieceMatrix board = ChessPieceMatrix(8);
    static std::vector<int> takenPieces;

    static bool isCheckOnWhite;
    static bool isCheckOnBlack;
    static bool isCheckmate;
    static bool isStalemate;
    static bool whiteToMove;
    static bool canEnPassant;
    static bool enPassantHappened;
    static bool canWhiteCastleKingSide;
    static bool canWhiteCastleQueenSide;
    static bool canBlackCastleKingSide;
    static bool canBlackCastleQueenSide;

    static bool whiteCastledKingSide;
    static bool whiteCastledQueenSide;
    static bool blackCastledKingSide;
    static bool blackCastledQueenSide;

    static int lastTakenPieceType;

    static Board& getInstance();
    void printBoard();
    std::tuple<int, int> getKingPosition(bool isWhite);
    bool isCheck(bool isWhite);
    void isCheckmateFunc(bool isWhite);

    // Delete copy constructor and assignment operator so that we can't copy the singleton
    Board(Board const&) = delete;
    void operator=(Board const&) = delete;
};
