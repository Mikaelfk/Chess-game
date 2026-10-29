#include "Frontend/ChessUI.h"
#include <iostream>
#include <stdexcept>
using namespace frontend;
void require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
void frame(ChessUI& ui,ImVec2 point,bool down) {
    auto& io=ImGui::GetIO(); io.DeltaTime=1.f/60; io.AddMousePosEvent(point.x,point.y); io.AddMouseButtonEvent(0,down);
    ImGui::NewFrame(); ui.draw(); ImGui::Render();
    require(ImGui::GetDrawData()->TotalVtxCount>0,"UI did not generate geometry");
}
bool captureIconAt(const ChessUI& ui, bool whitePlayer, int piece) {
    const float row = ui.captureRowY[whitePlayer ? 0 : 1];
    const float height = ImGui::GetFontSize()*1.4f;
    for (const auto* list : ImGui::GetDrawData()->CmdLists) for (const auto& cmd : list->CmdBuffer) {
        if (cmd.GetTexID() != ImTextureID(100+piece)) continue;
        for (unsigned int i=0;i+2<cmd.ElemCount;i+=3) {
            bool inRow = true;
            for (unsigned int j=0;j<3;++j) {
                const auto& vertex=list->VtxBuffer[cmd.VtxOffset+list->IdxBuffer[cmd.IdxOffset+i+j]];
                if (vertex.pos.y<row-.1f || vertex.pos.y>row+height+.1f) inRow=false;
            }
            if(inRow) return true;
        }
    }
    return false;
}
int main() {
try {
    ImGui::CreateContext(); auto& io=ImGui::GetIO(); io.IniFilename=nullptr;
    unsigned char* pixels; int w,h; io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h); io.Fonts->SetTexID(1);
    Game g; g.newGame(true,false,0); ChessUI ui(g); io.DisplaySize={390,820};
    for(int level=0;level<5;++level) ui.textures[chess::Pawn][level]=200+level;
    require(ui.pieceTexture(chess::Pawn,24)==200 && ui.pieceTexture(chess::Pawn,65)==202,"Texture level selection is incorrect");
    io.DisplayFramebufferScale={2,2};
    require(ui.pieceTexture(chess::Pawn,40)==202,"High-DPI texture selection is incorrect");
    io.DisplayFramebufferScale={1,1};
    for(int p=1;p<=12;++p) ui.textures[p].fill(100+p);
    frame(ui,{-100,-100},false); frame(ui,{-100,-100},false);
    require(ui.boardSize<=390,"Board exceeds phone width");
    require(ui.squareAt({ui.boardOrigin.x-1,ui.boardOrigin.y})==-1,"Outside board mapping accepted");
    for(bool white:{true,false}) {
        g.newGame(white,false,0); frame(ui,{-100,-100},false);
        auto from=ui.squareCenter(52),to=ui.squareCenter(36);
        require(ui.squareAt(from)==52&&ui.squareAt(to)==36,"Orientation mapping failed");
        frame(ui,from,true); frame(ui,to,true); frame(ui,to,false);
        require(g.recordedMoves().size()==1&&!g.animation,"Actual ImGui drag did not play cleanly");
    }
    g.newGame(true,false,0); frame(ui,{-100,-100},false);
    auto from=ui.squareCenter(52),to=ui.squareCenter(36);
    frame(ui,from,true); frame(ui,from,false); frame(ui,to,true); frame(ui,to,false);
    require(g.recordedMoves().size()==1&&g.animation.has_value(),"Actual ImGui tap did not animate");
    g.newGame(true,false,0);
    for(auto m:{std::pair{52,36},{11,27},{36,27},{3,27}}) {
        require(g.beginDrag(m.first),"Capture setup drag failed"); g.drop(m.second);
    }
    for(bool white:{true,false}) {
        g.white=white; frame(ui,{-100,-100},false);
        require(ui.captureRowY[white?0:1]>ui.boardOrigin.y+ui.boardSize,"Player capture row is not below board");
        require(ui.captureRowY[white?1:0]<ui.boardOrigin.y,"Opponent capture row is not above board");
        require(captureIconAt(ui,true,7),"Black captured pawn icon missing from White's row");
        require(captureIconAt(ui,false,1),"White captured pawn icon missing from Black's row");
    }
    for(auto size:{ImVec2{320,568},ImVec2{800,360},ImVec2{1280,900}}) {
        io.DisplaySize=size; frame(ui,{-100,-100},false); require(ui.boardSize<=size.x&&ui.boardSize<=600,"Responsive board sizing failed");
    }
    ImGui::DestroyContext(); std::cout<<"ImGui input and layout tests passed\n";
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
