#pragma once
#include <vector>

class ChessPiece {
public:
	ChessPiece();
	ChessPiece(int x, int y, bool isWhite);

	int pieceType;

	void move(int x, int y, int promote = 0);

	virtual bool isMoveLegal(int x, int y) {
		return false;
	};
	virtual std::vector<std::pair<int, int>> getLegalMoves() {
		return std::vector<std::pair<int, int>>();
	};

protected:
	int position_x;
	int position_y;
	bool isWhite;

	bool canMoveBePerformed(int x, int y);

private:
	bool willMovePutFriendlyKingInCheck(int x, int y);
};

