#include "Position.h"

#include <algorithm>
#include <cmath>

namespace chess {
namespace {
bool inside(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
constexpr int directions[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};
constexpr int knightSteps[8][2] = {{1,2}, {1,-2}, {-1,2}, {-1,-2}, {2,1}, {2,-1}, {-2,1}, {-2,-1}};
}

bool Move::operator==(const Move& m) const {
    return fromRow == m.fromRow && fromCol == m.fromCol && toRow == m.toRow && toCol == m.toCol && promotion == m.promotion;
}

Position Position::initial() {
    Position p;
    const int back[] = {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook};
    for (int c = 0; c < 8; ++c) {
        p.squares[0][c] = back[c] + 6;
        p.squares[1][c] = Pawn + 6;
        p.squares[6][c] = Pawn;
        p.squares[7][c] = back[c];
    }
    p.castling.fill(true);
    return p;
}

bool Position::attacked(int r, int c, bool white) const {
    const int pawnRow = r + (white ? 1 : -1);
    for (int dc : {-1, 1}) {
        if (inside(pawnRow, c + dc) && squares[pawnRow][c + dc] == Pawn + (white ? 0 : 6)) return true;
    }
    for (const auto& step : knightSteps) {
        const int nr = r + step[0], nc = c + step[1];
        if (inside(nr, nc) && squares[nr][nc] == Knight + (white ? 0 : 6)) return true;
    }
    for (int d = 0; d < 8; ++d) {
        for (int distance = 1; distance < 8; ++distance) {
            const int nr = r + directions[d][0] * distance, nc = c + directions[d][1] * distance;
            if (!inside(nr, nc)) break;
            const int piece = squares[nr][nc];
            if (!piece) continue;
            if (isWhite(piece) == white) {
                const int type = kind(piece);
                if (type == Queen || (d < 4 && type == Rook) || (d >= 4 && type == Bishop) || (distance == 1 && type == King)) return true;
            }
            break;
        }
    }
    return false;
}

bool Position::inCheck(bool white) const {
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c) {
        if (squares[r][c] == King + (white ? 0 : 6)) return attacked(r, c, !white);
    }
    return true; // A playable position must contain both kings.
}

std::vector<Move> Position::legalMoves() const {
    std::vector<Move> candidates;
    auto add = [&](int r, int c, int nr, int nc) {
        if (!inside(nr, nc)) return;
        const int target = squares[nr][nc];
        if (target && (isWhite(target) == whiteToMove || kind(target) == King)) return;
        if (kind(squares[r][c]) == Pawn && (nr == 0 || nr == 7)) {
            for (int promotion : {Queen, Rook, Bishop, Knight}) candidates.push_back({r,c,nr,nc,promotion});
        } else candidates.push_back({r,c,nr,nc});
    };
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c) {
        const int piece = squares[r][c], type = kind(piece);
        if (!piece || isWhite(piece) != whiteToMove) continue;
        if (type == Pawn) {
            const int step = whiteToMove ? -1 : 1;
            if (inside(r + step, c) && !squares[r + step][c]) {
                add(r,c,r+step,c);
                if (r == (whiteToMove ? 6 : 1) && !squares[r + 2*step][c]) add(r,c,r+2*step,c);
            }
            for (int dc : {-1, 1}) {
                const int nr = r + step, nc = c + dc;
                if (!inside(nr,nc)) continue;
                const bool enPassant = nr == enPassantRow && nc == enPassantCol && !squares[nr][nc]
                    && squares[r][nc] == Pawn + (whiteToMove ? 6 : 0);
                if (squares[nr][nc] || enPassant) add(r,c,nr,nc);
            }
        } else if (type == Knight) {
            for (const auto& step : knightSteps) add(r,c,r+step[0],c+step[1]);
        } else {
            for (int d = 0; d < 8; ++d) {
                if ((type == Bishop && d < 4) || (type == Rook && d >= 4)) continue;
                for (int distance = 1; distance <= (type == King ? 1 : 7); ++distance) {
                    const int nr = r + directions[d][0] * distance, nc = c + directions[d][1] * distance;
                    if (!inside(nr,nc)) break;
                    add(r,c,nr,nc);
                    if (squares[nr][nc]) break;
                }
            }
            if (type == King && r == (whiteToMove ? 7 : 0) && c == 4 && !inCheck(whiteToMove)) {
                for (int side = 0; side < 2; ++side) {
                    const int rookCol = side == 0 ? 7 : 0, target = side == 0 ? 6 : 2;
                    if (!castling[(whiteToMove ? 0 : 2) + side] || squares[r][rookCol] != Rook + (whiteToMove ? 0 : 6)) continue;
                    bool clear = true;
                    for (int col = std::min(4,rookCol)+1; col < std::max(4,rookCol); ++col) clear &= squares[r][col] == Empty;
                    if (clear && !attacked(r,side == 0 ? 5 : 3,!whiteToMove) && !attacked(r,target,!whiteToMove)) add(r,c,r,target);
                }
            }
        }
    }
    std::vector<Move> legal;
    for (const auto& move : candidates) if (!after(move).inCheck(whiteToMove)) legal.push_back(move);
    return legal;
}

