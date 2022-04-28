#include <iostream>
#include "Board.h"
#include "ChessPawn.h"

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
	Board board;
	ChessPawn pawn(6, 0, true);
	ChessPawn pawn2(5, 1, false);
	Board::printBoard();
	pawn.move(pawn.position_x, pawn.position_y, 5, 1);
	std::cout << pawn.position_x << " " << pawn.position_y << std::endl;
	Board::printBoard();
}
