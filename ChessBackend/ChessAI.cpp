#include "ChessAI.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

namespace chess {
namespace {
constexpr int mate = 100000;
constexpr int values[] = {0,100,320,330,500,900,0};
int evaluate(const Position& p) {
    int score = 0;
    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c) {
        const int piece = p.squares[r][c], type = kind(piece);
        if (!piece) continue;
        const int centre = 14 - std::abs(2*r-7) - std::abs(2*c-7);
        int value = values[type];
        if (type == Knight || type == Bishop) value += centre * 3;
        if (type == Pawn) value += (isWhite(piece) ? 6-r : r-1) * 8 + centre;
        score += isWhite(piece) ? value : -value;
    }
    return p.whiteToMove ? score : -score;
}

void orderMoves(const Position& p, std::vector<Move>& moves) {
    auto priority = [&](const Move& m) {
        const int attacker = kind(p.squares[m.fromRow][m.fromCol]);
        int victim = kind(p.squares[m.toRow][m.toCol]);
        if (attacker == Pawn && m.fromCol != m.toCol && victim == Empty) victim = Pawn;
        return (victim ? 10*values[victim] - values[attacker] : 0)
            + (attacker == Pawn && (m.toRow == 0 || m.toRow == 7) ? values[m.promotion] : 0);
    };
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) { return priority(a) > priority(b); });
}

struct Search {
    const std::atomic_bool& cancelled;
    std::chrono::steady_clock::time_point deadline;
    int nodeLimit, nodes = 0;
    bool interrupted = false;

    int negamax(const Position& p, int depth, int alpha, int beta, int ply) {
        if (++nodes > nodeLimit || cancelled.load() || std::chrono::steady_clock::now() >= deadline) {
            interrupted = true;
            return 0;
        }
        auto moves = p.legalMoves();
        if (moves.empty()) return p.inCheck(p.whiteToMove) ? -mate + ply : 0;
        if (p.halfmoveClock >= 100 || p.insufficientMaterial()) return 0;
        if (depth == 0) return evaluate(p);
        orderMoves(p, moves);
        int best = -mate;
        for (const auto& move : moves) {
            const int score = -negamax(p.after(move), depth-1, -beta, -alpha, ply+1);
            if (interrupted) return 0;
            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        return best;
    }
};
}

SearchResult chooseMove(const Position& p, Difficulty difficulty, const std::atomic_bool& cancelled, std::uint32_t seed) {
    SearchResult result;
    if (cancelled.load() || p.outcome() != Outcome::Playing) return result;
    auto moves = p.legalMoves();
    std::mt19937 random(seed);
    std::shuffle(moves.begin(), moves.end(), random);
    result.move = moves.front();
    if (difficulty == Difficulty::Easy) return result;
    const bool hard = difficulty == Difficulty::Hard;
    Search search{cancelled, std::chrono::steady_clock::now() + std::chrono::milliseconds(hard ? 1500 : 350), hard ? 100000 : 15000};
    orderMoves(p, moves);
    for (int depth = 1; depth <= (hard ? 4 : 2); ++depth) {
        int best = -mate, alpha = -mate;
        Move choice = moves.front();
        for (const auto& move : moves) {
            const int score = -search.negamax(p.after(move), depth-1, -mate, -alpha, 1);
            if (search.interrupted) break;
            if (score > best) { best = score; choice = move; }
            alpha = std::max(alpha, score);
        }
        if (search.interrupted) break; // Keep the last fully searched result.
        result.move = choice;
        result.completedDepth = depth;
        result.whiteEvaluation = p.whiteToMove ? best : -best;
        const auto chosen = std::find(moves.begin(), moves.end(), choice);
        std::rotate(moves.begin(), chosen, chosen+1);
    }
    result.nodes = search.nodes;
    if (cancelled.load()) result.move.reset();
    return result;
}
} // namespace chess
