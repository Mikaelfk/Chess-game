#pragma once
#include "ChessPiece.h"

class ChessKing : public ChessPiece {
public:
	ChessKing(int x, int y, bool isWhite);
    bool isMoveLegal(int x, int y) override;
	std::vector<std::pair<int, int>> getLegalMoves() override;
};

