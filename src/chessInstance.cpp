#include "chessInstance.h"
#include "board.h"
#include "board_base.h"
#include "chessInstanceIO.h"
#include "move.h"
#include "piece.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
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

void ChessInstance::setFEN(const wstring& pieceChars)
{
    info[L"FEN"] = __pieceCharsToFEN(pieceChars) + L" " + (firstColor == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
}

const wstring ChessInstance::getPieceChars()
{
    wstring rfen{ info[L"FEN"] };
    return __fenToPieceChars(rfen.substr(0, rfen.find(L' ')));
}

void ChessInstance::setBoard() { pboard->set(getPieceChars()); }

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void ChessInstance::initSet(const RecFormat fmt)
{
    auto __setRem = [&](const Move& move) {
        int length = move.remark.size();
        if (length > 0) {
            remCount += 1;
            if (length > remLenMax)
                remLenMax = length;
        }
    };

    function<void(Move&)> __set = [&](Move& move) {
        switch (fmt) {
        case RecFormat::ICCS: {
            move.setSeat(pboard->getSeat__ICCS(move.ICCS));
            move.zh = pboard->getZH(move.fseat(), move.tseat());
            break;
        }
        case RecFormat::ZH:
        case RecFormat::CC: {
            move.setSeat(pboard->getSeat__Zh(move.zh));
            move.ICCS = pboard->getICCS(move.fseat(), move.tseat());
            /*
            wstring zh{ getZH(move.fseat(), move.tseat()) };
            // wcout << move.toString_zh() << L'\n';
            if (move.zh != zh) {
                wcout << L"move.zh: " << move.zh << L'\n'
                      << L"getZH( ): " << zh << L'\n'
                      << move.toString() << L'\n' << pboard->toString() << endl;
                return;
            } //*/
            break;
        }
        case RecFormat::XQF:
        case RecFormat::BIN:
        case RecFormat::JSON: {
            move.ICCS = pboard->getICCS(move.fseat(), move.tseat());
            move.zh = pboard->getZH(move.fseat(), move.tseat());
            /*
            auto seats = getSeat__Zh(move.zh);
            // wcout << move.toString() << L'\n';
            if ((seats.first != move.fseat()) || (seats.second != move.tseat())) {
                wcout << L"move.fs_ts: " << move.fseat() << L' ' << move.tseat() << L'\n'
                      << L"getSeat__Zh( ): " << move.zh << L'\n'
                      << move.toString() << L'\n' << pboard->toString() << endl;
                return;
            } //*/
            break;
        }
        default:
            break;
        }

        movCount += 1;
        __setRem(move);
        move.maxCol = maxCol; // # 本着在视图中的列数
        if (move.othCol > othCol)
            othCol = move.othCol;
        if (move.stepNo > maxRow)
            maxRow = move.stepNo;
        pboard->go(move);
        //wcout << move.toString() << L"\n" << pboard->toString() << endl;

        if (move.next())
            __set(*move.next());
        pboard->back(move);
        if (move.other()) {
            maxCol += 1;
            __set(*move.other());
        }
    };

    __setRem(*prootMove);
    if (prootMove->next())
        __set(*prootMove->next()); // 驱动函数
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
        initSet(RecFormat::BIN); //借用？
    to(curmove);
}

const wstring ChessInstance::__fenToPieceChars(const wstring fen)
{
    //'数字字符对应下划线字符串'
    vector<pair<wchar_t, wstring>> num_lines{
        { L'9', L"_________" }, { L'8', L"________" }, { L'7', L"_______" },
        { L'6', L"______" }, { L'5', L"_____" }, { L'4', L"____" },
        { L'3', L"___" }, { L'2', L"__" }, { L'1', L"_" }
    };
    wstring chars{};
    wregex sp{ LR"(/)" };
    for (wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != wsregex_token_iterator{}; ++wti)
        chars.insert(0, *wti);
    wstring::size_type pos;
    for (auto& numline : num_lines)
        while ((pos = chars.find(numline.first)) != wstring::npos)
            chars.replace(pos, 1, numline.second);
    return chars;
}

const wstring ChessInstance::__pieceCharsToFEN(const wstring& pieceChars)
{
    //'下划线字符串对应数字字符'
    vector<pair<wstring, wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    wstring fen{};
    for (int i = 81; i >= 0; i -= 9)
        fen += pieceChars.substr(i, 9) + L"/";
    fen.erase(fen.size() - 1, 1);
    wstring::size_type pos;
    for (auto linenum : line_nums)
        while ((pos = fen.find(linenum.first)) != wstring::npos)
            fen.replace(pos, linenum.first.size(), linenum.second);
    return fen;
}
