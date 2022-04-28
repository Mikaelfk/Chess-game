#pragma once
#include "ChessPiece.h"
class ChessPawn : public ChessPiece {
public:
    ChessPawn(int x, int y, bool isWhite);
    bool hasMoved = false;
    bool isMoveLegal(int& x1, int& y1, int& x2, int& y2) override;
};

