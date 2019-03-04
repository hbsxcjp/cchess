#include "chessInstance.h"
#include "chessInstanceIO.h"
#include "board.h"
#include "board_base.h"
#include "move.h"
#include "piece.h"
#include <fstream>
#include <functional>
#include <algorithm>
#include <iostream>
using namespace std;
using namespace Board_base;


// ChessInstance
ChessInstance::ChessInstance()
    : info{ { L"Author", L"" },
        { L"Black", L"" },
        { L"BlackTeam", L"" },
        { L"Date", L"" },
        { L"ECCO", L"" },
        { L"Event", L"" },
        { L"FEN", L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1" },
        { L"Format", L"ZH" },
        { L"Game", L"Chinese Chess" },
        { L"Opening", L"" },
        { L"PlayType", L"" },
        { L"RMKWriter", L"" },
        { L"Red", L"" },
        { L"RedTeam", L"" },
        { L"Result", L"" },
        { L"Round", L"" },
        { L"Site", L"" },
        { L"Title", L"" },
        { L"Variation", L"" },
        { L"Version", L"" } }
    , pboard(make_shared<Board>())
    , prootMove(make_shared<Move>())
    , pcurrentMove(prootMove)
    , firstColor(PieceColor::RED)
{
}

ChessInstance::ChessInstance(const string& filename)
    : ChessInstance()
{
    ChessInstanceIO::read(filename, *this);
}

inline const PieceColor ChessInstance::currentColor() const
{
    return pcurrentMove->stepNo % 2 == 0
        ? firstColor
        : (firstColor == PieceColor::RED ? PieceColor::BLACK
                                         : PieceColor::RED);
}

inline const bool ChessInstance::isStart() const { return bool(pcurrentMove->prev()); }

inline const bool ChessInstance::isLast() const { return bool(pcurrentMove->next()); }

// 基本走法
void ChessInstance::forward()
{
    if (!isLast()) {
        pcurrentMove = pcurrentMove->next();
        pboard->go(*pcurrentMove);
    }
}

void ChessInstance::backward()
{
    if (!isStart()) {
        pboard->back(*pcurrentMove);
        pcurrentMove = pcurrentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void ChessInstance::forwardOther()
{
    if (pcurrentMove->other()) {
        auto toMove = pcurrentMove->other();
        pboard->back(*pcurrentMove);
        pboard->go(*toMove);
        pcurrentMove = toMove;
    }
}

// 复合走法
const vector<shared_ptr<Move>> ChessInstance::getPrevMoves(shared_ptr<Move> pmove) const
{
    vector<shared_ptr<Move>> pmv{ pmove };
    while (!pmove->prev()) {
        pmove = pmove->prev();
        pmv.push_back(pmove);
    }
    std::reverse(pmv.begin(), pmv.end());
    return pmv;
}

void ChessInstance::backwardTo(shared_ptr<Move> pmove)
{
    while (!isStart() && pmove != pcurrentMove) {
        backward();
        pmove = pmove->prev();
    }
}

void ChessInstance::to(shared_ptr<Move> pmove)
{
    if (pmove == pcurrentMove)
        return;
    toFirst();
    for (auto& pm : getPrevMoves(pmove))
        pboard->go(*pm);
    pcurrentMove = pmove;
}

void ChessInstance::toFirst()
{
    while (!isStart())
        backward();
}

void ChessInstance::toLast()
{
    while (!isLast())
        forward();
}

void ChessInstance::go(const int inc)
{
    //function<void(const ChessInstance*)> fward = inc > 0 ? &ChessInstance::forward : &ChessInstance::backward;//未成功!
    auto fward = mem_fn(inc > 0 ? &ChessInstance::forward : &ChessInstance::backward);
    for (int i = abs(inc); i != 0; --i)
        fward(this);
}

void ChessInstance::cutNext() { pcurrentMove->setNext(nullptr); }

void ChessInstance::cutOther()
{
    if (pcurrentMove->other())
        pcurrentMove->setOther(pcurrentMove->other()->other());
}

void ChessInstance::setBoard(const wstring& pieceChars) { pboard->set(pieceChars); }

const wstring ChessInstance::getFEN()
{
    wstring rfen{ info[L"FEN"] };
    return rfen.substr(0, rfen.find(L' '));
}

void ChessInstance::changeSide(const ChangeType ct) // 未测试
{
    auto curmove = pcurrentMove;
    toFirst();
    vector<pair<int, shared_ptr<Piece>>> seatPieces{};
    if (ct == ChangeType::EXCHANGE) {
        firstColor = firstColor == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
        for (auto& piecep : pboard->getLivePies())
            seatPieces.push_back(make_pair((*piecep).seat(), pboard->getOthPie(piecep)));
    } else {
        function<void(Move&)> __seat = [&](Move& move) {
            if (ct == ChangeType::ROTATE)
                move.setSeat(rotateSeat(move.fseat()), rotateSeat(move.tseat()));
            else
                move.setSeat(symmetrySeat(move.fseat()), symmetrySeat(move.tseat()));
            if (move.next())
                __seat(*move.next());
            if (move.other())
                __seat(*move.other());
        };
        if (prootMove->next())
            __seat(*prootMove->next()); // 驱动调用递归函数
        for (auto& piecep : pboard->getLivePies()) {
            auto seat = (*piecep).seat();
            seatPieces.push_back(make_pair(ct == ChangeType::ROTATE ? rotateSeat(seat) : symmetrySeat(seat), piecep));
        }
    }
    pboard->set(seatPieces);
    if (ct != ChangeType::ROTATE)
        ChessInstanceIO::__initSet(*this, RecFormat::BIN); //借用？
    to(curmove);
}
