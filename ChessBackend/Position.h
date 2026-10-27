#pragma once

#include <array>
#include <vector>
#include <string>

namespace chess {
// Piece codes match the existing image assets: white 1..6, black 7..12.
enum Piece { Empty, Pawn, Knight, Bishop, Rook, Queen, King };
inline int kind(int piece) { return piece == 0 ? 0 : (piece - 1) % 6 + 1; }
inline bool isWhite(int piece) { return piece >= 1 && piece <= 6; }

struct Move {
    int fromRow, fromCol, toRow, toCol;
    int promotion = Queen;
    bool operator==(const Move& other) const;
};

enum class Outcome { Playing, Checkmate, Stalemate, FiftyMoveDraw, InsufficientMaterial };

struct Position {
    std::array<std::array<int, 8>, 8> squares{};
    bool whiteToMove = true;
    // White king/queen side, then black king/queen side.
    std::array<bool, 4> castling{};
    int enPassantRow = -1, enPassantCol = -1;
    int halfmoveClock = 0;

    static Position initial();
    bool attacked(int row, int col, bool byWhite) const;
    bool inCheck(bool white) const;
    std::vector<Move> legalMoves() const;
    Position after(const Move& legalMove) const;
    bool play(const Move& move);
    Outcome outcome() const;
    std::string repetitionKey() const;
    bool insufficientMaterial() const;
};
} // namespace chess
