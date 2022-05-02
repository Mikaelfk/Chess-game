#pragma once
#include "ChessPiece.h"
class ChessQueen : public ChessPiece {
public:
	ChessQueen(int x, int y, bool isWhite);
    bool isMoveLegal(int x, int y) override;
};

