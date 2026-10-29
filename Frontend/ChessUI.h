#pragma once
#include "Game.h"
#include <imgui.h>
#include <array>
namespace frontend {
class ChessUI {
public:
    explicit ChessUI(Game& game) : game(game) {}
    std::array<std::array<ImTextureID, 5>, 13> textures{};
    ImTextureID pieceTexture(int piece, float logicalSize) const;
    ImVec2 boardOrigin{};
    float boardSize = 0;
    std::array<float, 2> captureRowY{}; // White, then Black; useful for layout checks.
    std::array<ImVec2, 5> replayButtonCenters{};
    float replayButtonSize = 0;
    ImVec2 replayCloseCenter{}, replayRestoreCenter{}, replayEntryCenter{};
    void draw();
    int squareAt(ImVec2 point) const;
    ImVec2 squareCenter(int square) const;
private:
    Game& game;
    int pressed = -1;
    bool dragging = false, settings = false, resultShown = false;
    ImVec2 pressPoint{};
    int sideChoice = 0, opponentChoice = 0, levelChoice = 1;
    void drawBoard(float maxHeight = 0);
    void drawReplay();
    void drawCaptures(bool whitePlayer);
};
}
