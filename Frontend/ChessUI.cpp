#include "ChessUI.h"
#include <algorithm>
#include <cmath>
#include <string>
namespace frontend {
namespace {
const char* names[] = {"", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
ImVec2 plus(ImVec2 a, ImVec2 b) { return {a.x+b.x,a.y+b.y}; }
enum class ReplayIcon { First, Previous, Play, Pause, Next, Last, Restore, Close };
bool iconButton(const char* id, const char* label, ReplayIcon icon, float size, ImVec2& center, bool enabled = true) {
    ImGui::BeginDisabled(!enabled);
    const bool clicked = ImGui::Button(id,{size,size});
    const auto a = ImGui::GetItemRectMin();
    center = {a.x+size*.5f,a.y+size*.5f};
    auto* draw = ImGui::GetWindowDrawList();
    const auto color = ImGui::GetColorU32(ImGuiCol_Text);
    const float radius = size*.20f, stroke = std::max(1.5f,size*.045f);
    auto triangle = [&](bool right) {
        const float direction = right ? 1.f : -1.f;
        draw->AddTriangleFilled({center.x+direction*radius,center.y},
            {center.x-direction*radius,center.y-radius}, {center.x-direction*radius,center.y+radius},color);
    };
    auto chevron = [&](bool right) {
        const float direction = right ? 1.f : -1.f;
        draw->AddLine({center.x-direction*radius*.6f,center.y-radius},
            {center.x+direction*radius*.6f,center.y},color,stroke);
        draw->AddLine({center.x+direction*radius*.6f,center.y},
            {center.x-direction*radius*.6f,center.y+radius},color,stroke);
    };
    switch(icon) {
    case ReplayIcon::First: case ReplayIcon::Last: {
        const bool right = icon == ReplayIcon::Last;
        triangle(right);
        const float x = center.x + (right?1:-1)*size*.29f;
        draw->AddLine({x,center.y-radius},{x,center.y+radius},color,stroke);
        break;
    }
    case ReplayIcon::Previous: chevron(false); break;
    case ReplayIcon::Next: chevron(true); break;
    case ReplayIcon::Play: triangle(true); break;
    case ReplayIcon::Pause:
        for (float offset : {-size*.12f,size*.12f})
            draw->AddRectFilled({center.x+offset-stroke,center.y-radius},{center.x+offset+stroke,center.y+radius},color);
        break;
    case ReplayIcon::Close:
        draw->AddLine({center.x-radius,center.y-radius},{center.x+radius,center.y+radius},color,stroke);
        draw->AddLine({center.x-radius,center.y+radius},{center.x+radius,center.y-radius},color,stroke);
        break;
    case ReplayIcon::Restore:
        draw->PathArcTo(center,radius,-1.57f,3.14f,20); draw->PathStroke(color,0,stroke);
        draw->AddTriangleFilled({center.x-radius-size*.1f,center.y-size*.04f},
            {center.x-radius+size*.1f,center.y-size*.04f},{center.x-radius,center.y+size*.12f},color);
        break;
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s",label);
    return clicked;
}

}
ImTextureID ChessUI::pieceTexture(int piece, float logicalSize) const {
    if (piece < 1 || piece > 12) return 0;
    const auto scale = ImGui::GetIO().DisplayFramebufferScale;
    const float pixels = logicalSize * std::max(scale.x, scale.y);
    int level = 0;
    while (level < 4 && (32 << level) < pixels) ++level;
    return textures[piece][level];
}
int ChessUI::squareAt(ImVec2 point) const {
    const float x = point.x-boardOrigin.x, y = point.y-boardOrigin.y;
    if (boardSize <= 0 || x < 0 || y < 0 || x >= boardSize || y >= boardSize) return -1;
    const int screen = int(y/(boardSize/8))*8+int(x/(boardSize/8));
    return game.white ? screen : 63-screen;
}
ImVec2 ChessUI::squareCenter(int square) const {
    const int screen = game.white ? square : 63-square;
    return {boardOrigin.x+(screen%8+0.5f)*boardSize/8,boardOrigin.y+(screen/8+0.5f)*boardSize/8};
}
void ChessUI::drawBoard(float maxHeight) {
    boardSize = std::max(80.0f,std::min(ImGui::GetContentRegionAvail().x,600.0f * ImGui::GetStyle().MouseCursorScale));
    if (maxHeight > 0) boardSize = std::min(boardSize, std::max(80.0f,maxHeight));
    boardOrigin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("Chessboard", {boardSize,boardSize});
    const bool hovered = ImGui::IsItemHovered();
    auto& io = ImGui::GetIO();
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || !game.inputAllowed()) { dragging = false; pressed = -1; }
    if (hovered && ImGui::IsMouseClicked(0) && game.inputAllowed()) { pressed = squareAt(io.MousePos); pressPoint = io.MousePos; }
    if (pressed >= 0 && ImGui::IsMouseDown(0) && !dragging && ImGui::IsMouseDragging(0,7)) dragging = game.beginDrag(pressed);
    if (pressed >= 0 && ImGui::IsMouseReleased(0)) {
        const int target = squareAt(io.MousePos);
        if (dragging) game.drop(target);
        else if (target == pressed) game.select(target);
        pressed = -1; dragging = false;
    }
    auto* draw = ImGui::GetWindowDrawList(); const float cell = boardSize/8;
    for (int square = 0; square < 64; ++square) {
        const auto center = squareCenter(square); const auto corner = plus(center,{-cell/2,-cell/2});
        const int screen = game.white ? square : 63-square;
        ImU32 color = ((screen/8+screen%8)%2) ? IM_COL32(121,146,122,255) : IM_COL32(228,221,200,255);
        if (game.selected == square) color = IM_COL32(228,185,89,255);
        const int p = game.position().squares[square/8][square%8];
        if (chess::kind(p) == chess::King && game.position().inCheck(chess::isWhite(p))) color = IM_COL32(215,95,85,255);
        draw->AddRectFilled(corner,plus(corner,{cell,cell}),color);
        if (screen%8 == 0) { const char label[] = {char('8'-square/8),0}; draw->AddText(plus(corner,{2,1}),IM_COL32(30,48,35,255),label); }
        if (screen/8 == 7) { const char label[] = {char('a'+square%8),0}; draw->AddText(plus(corner,{cell-10,cell-16}),IM_COL32(30,48,35,255),label); }
    }
    auto piece = [&](int p, ImVec2 center) {
        if (!p) return;
        const auto texture = pieceTexture(p, cell*.86f);
        if (texture) draw->AddImage(texture,plus(center,{-cell*.43f,-cell*.43f}),plus(center,{cell*.43f,cell*.43f}));
        else { const char label[] = {" PNBRQK"[chess::kind(p)],0}; draw->AddText(plus(center,{-5,-8}),chess::isWhite(p)?IM_COL32_WHITE:IM_COL32_BLACK,label); }
    };
    const auto& position = game.animation ? game.animation->before : game.position();
    // Draw moving pieces last so captures cannot cover them during a slide.
    for (int pass = 0; pass < 2; ++pass) for (int square = 0; square < 64; ++square) {
        const int p = position.squares[square/8][square%8];
        if (!p || (dragging && square == pressed)) continue;
        auto center = squareCenter(square);
        bool moving = false;
        if (game.animation) {
            const auto& animation = *game.animation; const auto m = animation.move;
            int target = -1;
            if (square == m.fromRow*8+m.fromCol) target = m.toRow*8+m.toCol;
            else if (chess::kind(position.squares[m.fromRow][m.fromCol]) == chess::King && std::abs(m.toCol-m.fromCol)==2
                && square == m.fromRow*8+(m.toCol>m.fromCol?7:0)) target = m.fromRow*8+(m.toCol>m.fromCol?5:3);
            moving = target >= 0;
            if (target >= 0) { const auto end = squareCenter(target); const float t = float(animation.elapsed/0.22); const float ease=t*t*(3-2*t); center.x+=(end.x-center.x)*ease; center.y+=(end.y-center.y)*ease; }
        }
        if (moving == (pass == 1)) piece(p,center);
    }
    if (game.selected >= 0 && !game.animation) for (int square=0;square<64;++square) if (game.legalTarget(square)) {
        const auto center = squareCenter(square);
        const bool capture = game.position().squares[square/8][square%8] || (chess::kind(game.position().squares[game.selected/8][game.selected%8])==chess::Pawn && square%8!=game.selected%8);
        if (capture) draw->AddCircle(center,cell*.43f,IM_COL32(161,46,46,220),32,3);
        else draw->AddCircleFilled(center,cell*.12f,IM_COL32(49,70,60,140));
    }
    if (dragging && pressed >= 0) piece(game.position().squares[pressed/8][pressed%8],io.MousePos);
}
void ChessUI::drawCaptures(bool whitePlayer) {
    const float font = ImGui::GetFontSize();
    const float icon = font * 1.4f;
    const auto origin = ImGui::GetCursorScreenPos();
    const float width = std::min(ImGui::GetContentRegionAvail().x, 600.0f * ImGui::GetStyle().MouseCursorScale);
    captureRowY[whitePlayer ? 0 : 1] = origin.y;
    // Reserve a row even when empty so a capture cannot shift the board.
    ImGui::Dummy({width, icon + 4});
    auto* draw = ImGui::GetWindowDrawList();
    const char* label = whitePlayer ? "White" : "Black";
    draw->AddText(plus(origin,{0,(icon-font)/2}),ImGui::GetColorU32(ImGuiCol_Text),label);
    float x = origin.x + ImGui::CalcTextSize(label).x + font * 0.5f;
    const auto counts = game.capturedBy(whitePlayer);
    for (int kind = chess::Pawn; kind <= chess::Queen; ++kind) {
        if (!counts[kind]) continue;
        const int piece = kind + (whitePlayer ? 6 : 0);
        const ImVec2 corner{x,origin.y};
        // A light backing keeps black captures legible on the dark app background.
        draw->AddRectFilled(corner,plus(corner,{icon,icon}),IM_COL32(228,221,200,255),3);
        const auto texture = pieceTexture(piece, icon);
        if (texture) draw->AddImage(texture,corner,plus(corner,{icon,icon}));
        else draw->AddText(corner,IM_COL32(30,48,35,255),names[kind]);
        x += icon + 2;
        if (counts[kind] > 1) {
            const auto count = std::to_string(counts[kind]) + "x";
            draw->AddText(nullptr,font*.8f,{x,origin.y+(icon-font*.8f)/2},ImGui::GetColorU32(ImGuiCol_Text),count.c_str());
            x += ImGui::CalcTextSize(count.c_str()).x*.8f + 2;
        }
        x += font*.3f;
    }
}
void ChessUI::drawReplay() {
    const float font = ImGui::GetFontSize();
    const float scale = std::max(1.0f,font/18.0f);
    const float gap = ImGui::GetStyle().ItemSpacing.x;
    const float width = ImGui::GetContentRegionAvail().x;
    const float button = std::min(44.0f*scale,(width-4*gap)/5);
    const float utility = button*.8f;
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("GAME REPLAY");
    ImGui::SameLine(width-button+ImGui::GetStyle().WindowPadding.x);
    if (iconButton("##closeReplay","Close replay",ReplayIcon::Close,button,replayCloseCenter)) {
        game.exitReplay(); resultShown = true; return;
    }
    // Only the board/analysis pane scrolls. The transport remains on screen.
    const float footer = button+utility+3*ImGui::GetStyle().ItemSpacing.y;
    const float paneHeight = std::max(1.0f,ImGui::GetContentRegionAvail().y-footer);
    ImGui::BeginChild("Replay position",{0,paneHeight},ImGuiChildFlags_None);
    auto analysisPane = [&] {
        ImGui::ProgressBar(float(game.evaluation),{-1,font+4},game.evaluationText.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("White / Black engine advantage; not a win probability");
        // Reserve two lines so a suggestion wrapping cannot move the board.
        ImGui::BeginChild("Replay suggestion",{0,font*2+ImGui::GetStyle().ItemSpacing.y},ImGuiChildFlags_None,ImGuiWindowFlags_NoScrollbar);
        ImGui::TextWrapped("%s",game.suggestion.c_str()); ImGui::EndChild();
    };
    auto positionPane = [&](float maxBoardHeight) {
        drawCaptures(!game.white);
        const float bottomRow = font*1.4f+4+2*ImGui::GetStyle().ItemSpacing.y;
        drawBoard(std::max(80.0f,maxBoardHeight > 0 ? maxBoardHeight : ImGui::GetContentRegionAvail().y-bottomRow));
        drawCaptures(game.white);
    };
    if (width > paneHeight*1.8f && width > 500*scale && ImGui::BeginTable("Replay landscape",2)) {
        ImGui::TableSetupColumn("Board",ImGuiTableColumnFlags_WidthFixed,std::max(80.0f,paneHeight));
        ImGui::TableSetupColumn("Analysis",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn(); positionPane(paneHeight-2*(font*1.4f+4+ImGui::GetStyle().ItemSpacing.y));
        ImGui::TableNextColumn(); analysisPane();
        ImGui::EndTable();
    } else { analysisPane(); positionPane(0); }
    const auto& io=ImGui::GetIO();
    if (ImGui::IsWindowHovered() && io.MouseSource == ImGuiMouseSource_TouchScreen && io.MouseDown[0] && pressed < 0 && !ImGui::IsAnyItemActive())
        ImGui::SetScrollY(ImGui::GetScrollY()-io.MouseDelta.y);
    ImGui::EndChild();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s%d / %d",game.branchRoot?"Variation: ":"Move ",game.step,game.replayLength());
    ImGui::SameLine(width-utility+ImGui::GetStyle().WindowPadding.x);
    if (iconButton("##restoreReplay","Return to recorded game",ReplayIcon::Restore,utility,replayRestoreCenter,game.branchRoot.has_value())) game.restoreRecording();
    const float toolbarWidth = button*5+gap*4;
    ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x+std::max(0.0f,(width-toolbarWidth)/2));
    if (iconButton("##replayFirst","First move",ReplayIcon::First,button,replayButtonCenters[0],game.step>0)) { game.playing=false; game.seek(0); }
    ImGui::SameLine();
    if (iconButton("##replayPrevious","Previous move",ReplayIcon::Previous,button,replayButtonCenters[1],game.step>0)) { game.playing=false; game.seek(game.step-1); }
    ImGui::SameLine();
    if (iconButton("##replayPlay",game.playing?"Pause":"Play",game.playing?ReplayIcon::Pause:ReplayIcon::Play,button,replayButtonCenters[2])) {
        if (game.step==game.replayLength()) game.seek(0);
        game.playing=!game.playing;
    }
    ImGui::SameLine();
    if (iconButton("##replayNext","Next move",ReplayIcon::Next,button,replayButtonCenters[3],game.step<game.replayLength())) { game.playing=false; game.seek(game.step+1,true); }
    ImGui::SameLine();
    if (iconButton("##replayLast","Last move",ReplayIcon::Last,button,replayButtonCenters[4],game.step<game.replayLength())) { game.playing=false; game.seek(game.replayLength()); }
}
void ChessUI::draw() {
    auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0,0}); ImGui::SetNextWindowSize(io.DisplaySize);
    if (game.replay) ImGui::SetNextWindowScroll({0,0});
    const auto replayFlags = game.replay ? ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse : 0;
    ImGui::Begin("Chess",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|replayFlags);
    if (game.replay) drawReplay();
    else {
        ImGui::TextUnformatted("CHESS"); ImGui::SameLine();
        if (ImGui::Button("New game")) { settings=true; sideChoice=game.white?0:1; opponentChoice=game.computer?0:1; levelChoice=game.difficulty; }
        ImGui::TextUnformatted(game.status().c_str());
        const int balance = game.material();
        if (balance == 0) ImGui::Text("Equal material | %s at bottom",game.white?"White":"Black");
        else ImGui::Text("%s +%d | %s at bottom",balance>0?"White":"Black",std::abs(balance),game.white?"White":"Black");
        drawCaptures(!game.white);
        drawBoard();
        drawCaptures(game.white);
        ImGui::TextWrapped("Tap a piece and destination, or drag to a highlighted square. Escape cancels a drag.");
        if (!game.replay && game.gameOver() && !game.recordedMoves().empty() && ImGui::Button("Replay game")) game.startReplay();
    }
    if (!game.saveError.empty()) ImGui::TextWrapped("%s",game.saveError.c_str());
    if (!game.replay && io.MouseSource == ImGuiMouseSource_TouchScreen && io.MouseDown[0] && pressed < 0 && !ImGui::IsAnyItemActive())
        ImGui::SetScrollY(ImGui::GetScrollY() - io.MouseDelta.y);
    if (settings) { ImGui::OpenPopup("New game settings"); settings=false; }
    ImGui::SetNextWindowSizeConstraints({0,0},{io.DisplaySize.x-16,io.DisplaySize.y-16});
    if (ImGui::BeginPopupModal("New game settings",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Combo("Your side",&sideChoice,"White\0Black\0");
        ImGui::Combo("Opponent",&opponentChoice,"Computer\0Local two-player\0");
        ImGui::Combo("Difficulty",&levelChoice,"Easy\0Medium\0Hard\0");
        ImGui::TextWrapped("Starting a new game replaces the saved game.");
        if (ImGui::Button("Start")) { game.newGame(sideChoice==0,opponentChoice==0,levelChoice); resultShown=false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (game.promotionPending()) ImGui::OpenPopup("Promote pawn");
    ImGui::SetNextWindowSizeConstraints({0,0},{io.DisplaySize.x-16,io.DisplaySize.y-16});
    if (ImGui::BeginPopupModal("Promote pawn",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        for (int kind : {chess::Queen,chess::Rook,chess::Bishop,chess::Knight}) if (ImGui::Button(names[kind])) { game.promote(kind); ImGui::CloseCurrentPopup(); }
        if (ImGui::Button("Cancel")) { game.cancelPromotion(); ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    if (!game.gameOver()) resultShown=false;
    if (game.gameOver() && !game.replay && !game.animation && !resultShown) { ImGui::OpenPopup("Game over"); resultShown=true; }
    ImGui::SetNextWindowSizeConstraints({0,0},{io.DisplaySize.x-16,io.DisplaySize.y-16});
    if (ImGui::BeginPopupModal("Game over",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(game.status().c_str());
        if (ImGui::Button("Play again")) { game.newGame(game.white,game.computer,game.difficulty); ImGui::CloseCurrentPopup(); }
        if (ImGui::Button("View board")) ImGui::CloseCurrentPopup();
        if (ImGui::Button("Replay game")) { game.startReplay(); ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
    ImGui::End();
}
}
