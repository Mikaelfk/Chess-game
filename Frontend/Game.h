#pragma once
#include "ChessBackend/ChessAI.h"
#include <atomic>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace frontend {
struct Animation { chess::Position before; chess::Move move; double elapsed = 0; };
class Game {
public:
    explicit Game(std::string savePath = {});
    ~Game();
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    bool white = true, computer = true, replay = false, playing = false;
    int difficulty = 1, selected = -1, step = 0;
    std::optional<int> branchRoot;
    std::optional<Animation> animation;
    double evaluation = 0.5;
    std::string evaluationText = "Equal material", suggestion, saveError;
    const chess::Position& position() const;
    const std::vector<chess::Move>& recordedMoves() const { return moves; }
    int replayLength() const { return int(replayMoves.size()); }
    bool thinking() const { return aiPending; }
    bool promotionPending() const { return pending.has_value(); }
    bool gameOver() const;
    bool inputAllowed() const;
    bool legalTarget(int square) const;
    std::string status() const;
    int material() const;
    std::array<int, 7> capturedBy(bool whitePlayer) const;
    void newGame(bool humanWhite, bool againstComputer, int level);
    void select(int square);
    bool beginDrag(int square);
    void drop(int square);
    void cancelSelection();
    void promote(int kind);
    void cancelPromotion();
    void startReplay();
    void seek(int index, bool animate = false);
    void restoreRecording();
    void exitReplay();
    void tick(double seconds);
    void setActive(bool value);
    std::string serialize() const;
    bool restore(const std::string& data);
    void save();
private:
    chess::Position live = chess::Position::initial();
    std::vector<chess::Position> history{live}, replayHistory;
    std::vector<chess::Move> moves, replayMoves;
    std::optional<chess::Move> pending;
    bool pendingAnimate = true, aiPending = false, aiStarted = false, active = true;
    double aiDelay = 0, replayDelay = 0;
    std::string path;
    unsigned epoch = 0;
    struct Job {
        unsigned epoch; bool analysis;
        std::shared_ptr<std::atomic_bool> cancel;
        std::future<chess::SearchResult> future;
    };
    std::vector<Job> jobs;
    void invalidate();
    void changed();
    void launch(bool analysis);
    void apply(chess::Move move, bool animate);
    void tryMove(int square, bool animate);
};
} // namespace frontend
