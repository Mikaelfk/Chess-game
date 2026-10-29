#include "Frontend/Game.h"
#include "third_party/json.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>
using frontend::Game;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
void finish(Game& g) { for(int i=0;i<3;++i) g.tick(.1); }
void move(Game& g,int from,int to,bool drag=false) {
    finish(g); const auto before=g.position().repetitionKey();
    if(drag) { require(g.beginDrag(from),"Drag rejected"); g.drop(to); }
    else { g.select(from); g.select(to); }
    require(before!=g.position().repetitionKey() || g.promotionPending(),"Move rejected");
}
void wait(Game& g,bool analysis=false) {
    for(int i=0;i<500;++i) {
        g.tick(.05); std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if(analysis ? g.suggestion!="Finding a move..." : !g.thinking()) return;
    }
    throw std::runtime_error("Worker timed out");
}
void fool(Game& g) { g.newGame(true,false,0); for(auto m:{std::pair{53,45},{12,28},{54,38},{3,39}}) move(g,m.first,m.second); finish(g); }
int main() {
try {
    Game g; g.newGame(true,false,0);
    g.select(-1); g.select(64); require(g.selected==-1,"Invalid selection accepted");
    g.select(52); require(g.legalTarget(36)&&!g.legalTarget(28),"Legal hints incorrect");
    require(!g.beginDrag(12),"Opponent drag accepted");
    require(g.beginDrag(52),"Pawn drag rejected"); g.drop(-1); g.drop(28);
    require(g.recordedMoves().empty(),"Illegal drop played"); g.drop(36);
    require(!g.animation && g.recordedMoves().size()==1,"Drag was animated");
    move(g,11,27); require(g.animation.has_value(),"Tap animation missing"); finish(g);
    move(g,36,27); require(g.material()==1,"Capture material incorrect");
    require(g.capturedBy(true)[chess::Pawn]==1 && g.capturedBy(false)[chess::Pawn]==0,"Capture assigned to wrong player");
    const auto saved=g.serialize(); Game restored; require(restored.restore(saved),"Restore failed");
    require(restored.serialize()==saved,"Save roundtrip changed data");
    require(restored.capturedBy(true)==g.capturedBy(true),"Restore lost captured pieces");
    for(auto invalid:{"{}","null","invalid","[]"}) require(!restored.restore(invalid),"Malformed save accepted");
    auto j=nlohmann::json::parse(saved); j["moves"][0][0]=1.5; require(!restored.restore(j.dump()),"Fractional move accepted");
    j=nlohmann::json::parse(saved); j["difficulty"]=5; require(!restored.restore(j.dump()),"Invalid difficulty accepted");
    require(restored.serialize()==saved,"Invalid restore mutated game");
    for(int kind:{chess::Queen,chess::Rook,chess::Bishop,chess::Knight}) {
        g.newGame(true,false,0);
        for(auto m:{std::pair{48,32},{15,31},{32,24},{31,39},{24,16},{39,47},{16,9},{47,54}}) move(g,m.first,m.second,true);
        move(g,9,0,true); require(g.promotionPending(),"Promotion missing");
        g.promote(chess::King); require(g.promotionPending(),"King promotion accepted");
        g.cancelPromotion(); require(!g.promotionPending(),"Promotion cancellation failed");
        move(g,9,0,true); g.promote(kind);
        require(g.position().squares[0][0]==kind&&!g.animation,"Dragged promotion animated or wrong piece");
        require(g.capturedBy(false)[chess::Pawn]==1,"Promoted pawn was counted as captured");
        require(g.capturedBy(true)[chess::Rook]==1,"Promotion capture omitted the rook");
    }
    g.newGame(true,false,0);
    for(auto m:{std::pair{52,36},{8,16},{36,28},{11,27},{28,19}}) move(g,m.first,m.second,true);
    require(g.capturedBy(true)[chess::Pawn]==1,"En passant capture missing");
    move(g,15,23,true); move(g,19,12,true);
    require(g.capturedBy(true)[chess::Pawn]==2,"Repeated pawn captures were not grouped");
    g.newGame(true,false,0);
    for(auto m:{std::pair{52,36},{11,27},{36,27},{3,27}}) move(g,m.first,m.second,true);
    for(int cycle=0;cycle<2;++cycle) for(auto m:{std::pair{62,45},{6,21},{45,62},{21,6}}) move(g,m.first,m.second,true);
    require(g.gameOver(),"Capture replay setup did not finish");
    g.startReplay(); g.seek(3);
    require(g.capturedBy(true)[chess::Pawn]==1 && g.capturedBy(false)[chess::Pawn]==0,"Replay capture counts include future moves");
    g.seek(4); require(g.capturedBy(false)[chess::Pawn]==1,"Replay omitted Black's capture");
    g.seek(2); move(g,57,42,true);
    require(g.capturedBy(true)[chess::Pawn]==0,"Analysis variation retained a discarded capture");
    g.restoreRecording(); g.seek(3);
    require(g.capturedBy(true)[chess::Pawn]==1,"Returning to recording lost its capture");
    fool(g); require(g.gameOver()&&g.status().find("checkmate")!=std::string::npos,"Checkmate missing");
    const auto final=g.serialize(); g.startReplay(); require(g.replay&&g.step==0,"Replay did not start");
    wait(g,true); require(!g.suggestion.empty(),"Replay suggestion missing");
    move(g,52,36,true); require(g.branchRoot==0&&g.replayLength()==1,"Variation did not branch");
    require(g.serialize()==final,"Replay changed saved game");
    g.restoreRecording(); require(!g.branchRoot&&g.replayLength()==4&&g.step==0,"Recording restore failed");
    g.seek(4); require(g.capturedBy(false)[chess::Pawn]==0,"Replay reported nonexistent captures"); require(g.evaluation==0&&g.evaluationText.find("checkmate")!=std::string::npos,"Mate evaluation incorrect");
    g.seek(0); g.seek(4); wait(g); require(g.evaluation==0,"Stale analysis replaced mate");
    g.exitReplay(); require(g.serialize()==final,"Replay changed finished position");
    g.newGame(true,false,0);
    for(int i=0;i<2;++i) for(auto m:{std::pair{62,45},{6,21},{45,62},{21,6}}) move(g,m.first,m.second,true);
    require(g.gameOver()&&g.status().find("repetition")!=std::string::npos,"Repetition draw missing");
    for(int level=0;level<3;++level) {
        g.newGame(false,true,level); require(g.thinking(),"Computer opening not queued");
        g.tick(.25); require(g.recordedMoves().empty(),"Computer pause missing");
        wait(g); require(g.recordedMoves().size()==1&&g.position().whiteToMove==false,"Computer opening failed");
    }
    g.newGame(false,true,2); g.tick(.25); g.tick(.25);
    g.newGame(true,false,0); wait(g); require(g.recordedMoves().empty(),"Cancelled AI changed new game");
    g.newGame(false,true,0); g.setActive(false); g.tick(.25); require(!g.thinking()&&g.recordedMoves().empty(),"Background search ran");
    g.setActive(true); wait(g); require(g.recordedMoves().size()==1,"AI failed to resume");
    const auto path=std::filesystem::temp_directory_path()/"chess-imgui-test.json";
    { Game disk(path.string()); disk.newGame(true,false,0); move(disk,52,36,true); }
    { Game disk(path.string()); require(disk.recordedMoves().size()==1,"Disk restore failed"); disk.newGame(false,false,1); }
    { Game disk(path.string()); require(!disk.white&&!disk.computer&&disk.recordedMoves().empty(),"Save replacement failed"); }
    std::filesystem::remove(path);
    std::cout<<"Frontend controller tests passed\n";
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
