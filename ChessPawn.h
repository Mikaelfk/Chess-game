#pragma once
#include "ChessPiece.h"
class ChessPawn : public ChessPiece {
public:
    ChessPawn(int x, int y, bool isWhite);
    bool isMoveLegal(int& x, int& y) override;
};

