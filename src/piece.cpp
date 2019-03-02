#include "piece.h"
#include "board.h"
#include "board_base.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
using namespace std;
using namespace Board_base;

Piece::Piece(const wchar_t _char)
    : st{ nullSeat }
    , ch{ _char }
    , clr{ isalpha(_char)
            ? (islower(_char) ? PieceColor::BLACK : PieceColor::RED)
            : PieceColor::BLANK }
{
}

inline const vector<int> Piece::getSeats(const PieceColor bottomColor) { return allSeats; }

const vector<int> Piece::getFilterMoveSeats(Board& board)
{
    vector<int> res{ __MoveSeats(board) };
    auto p = remove_if(res.begin(), res.end(), [&](int seat) {
        return board.getColor(seat) == color();
    });
    return (vector<int>{ res.begin(), p });
}

const vector<int> Piece::getCanMoveSeats(Board& board)
{
    vector<int> res{};
    auto fseat = seat();
    for (auto tseat : getFilterMoveSeats(board)) {
        auto eatPiece = board.move_go(fseat, tseat);
        // 移动棋子后，检测是否会被对方将军
        if (!board.isKilled(color()))
            res.push_back(tseat);
        board.move_back(fseat, tseat, eatPiece);
    }
    return res;
}

inline const vector<int> Piece::__MoveSeats(Board& board) { return allSeats; }

const vector<int> Piece::__filterMove_obstruct(Board& board,
    const vector<pair<int, int>>& move_obs)
{
    vector<int> res{};
    for (auto st_c : move_obs)
        if (board.isBlank(st_c.second))
            res.push_back(st_c.first);
    return res;
}

inline const vector<int> King::getSeats(const PieceColor bottomColor) { return color() == bottomColor ? bottomKingSeats : topKingSeats; }

inline const vector<int> King::__MoveSeats(Board& board) { return getKingMoveSeats(seat()); }

inline const vector<int> Advisor::getSeats(const PieceColor bottomColor) { return color() == bottomColor ? bottomAdvisorSeats : topAdvisorSeats; }

inline const vector<int> Advisor::__MoveSeats(Board& board) { return getAdvisorMoveSeats(seat()); }

inline const vector<int> Bishop::getSeats(const PieceColor bottomColor) { return color() == bottomColor ? bottomBishopSeats : topBishopSeats; }

inline const vector<int> Bishop::__MoveSeats(Board& board) { return __filterMove_obstruct(board, getBishopMove_CenSeats(seat())); }

inline const vector<int> Knight::__MoveSeats(Board& board) { return __filterMove_obstruct(board, getKnightMove_LegSeats((seat()))); }

const vector<int> Rook::__MoveSeats(Board& board)
{
    vector<int> res{};
    auto seatLines = getRookCannonMoveSeat_Lines(seat());
    for (auto seatLine : seatLines)
        for (auto seat : seatLine) {
            res.push_back(seat);
            if (!board.isBlank(seat))
                break;
        }
    return res;
}

const vector<int> Cannon::__MoveSeats(Board& board)
{
    vector<int> res{};
    auto seatLines = getRookCannonMoveSeat_Lines(seat());
    for (auto seatLine : seatLines) {
        bool skip = false;
        for (auto seat : seatLine) {
            if (!skip) {
                if (board.isBlank(seat))
                    res.push_back(seat);
                else
                    skip = true;
            } else if (!board.isBlank(seat)) {
                res.push_back(seat);
                break;
            }
        }
    }
    return res;
}

inline const vector<int> Pawn::getSeats(const PieceColor bottomColor) { return color() == bottomColor ? bottomPawnSeats : topPawnSeats; }

inline const vector<int> Pawn::__MoveSeats(Board& board) { return getPawnMoveSeats(board.isBottomSide(color()), seat()); }

const wstring Piece::toString()
{
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(4) << setw(5) << static_cast<int>(color()) << setw(6)
        << wchar() << setw(6) << seat() << setw(5) << chName() << setw(9)
        << isBlank() << setw(8) << isKing() << setw(8) << isStronge() << L'\n';
    return wss.str();
}

// 相关特征棋子名字串: 类内声明，类外定义
const wstring Pieces::kingNames{ L"帅将" };
const wstring Pieces::pawnNames{ L"兵卒" };
const wstring Pieces::advbisNames{ L"仕相士象" };
const wstring Pieces::strongeNames{ L"马车炮兵卒" };
const wstring Pieces::lineNames{ L"帅车炮兵将卒" };
const wstring Pieces::allNames{ L"帅仕相马车炮兵将士象卒" };

// 类外初始化类内静态const成员
const shared_ptr<Piece> Pieces::nullPiePtr{ make_shared<NullPie>(L'_') }; // 空棋子指针

