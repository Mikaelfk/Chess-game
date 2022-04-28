#include <iostream>
#include <vector>
#include "Board.h"
#include "ChessPawn.h"
#include "ChessKnight.h"
#include "ChessBishop.h"
#include "ChessRook.h"
#include "ChessQueen.h"

/*
Each piece type is represented with a number:
0 - Empty
White: 
1 - Pawn
2 - Knight
3 - Bishop
4 - Rook
5 - Queen
6 - King

Black:
7 - Pawn
8 - Knight
9 - Bishop
10 - Rook
11 - Queen
12 - King
*/

int main()
{
	// Initialize board
    Board::getInstance;

	typedef std::vector<ChessPawn> PawnVector;
	PawnVector whitePawns;
	PawnVector blackPawns;

	for (int i = 0; i < 8; i++) {
		whitePawns.push_back(ChessPawn(6, i, true));
		blackPawns.push_back(ChessPawn(1, i, false));
	}

	ChessQueen queen(5, 4, true);
	Board::printBoard();
	queen.move(5, 5);
	std::cout << std::endl;
	Board::printBoard();
}
