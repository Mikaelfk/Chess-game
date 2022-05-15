#include <iostream>
#include <vector>
#include "Board.h"

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
    Board::getInstance();
	/*
	Board::board[6][4]->move(4, 4);
	Board::board[1][4]->move(3, 4);

	Board::board[7][5]->move(4, 2);
	Board::board[1][3]->move(3, 3);

	Board::board[4][2]->move(3, 1);
	Board::board[0][4]->move(1, 4);

	std::vector<std::pair<int, int>> legalmoves = Board::board[3][3]->getLegalMoves();
	// print legal moves
	for (int i = 0; i < legalmoves.size(); i++)
	{
		std::cout << legalmoves[i].first << " " << legalmoves[i].second << std::endl;
	}
	*/
	/*
	// White checkmated in 2 moves
	Board::board[6][5]->move(5, 5);
	Board::board[1][4]->move(3, 4);

	Board::board[6][6]->move(4, 6);
	Board::board[0][3]->move(4, 7);
	*/
	/*
	// Checkmate black in 3 moves
	Board::board[6][4]->move(4, 4);
	Board::board[1][5]->move(3, 5);

	Board::board[6][5]->move(4, 5);
	Board::board[1][6]->move(3, 6);

	Board::board[7][3]->move(3, 7);
	*/

	/*
	// En passant test
	Board::board[6][4]->move(4, 4);
	Board::board[1][3]->move(3, 3);

	Board::board[4][4]->move(3, 4);
	Board::board[1][5]->move(3, 5);

	Board::board[3][4]->move(2, 5);
	*/

	// Castling test
	Board::board[6][4]->move(4, 4);
	Board::board[1][4]->move(3, 4);
	
	Board::board[7][5]->move(4, 2);
	Board::board[0][5]->move(3, 2);
	
	Board::board[7][6]->move(5, 5);
	Board::board[0][6]->move(2, 5);

	Board::board[7][4]->move(7, 6);
	Board::board[0][4]->move(0, 6);
	Board::printBoard();
}
