#pragma once
#include "ChessPiece.h"
class ChessRook : public ChessPiece {
public:
	ChessRook(int x, int y, bool isWhite);
    bool isMoveLegal(int& x, int& y) override;
};