Position Position::after(const Move& move) const {
    Position next = *this;
    const int piece = squares[move.fromRow][move.fromCol], target = squares[move.toRow][move.toCol];
    next.halfmoveClock = kind(piece) == Pawn || target ? 0 : halfmoveClock + 1;
    next.squares[move.fromRow][move.fromCol] = Empty;
    next.squares[move.toRow][move.toCol] = piece;
    if (kind(piece) == Pawn) {
        if (move.toRow == enPassantRow && move.toCol == enPassantCol && !target && move.fromCol != move.toCol) {
            next.squares[move.fromRow][move.toCol] = Empty;
        }
        if (move.toRow == 0 || move.toRow == 7) next.squares[move.toRow][move.toCol] = move.promotion + (isWhite(piece) ? 0 : 6);
    }
    if (kind(piece) == King) {
        const int offset = isWhite(piece) ? 0 : 2;
        next.castling[offset] = next.castling[offset+1] = false;
        if (std::abs(move.toCol - move.fromCol) == 2) {
            const int from = move.toCol == 6 ? 7 : 0, to = move.toCol == 6 ? 5 : 3;
            next.squares[move.toRow][to] = next.squares[move.toRow][from];
            next.squares[move.toRow][from] = Empty;
        }
    }
    for (int side = 0; side < 4; ++side) {
        const int row = side < 2 ? 7 : 0, col = side % 2 == 0 ? 7 : 0;
        if ((move.fromRow == row && move.fromCol == col) || (move.toRow == row && move.toCol == col)) next.castling[side] = false;
    }
    next.enPassantRow = next.enPassantCol = -1;
    if (kind(piece) == Pawn && std::abs(move.toRow - move.fromRow) == 2) {
        next.enPassantRow = (move.toRow + move.fromRow) / 2;
        next.enPassantCol = move.fromCol;
    }
    next.whiteToMove = !whiteToMove;
    return next;
}

bool Position::play(const Move& move) {
    const auto legal = legalMoves();
    if (std::find(legal.begin(),legal.end(),move) == legal.end()) return false;
    *this = after(move);
    return true;
}

bool Position::insufficientMaterial() const {
    int minors = 0, bishopColour = -1;
    bool onlySameColourBishops = true;
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c) {
        const int type = kind(squares[r][c]);
        if (type == Empty || type == King) continue;
        if (type != Bishop && type != Knight) return false;
        ++minors;
        if (type == Knight) onlySameColourBishops = false;
        else if (bishopColour == -1) bishopColour = (r+c)%2;
        else if (bishopColour != (r+c)%2) onlySameColourBishops = false;
    }
    return minors <= 1 || onlySameColourBishops;
}

std::string Position::repetitionKey() const {
    std::string key;
    for (const auto& row : squares) for (int piece : row) key += char('a'+piece);
    key += whiteToMove ? 'w' : 'b';
    for (bool right : castling) key += right ? '1' : '0';
    if (enPassantRow >= 0) {
        for (const auto& move : legalMoves()) {
            if (kind(squares[move.fromRow][move.fromCol]) == Pawn && move.toRow == enPassantRow && move.toCol == enPassantCol && move.fromCol != move.toCol) {
                key += char('0'+enPassantRow);
                key += char('0'+enPassantCol);
                break;
            }
        }
    }
    return key;
}

Outcome Position::outcome() const {
    if (legalMoves().empty()) return inCheck(whiteToMove) ? Outcome::Checkmate : Outcome::Stalemate;
    if (halfmoveClock >= 100) return Outcome::FiftyMoveDraw;
    if (insufficientMaterial()) return Outcome::InsufficientMaterial;
    return Outcome::Playing;
}
} // namespace chess
