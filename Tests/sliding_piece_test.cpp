#include "../ChessBackend/Position.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>

using namespace chess;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

bool has(const Position& position, const Move& move) {
    const auto moves = position.legalMoves();
    return std::find(moves.begin(), moves.end(), move) != moves.end();
}

Position rayPosition(int piece, bool white) {
    Position position;
    position.whiteToMove = white;
    // Kings stay outside the tested paths and potential rook-blockers' attacks.
    position.squares[0][0] = King + (white ? 0 : 6);
    position.squares[0][7] = King + (white ? 6 : 0);
    position.squares[4][4] = piece + (white ? 0 : 6);
    return position;
}

// Test the application's legal move generation and execution for every ray.
void testRays(int piece, bool straight, bool diagonal) {
    for (bool white : {true, false}) {
        for (auto direction : {std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1},
                               {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}) {
            const auto [dx, dy] = direction;
            const int row = 4 + 3 * dx, col = 4 + 3 * dy;
            const Move move{4, 4, row, col};
            const bool allowed = (dx == 0 || dy == 0) ? straight : diagonal;
            auto position = rayPosition(piece, white);
            require(!position.inCheck(white), "Ray test starts in check");
            require(has(position, move) == allowed, "Unexpected empty ray legality");
            auto moved = position;
            require(moved.play(move) == allowed, "Move execution disagrees with legal moves");
            if (allowed) {
                require(moved.squares[4][4] == Empty && moved.squares[row][col] == piece + (white ? 0 : 6),
                        "Sliding piece did not move to its destination");
                require(moved.whiteToMove != white, "Sliding move did not change the turn");
            }

            position.squares[row][col] = Rook + (white ? 6 : 0);
            require(has(position, move) == allowed, "Unexpected enemy capture legality");
            auto captured = position;
            require(captured.play(move) == allowed, "Capture execution disagrees with legal moves");
            if (allowed) {
                require(captured.squares[row][col] == piece + (white ? 0 : 6), "Captured piece was not replaced");
                require(captured.squares[4][4] == Empty && captured.halfmoveClock == 0,
                        "Capture did not clear its source or reset the draw counter");
            }

            position.squares[row][col] = Rook + (white ? 0 : 6);
            require(!has(position, move) && !position.play(move), "Friendly capture accepted");
            position.squares[row][col] = Empty;
            for (bool blockerWhite : {true, false}) {
                position.squares[4 + dx][4 + dy] = Rook + (blockerWhite ? 0 : 6);
                require(!position.inCheck(white), "Blocker test starts in check");
                require(!has(position, move) && !position.play(move), "Move through blocker accepted");
            }

            require(!position.play({4, 4, 4, 4}), "Stationary move accepted");
            require(!position.play({4, 4, 5, 6}), "Non-sliding geometry accepted");
            for (auto square : {std::pair{-1, 4}, {8, 4}, {4, -1}, {4, 8}}) {
                require(!position.play({4, 4, square.first, square.second}), "Off-board destination accepted");
                require(!position.play({square.first, square.second, 4, 4}), "Off-board source accepted");
            }
        }
    }
}

int main() {
    testRays(Bishop, false, true);
    testRays(Rook, true, false);
    testRays(Queen, true, true);
}
