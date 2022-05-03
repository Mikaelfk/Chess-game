#pragma once
#include <vector>

class ChessPiece {
public:
	ChessPiece();
	ChessPiece(int x, int y, bool isWhite);
	int position_x;
	int position_y;
	int pieceType;
	bool isWhite;
	void move(int x, int y);
	virtual bool isMoveLegal(int x, int y) {
		return false;
	};
	virtual std::vector<std::pair<int, int>> getLegalMoves() {
		return std::vector<std::pair<int, int>>();
	};
};

