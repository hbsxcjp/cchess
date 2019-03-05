#include "pieces.h"
#include "board.h"
#include "board_base.h"
#include "piece.h"
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
using namespace std;
using namespace Board_base;

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

const shared_ptr<Piece> Pieces::getKingPie(const PieceColor color) const { return piePtrs[color == PieceColor::RED ? 0 : 16]; }

const shared_ptr<Piece> Pieces::getOthPie(const shared_ptr<Piece> pie) const
{
    int index{ 0 };
    for (auto& pm : piePtrs) {
        if (pm == pie)
            break;
        ++index;
    }
    return piePtrs[index < 16 ? 16 + index : index - 16]; 
}

const shared_ptr<Piece> Pieces::getFreePie(const wchar_t ch) const
{
    if (ch == Piece::nullChar)
        return nullPiePtr;
    for (auto& pp : piePtrs)
        if (pp->wchar() == ch && pp->seat() == nullSeat)
            return pp;
    return nullPiePtr;
}

const vector<shared_ptr<Piece>> Pieces::getLivePies() const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) { return p->seat() != nullSeat; });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getLivePies(const PieceColor color) const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) { return p->seat() != nullSeat && p->color() == color; });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getLiveStrongePies(const PieceColor color) const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) { return p->seat() != nullSeat && p->color() == color && p->isStronge(); });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getNamePies(const PieceColor color, wchar_t name) const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) {
        return p->seat() != nullSeat && p->color() == color && p->chName() == name;
    });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getNameColPies(const PieceColor color, wchar_t name,
    int col) const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) {
        return p->seat() != nullSeat && p->color() == color && p->chName() == name && getCol(p->seat()) == col;
    });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getEatedPies() const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) { return p->seat() == nullSeat; });
    return res;
}

const vector<shared_ptr<Piece>> Pieces::getEatedPies(const PieceColor color) const
{
    vector<shared_ptr<Piece>> res{};
    copy_if(piePtrs.begin(), piePtrs.end(), back_inserter(res), [&](const shared_ptr<Piece>& p) { return p->seat() == nullSeat && p->color() == color; });
    return res;
}

void Pieces::clearSeat()
{
    for_each(piePtrs.begin(), piePtrs.end(), [&](shared_ptr<Piece>& p) { p->setSeat(nullSeat); });
}

// 类外初始化类内静态const成员
const shared_ptr<Piece> Pieces::nullPiePtr{ make_shared<NullPie>(Piece::nullChar) }; // 空棋子指针

const wstring Pieces::toString() const
{
    wstringstream wss{};
    wss << L"color wchar seat chName isBlank isKing isStronge\n";
    for (auto p : piePtrs)
        wss << p->toString();
    wss << nullPiePtr->toString();
    return wss.str(); // 关注空棋子的特性!
}

const wstring Pieces::test() const
{
    wstringstream wss{};
    wss << L"test "
           L"piece.h\n-----------------------------------------------------\n";

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
