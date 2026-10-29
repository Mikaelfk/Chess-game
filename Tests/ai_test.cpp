#include "../ChessBackend/ChessAI.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
using namespace chess;
void require(bool ok,const char* message) { if(!ok) {std::cerr<<message<<'\n';std::exit(1);} }
void testTerminalPositionsAndCancellation() {
    Position kings;
    kings.squares[7][7] = King; kings.squares[0][0] = King + 6;
    auto fiftyMove = kings; fiftyMove.squares[4][0] = Rook; fiftyMove.halfmoveClock = 100;
    auto stalemate = kings; stalemate.squares[7][7] = Empty; stalemate.squares[2][2] = King;
    stalemate.squares[2][1] = Queen; stalemate.whiteToMove = false;
    auto checkmate = stalemate; checkmate.squares[2][1] = Empty; checkmate.squares[1][1] = Queen;
    std::atomic_bool stop(false);
    for (auto level : {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard}) {
        for (const auto& p : {kings, fiftyMove, stalemate, checkmate}) {
            require(p.outcome() != Outcome::Playing, "Terminal AI test position is still playable");
            const auto result = chooseMove(p, level, stop, 42);
            require(!result.move && !result.whiteEvaluation && result.nodes == 0 && result.completedDepth == 0,
                    "AI searched or suggested a move after game over");
        }
        stop = true;
        const auto result = chooseMove(Position::initial(), level, stop, 42);
        require(!result.move && !result.whiteEvaluation && result.nodes == 0 && result.completedDepth == 0,
                "Pre-cancelled search did work or returned a move");
        stop = false;
    }
}

void testSeededEasyMove() {
    const auto p = Position::initial();
    std::atomic_bool stop(false);
    const auto first = chooseMove(p, Difficulty::Easy, stop, 1234);
    const auto second = chooseMove(p, Difficulty::Easy, stop, 1234);
    require(first.move && second.move && *first.move == *second.move, "Easy move is not repeatable with the same seed");
    require(first.nodes == 0 && first.completedDepth == 0 && !first.whiteEvaluation,
            "Easy difficulty unexpectedly searched or reported an engine evaluation");
}

int main() {
    testTerminalPositionsAndCancellation();
    testSeededEasyMove();
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
