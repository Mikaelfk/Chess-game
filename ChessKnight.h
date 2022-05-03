#pragma once
#include "ChessPiece.h"
class ChessKnight : public ChessPiece {
public: 
    ChessKnight(int x, int y, bool isWhite);
    bool isMoveLegal(int x, int y) override;
	std::vector<std::pair<int, int>> getLegalMoves() override;
};

