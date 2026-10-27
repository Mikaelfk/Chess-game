#include "../ChessBackend/Board.h"
#include "../ChessBackend/Pieces/ChessBishop.h"
#include "../ChessBackend/Pieces/ChessQueen.h"
#include "../ChessBackend/Pieces/ChessRook.h"

#include <cstdlib>
#include <type_traits>

static_assert(std::has_virtual_destructor_v<ChessPiece>);

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

void clearBoard() {
    for (auto& row : Board::board) {
        for (auto*& piece : row) {
            delete piece;
            piece = new ChessPiece();
        }
    }
}

// Check every ray in both colours, including captures and intervening pieces.
template <typename Piece>
void testRays(bool straight, bool diagonal) {
    for (bool white : {true, false}) {
        for (auto direction : {std::pair{1, 0}, {-1, 0}, {0, 1}, {0, -1},
                               {1, 1}, {1, -1}, {-1, 1}, {-1, -1}}) {
            clearBoard();
            auto* piece = new Piece(4, 4, white);
            delete Board::board[4][4];
            Board::board[4][4] = piece;
            const auto [dx, dy] = direction;
            const int x = 4 + 3 * dx, y = 4 + 3 * dy;
            const bool allowed = (dx == 0 || dy == 0) ? straight : diagonal;
            require(piece->isMoveLegal(x, y) == allowed, "Unexpected empty ray legality");
            delete Board::board[x][y];
            Board::board[x][y] = new ChessRook(x, y, !white);
            require(piece->isMoveLegal(x, y) == allowed, "Unexpected capture legality");
            delete Board::board[x][y];
            Board::board[x][y] = new ChessRook(x, y, white);
            require(!piece->isMoveLegal(x, y), "Friendly capture accepted");
            delete Board::board[x][y];
            Board::board[x][y] = new ChessPiece();
            for (bool blockerWhite : {true, false}) {
                delete Board::board[4 + dx][4 + dy];
                Board::board[4 + dx][4 + dy] = new ChessRook(4 + dx, 4 + dy, blockerWhite);
                require(!piece->isMoveLegal(x, y), "Move through blocker accepted");
            }
            require(!piece->isMoveLegal(4, 4), "Stationary move accepted");
            require(!piece->isMoveLegal(5, 6), "Non-sliding geometry accepted");
            for (auto square : {std::pair{-1, 4}, {8, 4}, {4, -1}, {4, 8}}) {
                require(!piece->isMoveLegal(square.first, square.second), "Off-board move accepted");
            }
        }
    }
}

int main() {
    Board::getInstance();
    testRays<ChessBishop>(false, true);
    testRays<ChessRook>(true, false);
    testRays<ChessQueen>(true, true);
    clearBoard();
    for (auto& row : Board::board) for (auto* piece : row) delete piece;
}