// 一副棋子类
Pieces::Pieces()
    : piePtrs{
        make_shared<King>(L'K'), make_shared<Advisor>(L'A'), make_shared<Advisor>(L'A'),
        make_shared<Bishop>(L'B'), make_shared<Bishop>(L'B'), make_shared<Knight>(L'N'), make_shared<Knight>(L'N'),
        make_shared<Rook>(L'R'), make_shared<Rook>(L'R'), make_shared<Cannon>(L'C'), make_shared<Cannon>(L'C'),
        make_shared<Pawn>(L'P'), make_shared<Pawn>(L'P'), make_shared<Pawn>(L'P'), make_shared<Pawn>(L'P'), make_shared<Pawn>(L'P'),
        make_shared<King>(L'k'), make_shared<Advisor>(L'a'), make_shared<Advisor>(L'a'),
        make_shared<Bishop>(L'b'), make_shared<Bishop>(L'b'), make_shared<Knight>(L'n'), make_shared<Knight>(L'n'),
        make_shared<Rook>(L'r'), make_shared<Rook>(L'r'), make_shared<Cannon>(L'c'), make_shared<Cannon>(L'c'),
        make_shared<Pawn>(L'p'), make_shared<Pawn>(L'p'), make_shared<Pawn>(L'p'), make_shared<Pawn>(L'p'), make_shared<Pawn>(L'p')
    }
{
}

inline const shared_ptr<Piece> Pieces::getKingPie(const PieceColor color) { return piePtrs[color == PieceColor::RED ? 0 : 16]; }

inline const shared_ptr<Piece> Pieces::getOthPie(const shared_ptr<Piece> pie)
{
    int index{ 0 };
    for (auto& pm : piePtrs) {
        if (pm == pie)
            break;
        ++index;
    }
    return piePtrs[index < 16 ? 16 + index : index - 16];
}

const shared_ptr<Piece> Pieces::getFreePie(const wchar_t ch)
{
    for (auto& p : piePtrs)
        if (p->wchar() == ch && p->seat() == nullSeat)
            return p;
    return nullPiePtr;
}

const vector<shared_ptr<Piece>> Pieces::getLivePies()
{
    vector<shared_ptr<Piece>> res{ piePtrs };
    auto p = remove_if(res.begin(), res.end(),
        [&](shared_ptr<Piece>& p) { return p->seat() == nullSeat; });
    return (vector<shared_ptr<Piece>>{ res.begin(), p });
}

const vector<shared_ptr<Piece>> Pieces::getLivePies(const PieceColor color)
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) { return p->seat() != nullSeat && p->color() == color; });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getLiveStrongePies(const PieceColor color)
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) { return p->seat() != nullSeat && p->color() == color && p->isStronge(); });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getNamePies(const PieceColor color, wchar_t name)
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) {
        return p->seat() != nullSeat && p->color() == color && p->chName() == name;
    });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getNameColPies(const PieceColor color, wchar_t name,
    int col)
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) {
        return p->seat() != nullSeat && p->color() == color && p->chName() == name && getCol(p->seat()) == col;
    });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getEatedPies()
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) { return p->seat() == nullSeat; });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getEatedPies(const PieceColor color)
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](shared_ptr<Piece>& p) { return p->seat() == nullSeat && p->color() == color; });
    return res;
}

void Pieces::clearSeat()
{
    for_each(piePtrs.begin(), piePtrs.end(), [&](shared_ptr<Piece>& p) { p->setSeat(nullSeat); });
}

const wstring Pieces::toString()
{
    wstring ws{
        L"color wchar seat chName isBlank isKing isStronge\n"
    }; 
    for (auto p : piePtrs)
        ws += p->toString();
    return ws + nullPiePtr->toString(); // 关注空棋子的特性!
}

const wstring Pieces::test()
{
    wstringstream wss{};
    wss << L"test "
           L"piece.h\n-----------------------------------------------------\n"
        << L"特征棋子串：\n"
        << kingNames << L' ' << pawnNames << L' ' << advbisNames << L' '
        << strongeNames << L' ' << lineNames << L' ' << allNames << L"\n";

    wss << L"Pieces::toString:\n";
    wss << toString() << L"共计：" << piePtrs.size() << L"个\n";

    wss << L"Piece::getKingPie\\Piece::chName：\n红棋->"
        << getKingPie(PieceColor::RED)->chName() << L' ' << L"黑棋->"
        << getKingPie(PieceColor::BLACK)->chName();

    wss << L"\nPieces::getOthPie：\n";
    for (auto piePtr : piePtrs)
        wss << piePtr->chName() << L"->" << getOthPie(piePtr)->chName() << L' ';

    for (auto id : (vector<int>{ 0, 1, 2, 3, 4, 5, 9, 10, 17, 18 }))
        piePtrs[id]->setSeat(id);
    wss << L"\nPieces::getLivePies：\n";
    wss << L"getLivePies()：";
    for (auto& piePtr : getLivePies())
        wss << piePtr->chName() << L' ';
    wss << L"\ngetLivePies(PieceColor::RED)：";
    for (auto& piePtr : getLivePies(PieceColor::RED))
        wss << piePtr->chName() << L' ';
    wss << L"\ngetLivePies(PieceColor::BLACK)：";
    for (auto& piePtr : getLivePies(PieceColor::BLACK))
        wss << piePtr->chName() << L' ';
    wss << L"\ngetEatedPies()：";
    for (auto& piePtr : getEatedPies())
        wss << piePtr->chName() << L' ';
    wss << L"\ngetNamePies(PieceColor::RED, L'N')：\n";
    for (auto& piePtr : getNamePies(PieceColor::RED, L'马'))
        wss << piePtr->toString();

    wss << L"\ngetSeats(PieceColor::RED)：\n";
    for (auto id : { 0, 1, 2, 4, 6, 8, 10, 14, 18, 22, 27 }) {
        auto p = piePtrs[id];
        wss << p->chName() << L':';
        for (auto s : p->getSeats(PieceColor::RED))
            wss << s << L' ';
        wss << L"\n";
    }
    return wss.str();
}
