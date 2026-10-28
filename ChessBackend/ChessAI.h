#pragma once
#include "Position.h"

#include <atomic>
#include <cstdint>
#include <optional>

namespace chess {
enum class Difficulty { Easy, Medium, Hard };
struct SearchResult {
    std::optional<Move> move;
    int completedDepth = 0;
    int nodes = 0;
    // Positive favours White; 100 units is approximately one pawn.
    std::optional<int> whiteEvaluation;
};

// Works only on value copies; safe to run while the GUI displays the live game.
SearchResult chooseMove(const Position& position, Difficulty difficulty,
                        const std::atomic_bool& cancelled, std::uint32_t seed);
} // namespace chess
