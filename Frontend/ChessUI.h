#pragma once
#include "Game.h"
#include <imgui.h>
#include <array>
namespace frontend {
class ChessUI {
public:
    explicit ChessUI(Game& game) : game(game) {}
    std::array<ImTextureID,13> textures{};
    ImVec2 boardOrigin{};
    float boardSize = 0;
    void draw();
    int squareAt(ImVec2 point) const;
    ImVec2 squareCenter(int square) const;
private:
    Game& game;
    int pressed = -1;
    bool dragging = false, settings = false, resultShown = false;
    ImVec2 pressPoint{};
    int sideChoice = 0, opponentChoice = 0, levelChoice = 1;
    void drawBoard();
};
}
