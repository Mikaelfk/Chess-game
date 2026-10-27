#pragma once
#include <utility>
#include <vector>

class ChessPiece {
   public:
    ChessPiece();
    ChessPiece(int x, int y, bool isWhite);
    virtual ~ChessPiece() = default;

    int getPieceType();

    bool move(int x, int y, int promote = 0);

    virtual bool isMoveLegal(int x, int y) {
        return false;
    };
    virtual std::vector<std::pair<int, int>> getLegalMoves() {
        return std::vector<std::pair<int, int>>();
    };

   protected:
    int position_x;
    int position_y;
    bool isWhite;
    int pieceType;

    bool canMoveBePerformed(int x, int y);
    bool isSlidingMoveLegal(int x, int y, bool allowStraight, bool allowDiagonal) const;

   private:
    bool willMovePutFriendlyKingInCheck(int x, int y);
};
