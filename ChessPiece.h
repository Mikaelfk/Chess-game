#pragma once
#include "Board.h"

class ChessPiece {
public:
	ChessPiece(int x, int y, bool isWhite);
	int position_x;
	int position_y;
	int pieceType;
	bool isWhite;
	void move(int x1, int y1, int x2, int y2);
	virtual bool isMoveLegal(int& x1, int& y1, int& x2, int& y2) {
		return false;
	};
};

