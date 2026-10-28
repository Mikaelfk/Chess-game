#include "../ChessBackend/ChessAI.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
using namespace chess;
void require(bool ok,const char* message) { if(!ok) {std::cerr<<message<<'\n';std::exit(1);} }
int main() {
    std::atomic_bool stop(false);
    for (bool white : {true,false}) for (Difficulty level : {Difficulty::Easy,Difficulty::Medium,Difficulty::Hard}) {
        auto p=Position::initial(); p.whiteToMove=white;
        const auto before=p.repetitionKey();
        const auto result=chooseMove(p,level,stop,42);
        auto legal=p.legalMoves();
        require(result.move && std::find(legal.begin(),legal.end(),*result.move)!=legal.end(), "AI chose illegal move");
        require(before==p.repetitionKey(), "AI changed live position");
        if(level==Difficulty::Easy) require(result.completedDepth==0, "Easy should use random legal moves");
        if(level==Difficulty::Medium) require(result.completedDepth>=1 && result.completedDepth<=2, "Medium search depth incorrect");
        if(level==Difficulty::Hard) require(result.completedDepth>=2 && result.nodes<=100001, "Hard search budget incorrect");
    }
    // Both colours should take a free queen rather than a quiet move.
    for(bool white : {true,false}) {
        Position p; p.whiteToMove=white; p.squares[7][7]=King; p.squares[0][7]=King+6;
        const int row=white?4:3;
        p.squares[row][0]=Rook+(white?0:6); p.squares[row][3]=Queen+(white?6:0);
        const auto result=chooseMove(p,Difficulty::Medium,stop,9);
        require(result.move && result.move->toRow==row && result.move->toCol==3, "AI missed free queen");
        require(result.whiteEvaluation && (white ? *result.whiteEvaluation>0 : *result.whiteEvaluation<0),
                "Evaluation sign does not identify the leading colour");
    }
    Position p; p.squares[0][0]=King+6; p.squares[2][2]=King; p.squares[2][1]=Queen; p.whiteToMove=false;
    require(!chooseMove(p,Difficulty::Hard,stop,1).move, "AI moved after stalemate");
    p=Position::initial();
    for(Move m:{Move{6,5,5,5},{1,4,3,4},{6,6,4,6}}) require(p.play(m), "Mate setup failed");
    const auto mate=chooseMove(p,Difficulty::Medium,stop,1);
    require(mate.move && p.after(*mate.move).outcome()==Outcome::Checkmate, "AI missed mate in one");
    require(mate.whiteEvaluation && *mate.whiteEvaluation < -90000,"Forced mate evaluation missing");
    stop=true;
    require(!chooseMove(Position::initial(),Difficulty::Hard,stop,1).move, "Cancelled AI returned move");
}
