#pragma once
#include "ChessPiece.h"
class ChessKnight : public ChessPiece {
public: 
    ChessKnight(int x, int y, bool isWhite);
    bool isMoveLegal(int& x1, int& y1, int& x2, int& y2) override;
};

