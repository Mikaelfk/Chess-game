#pragma once
#include "ChessPiece.h"
class ChessBishop : public ChessPiece {
public:
	ChessBishop(int x, int y, bool isWhite);
	bool isMoveLegal(int x, int y) override;
	std::vector<std::pair<int, int>> getLegalMoves() override;
};

