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
int main() {
try {
    ImGui::CreateContext(); auto& io=ImGui::GetIO(); io.IniFilename=nullptr;
    unsigned char* pixels; int w,h; io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h); io.Fonts->SetTexID(1);
    Game g; g.newGame(true,false,0); ChessUI ui(g); io.DisplaySize={390,820};
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
    for(auto size:{ImVec2{320,568},ImVec2{800,360},ImVec2{1280,900}}) {
        io.DisplaySize=size; frame(ui,{-100,-100},false); require(ui.boardSize<=size.x&&ui.boardSize<=600,"Responsive board sizing failed");
    }
    ImGui::DestroyContext(); std::cout<<"ImGui input and layout tests passed\n";
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
