#include "Game.h"
#include "third_party/json.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace frontend {
Game::Game(std::string savePath) : path(std::move(savePath)) {
    if (!path.empty()) {
        std::ifstream input(path);
        if (input && !restore(std::string(std::istreambuf_iterator<char>(input), {})))
            saveError = "Saved game could not be loaded. Starting a new game.";
    }
    changed();
}
Game::~Game() { invalidate(); }
const chess::Position& Game::position() const { return replay ? replayHistory[step] : live; }
void Game::invalidate() {
    ++epoch;
    for (auto& job : jobs) job.cancel->store(true);
    aiPending = aiStarted = false;
}
bool Game::gameOver() const {
    if (live.outcome() != chess::Outcome::Playing) return true;
    const auto key = live.repetitionKey();
    return std::count_if(history.begin(), history.end(), [&](const auto& p) { return p.repetitionKey() == key; }) >= 3;
}
bool Game::inputAllowed() const {
    return active && !animation && !pending && (replay || (!gameOver() && !aiPending && (!computer || live.whiteToMove == white)));
}
int Game::material() const {
    const int values[] = {0,1,3,3,5,9,0}; int balance = 0;
    for (const auto& row : position().squares) for (int p : row)
        if (p) balance += (chess::isWhite(p) ? 1 : -1) * values[chess::kind(p)];
    return balance;
}
std::string Game::status() const {
    const auto& p = position();
    switch (p.outcome()) {
    case chess::Outcome::Checkmate: return p.whiteToMove ? "Black wins - checkmate" : "White wins - checkmate";
    case chess::Outcome::Stalemate: return "Draw - stalemate";
    case chess::Outcome::FiftyMoveDraw: return "Draw - fifty-move rule";
    case chess::Outcome::InsufficientMaterial: return "Draw - insufficient material";
    default: break;
    }
    if (!replay && gameOver()) return "Draw - threefold repetition";
    if (pending) return "Choose a promotion piece";
    if (aiPending) return "Computer is thinking...";
    return std::string(p.whiteToMove ? "White to move" : "Black to move") + (p.inCheck(p.whiteToMove) ? " - Check!" : "");
}
void Game::newGame(bool humanWhite, bool againstComputer, int level) {
    if (level < 0 || level > 2) return;
    invalidate(); white = humanWhite; computer = againstComputer; difficulty = level;
    live = chess::Position::initial(); history = {live}; moves.clear();
    replay = playing = false; branchRoot.reset(); pending.reset(); animation.reset(); selected = -1;
    replayHistory.clear(); replayMoves.clear(); step = 0; saveError.clear(); save(); changed();
}
bool Game::legalTarget(int square) const {
    if (selected < 0 || square < 0 || square >= 64) return false;
    for (const auto& m : position().legalMoves())
        if (m.fromRow*8+m.fromCol == selected && m.toRow*8+m.toCol == square) return true;
    return false;
}
void Game::tryMove(int square, bool animate) {
    for (auto m : position().legalMoves()) {
        if (m.fromRow*8+m.fromCol != selected || m.toRow*8+m.toCol != square) continue;
        if (chess::kind(position().squares[m.fromRow][m.fromCol]) == chess::Pawn && (m.toRow == 0 || m.toRow == 7)) {
            pending = m; pendingAnimate = animate;
        } else apply(m, animate);
        return;
    }
}
void Game::select(int square) {
    if (!inputAllowed() || square < 0 || square >= 64) return;
    playing = false;
    if (legalTarget(square)) { tryMove(square, true); return; }
    const int p = position().squares[square/8][square%8];
    selected = p && chess::isWhite(p) == position().whiteToMove ? square : -1;
}
bool Game::beginDrag(int square) {
    if (!inputAllowed() || square < 0 || square >= 64) return false;
    const int p = position().squares[square/8][square%8];
    if (!p || chess::isWhite(p) != position().whiteToMove) return false;
    selected = square; playing = false; return true;
}
void Game::drop(int square) { if (inputAllowed() && legalTarget(square)) tryMove(square, false); }
void Game::cancelSelection() { selected = -1; }
void Game::promote(int kind) {
    if (!pending || (kind != chess::Queen && kind != chess::Rook && kind != chess::Bishop && kind != chess::Knight)) return;
    auto m = *pending; m.promotion = kind; pending.reset(); apply(m, pendingAnimate);
}
void Game::cancelPromotion() { pending.reset(); selected = -1; }
void Game::apply(chess::Move move, bool animate) {
    const auto before = position(); auto after = before;
    if (!after.play(move)) return;
    if (replay) {
        if (!branchRoot) branchRoot = step;
        replayHistory.resize(step+1); replayMoves.resize(step);
        replayHistory.push_back(after); replayMoves.push_back(move); ++step;
    } else { live = after; history.push_back(live); moves.push_back(move); save(); }
    selected = -1;
    if (animate) animation = Animation{before, move}; else animation.reset();
    changed();
}
void Game::changed() {
    invalidate();
    if (!active) return;
    if (replay) {
        const int balance = material(); evaluation = std::clamp(0.5 + balance*0.025,0.0,1.0);
        evaluationText = balance == 0 ? "Equal material" : std::string(balance > 0 ? "White +" : "Black +") + std::to_string(std::abs(balance)) + " (material)";
        suggestion = "Finding a move...";
        if (position().outcome() != chess::Outcome::Playing) {
            const bool mate = position().outcome() == chess::Outcome::Checkmate;
            evaluation = mate ? (position().whiteToMove ? 0.0 : 1.0) : 0.5;
            evaluationText = status(); suggestion = "No move - game over";
        } else launch(true);
    } else if (computer && !gameOver() && live.whiteToMove != white) { aiPending = true; aiDelay = 0.5; }
}
void Game::launch(bool analysis) {
    auto token = std::make_shared<std::atomic_bool>(false); const auto snapshot = position();
    const auto level = analysis ? chess::Difficulty::Hard : static_cast<chess::Difficulty>(difficulty);
    jobs.push_back(Job{epoch, analysis, token, std::async(std::launch::async, [snapshot, token, level] {
        return chess::chooseMove(snapshot, level, *token, std::random_device{}());
    })});
}
void Game::tick(double seconds) {
    if (!active) return;
    seconds = std::clamp(seconds, 0.0, 0.25);
    if (animation) { animation->elapsed += seconds; if (animation->elapsed >= 0.22) animation.reset(); }
    if (aiPending && !aiStarted && (aiDelay -= seconds) <= 0) { aiStarted = true; launch(false); }
    for (std::size_t i = 0; i < jobs.size();) {
        if (jobs[i].future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) { ++i; continue; }
        auto job = std::move(jobs[i]); jobs.erase(jobs.begin()+i); const auto result = job.future.get();
        if (job.cancel->load() || job.epoch != epoch) continue;
        if (!job.analysis) { aiPending = false; if (result.move) apply(*result.move, true); }
        else {
            if (result.whiteEvaluation) {
                const int score = *result.whiteEvaluation; evaluation = 0.5 + 0.5*std::tanh(score/600.0);
                std::ostringstream text;
                if (std::abs(score) > 90000) text << (score > 0 ? "White" : "Black") << " - forced mate";
                else text << (score >= 0 ? "White +" : "Black +") << std::fixed << std::setprecision(2) << std::abs(score)/100.0 << " (engine)";
                evaluationText = text.str();
            }
            if (result.move) {
                const auto m = *result.move;
                suggestion = std::string("Suggested: ")+char('a'+m.fromCol)+char('8'-m.fromRow)+" -> "+char('a'+m.toCol)+char('8'-m.toRow);
                if (m.promotion != chess::Queen || ((m.toRow == 0 || m.toRow == 7) && chess::kind(position().squares[m.fromRow][m.fromCol]) == chess::Pawn))
                    suggestion += " (promotion " + std::to_string(m.promotion) + ")";
            }
        }
    }
    if (replay && playing && !animation && (replayDelay -= seconds) <= 0) {
        if (step < replayLength()) { seek(step+1,true); replayDelay = 0.9; } else playing = false;
    }
}
void Game::setActive(bool value) { if (active == value) return; active = value; animation.reset(); save(); changed(); }
void Game::startReplay() {
    if (!gameOver() || moves.empty()) return;
    replay = true; replayHistory = history; replayMoves = moves; branchRoot.reset(); playing = false; seek(0);
}
void Game::seek(int index, bool animate) {
    if (!replay) return;
    const int target = std::clamp(index,0,replayLength());
    animation.reset(); pending.reset(); selected = -1;
    if (animate && target == step+1) animation = Animation{replayHistory[step],replayMoves[step]};
    step = target; if (step == replayLength()) playing = false; changed();
}
void Game::restoreRecording() {
    if (!replay) return;
    const int target = branchRoot.value_or(step); replayHistory = history; replayMoves = moves;
    branchRoot.reset(); playing = false; seek(target);
}
void Game::exitReplay() { if (!replay) return; replay = playing = false; pending.reset(); animation.reset(); selected = -1; changed(); }
std::string Game::serialize() const {
    nlohmann::json timeline = nlohmann::json::array();
    for (const auto m : moves) timeline.push_back({m.fromRow,m.fromCol,m.toRow,m.toCol,m.promotion});
    return nlohmann::json{{"version",1},{"white",white},{"computer",computer},{"difficulty",difficulty},{"moves",timeline}}.dump();
}
bool Game::restore(const std::string& data) {
    try {
        const auto j = nlohmann::json::parse(data);
        if (j.at("version") != 1 || !j.at("white").is_boolean() || !j.at("computer").is_boolean() || !j.at("difficulty").is_number_integer()
            || !j.at("moves").is_array() || j.at("moves").size() > 10000) return false;
        const int level = j.at("difficulty"); if (level < 0 || level > 2) return false;
        auto p = chess::Position::initial(); std::vector<chess::Position> positions{p}; std::vector<chess::Move> timeline;
        for (const auto& fields : j.at("moves")) {
            if (!fields.is_array() || fields.size() != 5) return false;
            for (const auto& f : fields) if (!f.is_number_integer() || f.get<double>() < -1 || f.get<double>() > 63) return false;
            const chess::Move m{fields[0],fields[1],fields[2],fields[3],fields[4]};
            if (p.outcome() != chess::Outcome::Playing || std::count_if(positions.begin(),positions.end(),[&](const auto& v){return v.repetitionKey()==p.repetitionKey();}) >= 3 || !p.play(m)) return false;
            timeline.push_back(m); positions.push_back(p);
        }
        invalidate(); live = p; history = std::move(positions); moves = std::move(timeline);
        white = j.at("white"); computer = j.at("computer"); difficulty = level;
        replay = playing = false; animation.reset(); pending.reset(); branchRoot.reset(); selected = -1; changed(); return true;
    } catch (const nlohmann::json::exception&) { return false; }
}
void Game::save() {
    if (path.empty()) return;
    try {
        const std::filesystem::path destination(path);
        if (!destination.parent_path().empty()) std::filesystem::create_directories(destination.parent_path());
        const auto temporary = path + ".tmp";
        { std::ofstream file(temporary, std::ios::binary | std::ios::trunc); file << serialize(); file.close(); if (!file) throw std::runtime_error("write"); }
#ifdef _WIN32
        if (!MoveFileExW(std::filesystem::path(temporary).c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("replace");
#else
        std::filesystem::rename(temporary, destination);
#endif
        saveError.clear();
    } catch (const std::exception&) { saveError = "Could not save game. Check available storage."; }
}
} // namespace frontend
