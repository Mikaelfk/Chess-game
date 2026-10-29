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
void testInitialPositionAndMoves() {
    Position p = Position::initial();
    const int backRank[] = {Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook};
    for (int col = 0; col < 8; ++col) {
        require(p.squares[0][col] == backRank[col] + 6 && p.squares[1][col] == Pawn + 6,
                "Black starting pieces are incorrect");
        require(p.squares[6][col] == Pawn && p.squares[7][col] == backRank[col],
                "White starting pieces are incorrect");
        for (int row = 2; row < 6; ++row) require(p.squares[row][col] == Empty, "Starting middle ranks are not empty");
    }
    require(p.whiteToMove && std::all_of(p.castling.begin(), p.castling.end(), [](bool right) { return right; }),
            "Starting turn or castling rights are incorrect");
    require(p.enPassantRow == -1 && p.enPassantCol == -1 && p.halfmoveClock == 0,
            "Starting move counters are incorrect");
    require(!p.inCheck(true) && !p.inCheck(false) && p.outcome() == Outcome::Playing,
            "Initial position is not a playable game");
    require(p.play({6,4,4,4}), "White pawn could not make its opening double step");
    require(p.squares[6][4] == Empty && p.squares[4][4] == Pawn && !p.whiteToMove,
            "Opening move did not update board and turn");
    require(p.enPassantRow == 5 && p.enPassantCol == 4, "Double step did not set the en-passant square");
    const Position before = p;
    for (Move rejected : {Move{4,4,3,4}, {1,0,4,0}, {0,0,3,0}, {1,0,-1,0}, {-1,0,2,0}}) {
        require(!p.play(rejected), "Illegal move was accepted");
        require(p.squares == before.squares && p.whiteToMove == before.whiteToMove && p.castling == before.castling
                && p.enPassantRow == before.enPassantRow && p.enPassantCol == before.enPassantCol
                && p.halfmoveClock == before.halfmoveClock, "Rejected move modified game state");
    }
}

void testCheckAndBlockingMove() {
    Position p = Position::initial();
    for (Move move : {Move{6,4,4,4}, {1,5,3,5}, {7,3,3,7}}) {
        require(p.play(move), "Check setup move was rejected");
    }
    require(p.inCheck(false) && !p.inCheck(true) && p.outcome() == Outcome::Playing,
            "Queen check was not detected correctly");
    require(!p.play({1,0,2,0}), "A player ignored check");
    require(p.play({1,6,2,6}), "Blocking check was rejected");
    require(!p.inCheck(false) && !p.inCheck(true), "Blocking move did not remove check");
}

void testInsufficientMaterial() {
    require(empty().insufficientMaterial(), "Bare kings must draw");
    for (int minor : std::array<int, 4>{Bishop, Knight, Bishop + 6, Knight + 6}) {
        auto p = empty();
        p.squares[4][4] = minor;
        require(p.insufficientMaterial() && p.outcome() == Outcome::InsufficientMaterial,
                "King and a single minor must draw");
    }
    auto p = empty();
    p.squares[4][4] = Bishop;
    p.squares[2][2] = Bishop + 6;
    require(p.insufficientMaterial(), "Same-colour bishops must draw");
    p.squares[2][2] = Empty;
    p.squares[2][3] = Bishop + 6;
    require(!p.insufficientMaterial(), "Opposite-colour bishops can permit mate");
    p.squares[2][3] = Knight + 6;
    require(!p.insufficientMaterial(), "Bishop versus knight can permit mate");
    p.squares[4][4] = Knight;
    require(!p.insufficientMaterial(), "Two knights must not trigger an automatic material draw");
    for (int piece : std::array<int, 6>{Pawn, Rook, Queen, Pawn + 6, Rook + 6, Queen + 6}) {
        p = empty();
        p.squares[4][4] = piece;
        require(!p.insufficientMaterial(), "Pawn or major piece incorrectly caused a material draw");
    }
}

void testDrawClockAndMatePrecedence() {
    auto p = empty();
    p.squares[7][0] = Rook;
    p.halfmoveClock = 99;
    require(p.outcome() == Outcome::Playing, "Fifty-move draw occurred before 100 halfmoves");
    require(p.play({7,0,6,0}) && p.halfmoveClock == 100 && p.outcome() == Outcome::FiftyMoveDraw,
            "Quiet move did not reach the fifty-move threshold");
    p = Position::initial(); p.halfmoveClock = 99;
    require(p.play({6,4,5,4}) && p.halfmoveClock == 0 && p.outcome() == Outcome::Playing,
            "Pawn move did not reset the draw clock");
    p = empty(); p.squares[4][0] = Rook; p.squares[4][3] = Bishop + 6; p.halfmoveClock = 99;
    require(p.play({4,0,4,3}) && p.halfmoveClock == 0 && p.outcome() == Outcome::Playing,
            "Capture did not reset the draw clock");
    p = fromFen("k7/1Q6/2K5/8/8/8/8/8 b - -"); p.halfmoveClock = 100;
    require(p.outcome() == Outcome::Checkmate, "Fifty-move rule hid checkmate");
}

void testRepetitionIdentity() {
    const auto initial = Position::initial();
    auto changed = initial; changed.whiteToMove = false;
    require(changed.repetitionKey() != initial.repetitionKey(), "Repetition ignored the side to move");
    for (int side = 0; side < 4; ++side) {
        changed = initial; changed.castling[side] = false;
        require(changed.repetitionKey() != initial.repetitionKey(), "Repetition ignored lost castling rights");
    }
    auto p = fromFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6");
    auto withoutTarget = p; withoutTarget.enPassantRow = withoutTarget.enPassantCol = -1;
    require(has(p, {3,4,2,3}) && p.repetitionKey() != withoutTarget.repetitionKey(),
            "Repetition ignored a legal en-passant capture");
    p = fromFen("4k3/8/8/r4pPK/8/8/8/8 w - f6");
    withoutTarget = p; withoutTarget.enPassantRow = withoutTarget.enPassantCol = -1;
    require(!has(p, {3,6,2,5}) && p.repetitionKey() == withoutTarget.repetitionKey(),
            "Pinned en-passant capture incorrectly changed repetition identity");
}

int main() {
    testInsufficientMaterial();
    testDrawClockAndMatePrecedence();
    testRepetitionIdentity();
    testInitialPositionAndMoves();
    testCheckAndBlockingMove();
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
