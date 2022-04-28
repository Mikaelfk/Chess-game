#include <iostream>
#include "Board.h"
#include "ChessPawn.h"

int main()
{
	Board board;

	//print 2 dimensional array board.board
	ChessPawn pawn(0,0,true);
	int x = 1;
	pawn.isMoveLegal(x, x, x, x);
}
