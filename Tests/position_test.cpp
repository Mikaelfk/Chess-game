#include "../ChessBackend/Position.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cctype>
using namespace chess;
void require(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
long long perft(const Position& p, int depth) {
    if (!depth) return 1;
    long long count = 0;
    for (const auto& m : p.legalMoves()) count += perft(p.after(m), depth-1);
    return count;
}
Position empty() {
    Position p;
    p.squares[7][4] = King;
    p.squares[0][4] = King+6;
    return p;
}
Position fromFen(const std::string& fen) {
    Position p;
    std::istringstream stream(fen);
    std::string board, side, rights, ep;
    stream >> board >> side >> rights >> ep;
    int row=0,col=0;
    for(char ch:board) {
        if(ch=='/') {++row; col=0;}
        else if(std::isdigit(static_cast<unsigned char>(ch))) col+=ch-'0';
        else {
            const std::string codes=" pnbrqk";
            p.squares[row][col++]=int(codes.find(char(std::tolower(static_cast<unsigned char>(ch))))) + (std::isupper(static_cast<unsigned char>(ch))?0:6);
        }
    }
    p.whiteToMove=side=="w";
    const std::string flags="KQkq";
    for(int i=0;i<4;++i) p.castling[i]=rights.find(flags[i])!=std::string::npos;
    if(ep!="-") {p.enPassantRow='8'-ep[1];p.enPassantCol=ep[0]-'a';}
    return p;
}

bool has(const Position& p, Move m) { const auto moves = p.legalMoves(); return std::find(moves.begin(), moves.end(), m) != moves.end(); }
int main() {
    Position p = Position::initial();
    require(perft(p,1)==20 && perft(p,2)==400 && perft(p,3)==8902, "Initial position move counts differ from standard chess");
    // Published perft positions cover castling, pins and en-passant interactions.
    p=fromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    require(perft(p,1)==48 && perft(p,2)==2039 && perft(p,3)==97862, "Kiwipete move counts incorrect");
    p=fromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    require(perft(p,1)==14 && perft(p,2)==191 && perft(p,3)==2812, "Pawn endgame move counts incorrect");
    // Castling: both sides, rook presence, attacked transit and captured-rook rights.
    p = empty(); p.castling.fill(true); p.squares[7][0] = p.squares[7][7] = Rook;
    require(has(p,{7,4,7,6}) && has(p,{7,4,7,2}), "White castling missing");
    auto castled = p.after({7,4,7,6});
    require(castled.squares[7][5]==Rook && castled.squares[7][6]==King && !castled.castling[0] && !castled.castling[1], "Castling state incorrect");
    p.squares[0][5] = Rook+6;
    require(!has(p,{7,4,7,6}), "Castling through check accepted");
    p.squares[0][5] = 0; p.squares[7][7] = 0;
    require(!has(p,{7,4,7,6}), "Castling without rook accepted");
    p = empty(); p.whiteToMove = false; p.castling.fill(true); p.squares[0][0] = p.squares[0][7] = Rook+6;
    require(has(p,{0,4,0,6}) && has(p,{0,4,0,2}), "Black castling missing");
    p.squares[7][7] = Rook; p.whiteToMove = true;
    require(p.play({7,7,0,7}) && !p.castling[2], "Captured rook retained castling right");
    // En passant removes the correct pawn and expires after one turn.
    p = Position::initial();
    for (Move m : {Move{6,4,4,4},{1,0,2,0},{4,4,3,4},{1,3,3,3}}) require(p.play(m), "En-passant setup failed");
    require(has(p,{3,4,2,3}), "En passant missing");
    auto captured = p.after({3,4,2,3});
    require(captured.squares[3][3]==0 && captured.squares[2][3]==Pawn, "En-passant capture incorrect");
    require(p.play({7,6,5,5}) && p.enPassantRow == -1, "En passant did not expire");
    p = empty(); p.squares[7][4]=0; p.squares[3][7]=King; p.squares[3][6]=Pawn; p.squares[3][5]=Pawn+6; p.squares[3][0]=Rook+6;
    p.enPassantRow=2; p.enPassantCol=5;
    require(!has(p,{3,6,2,5}), "En passant exposing own king accepted");
    // Promotion choices and pinned pieces.
    for (bool white : {true,false}) {
        p = empty(); p.whiteToMove=white;
        const int from=white?1:6, to=white?0:7;
        p.squares[from][0]=Pawn+(white?0:6);
        for (int type : {Queen,Rook,Bishop,Knight}) {
            auto promoted=p;
            require(promoted.play({from,0,to,0,type}) && promoted.squares[to][0]==type+(white?0:6), "Promotion failed");
        }
    }
    p=empty(); p.squares[6][4]=Rook; p.squares[1][4]=Rook+6;
    require(!has(p,{6,4,6,5}), "Pinned rook can expose its king");
    require(p.squares[6][4]==Rook && p.whiteToMove, "Move generation modified position");
    // Fool's mate and a genuine stalemate are distinct outcomes.
    p=Position::initial();
    for (Move m : {Move{6,5,5,5},{1,4,3,4},{6,6,4,6},{0,3,4,7}}) require(p.play(m), "Mate setup failed");
    require(p.outcome()==Outcome::Checkmate, "Checkmate not detected");
    p={}; p.squares[0][0]=King+6; p.squares[2][2]=King; p.squares[2][1]=Queen; p.whiteToMove=false;
    require(p.outcome()==Outcome::Stalemate, "Stalemate not detected");
    p=empty(); require(p.outcome()==Outcome::InsufficientMaterial, "King-only draw missing");
    p.squares[7][0]=Rook; p.halfmoveClock=100;
    require(p.outcome()==Outcome::FiftyMoveDraw, "Fifty-move draw missing");
    p=Position::initial(); auto key=p.repetitionKey();
    for (Move m : {Move{7,6,5,5},{0,6,2,5},{5,5,7,6},{2,5,0,6}}) require(p.play(m), "Repetition setup failed");
    require(p.repetitionKey()==key, "Repetition key depends on move counters");
    p.enPassantRow=2; p.enPassantCol=3;
    require(p.repetitionKey()==key, "Unusable en-passant target affected repetition");
}
