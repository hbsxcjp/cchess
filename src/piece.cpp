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
    : clr{ isalpha(_char)
            ? (islower(_char) ? PieceColor::BLACK : PieceColor::RED)
            : PieceColor::BLANK }
    , ch{ _char }
    , st{ nullSeat }
    , id{ curIndex++ }
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
    wss << setw(4) << index() << setw(5) << static_cast<int>(color()) << setw(6)
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
int Piece::curIndex{ -1 };
const wchar_t Piece::nullChar{ L'_' };
//NullPie Pieces::nullPiece{ NullPie(Piece::nullChar) }; // 空棋子
//const shared_ptr<Piece> Pieces::nullPiePtr{ &nullPiece }; // 空棋子指针
const shared_ptr<Piece> Pieces::nullPiePtr{ make_shared<NullPie>(Piece::nullChar) }; // 空棋子指针

// 一副棋子类
Pieces::Pieces()
    : piePtrs{
        shared_ptr<Piece>(make_shared<King>(L'K')),
        shared_ptr<Piece>(make_shared<King>(L'k')),
        shared_ptr<Piece>(make_shared<Advisor>(L'A')),
        shared_ptr<Piece>(make_shared<Advisor>(L'A')),
        shared_ptr<Piece>(make_shared<Advisor>(L'a')),
        shared_ptr<Piece>(make_shared<Advisor>(L'a')),
        shared_ptr<Piece>(make_shared<Bishop>(L'B')),
        shared_ptr<Piece>(make_shared<Bishop>(L'B')),
        shared_ptr<Piece>(make_shared<Bishop>(L'b')),
        shared_ptr<Piece>(make_shared<Bishop>(L'b')),
        shared_ptr<Piece>(make_shared<Knight>(L'N')),
        shared_ptr<Piece>(make_shared<Knight>(L'N')),
        shared_ptr<Piece>(make_shared<Knight>(L'n')),
        shared_ptr<Piece>(make_shared<Knight>(L'n')),
        shared_ptr<Piece>(make_shared<Rook>(L'R')),
        shared_ptr<Piece>(make_shared<Rook>(L'R')),
        shared_ptr<Piece>(make_shared<Rook>(L'r')),
        shared_ptr<Piece>(make_shared<Rook>(L'r')),
        shared_ptr<Piece>(make_shared<Cannon>(L'C')),
        shared_ptr<Piece>(make_shared<Cannon>(L'C')),
        shared_ptr<Piece>(make_shared<Cannon>(L'c')),
        shared_ptr<Piece>(make_shared<Cannon>(L'c')),
        shared_ptr<Piece>(make_shared<Pawn>(L'P')),
        shared_ptr<Piece>(make_shared<Pawn>(L'P')),
        shared_ptr<Piece>(make_shared<Pawn>(L'P')),
        shared_ptr<Piece>(make_shared<Pawn>(L'P')),
        shared_ptr<Piece>(make_shared<Pawn>(L'P')),
        shared_ptr<Piece>(make_shared<Pawn>(L'p')),
        shared_ptr<Piece>(make_shared<Pawn>(L'p')),
        shared_ptr<Piece>(make_shared<Pawn>(L'p')),
        shared_ptr<Piece>(make_shared<Pawn>(L'p')),
        shared_ptr<Piece>(make_shared<Pawn>(L'p'))
        /*
        make_shared<King>(L'K'),
        make_shared<King>(L'k'),
        make_shared<Advisor>(L'A'),
        make_shared<Advisor>(L'A'),
        make_shared<Advisor>(L'a'),
        make_shared<Advisor>(L'a'),
        make_shared<Bishop>(L'B'),
        make_shared<Bishop>(L'B'),
        make_shared<Bishop>(L'b'),
        make_shared<Bishop>(L'b'),
        make_shared<Knight>(L'N'),
        make_shared<Knight>(L'N'),
        make_shared<Knight>(L'n'),
        make_shared<Knight>(L'n'),
        make_shared<Rook>(L'R'),
        make_shared<Rook>(L'R'),
        make_shared<Rook>(L'r'),
        make_shared<Rook>(L'r'),
        make_shared<Cannon>(L'C'),
        make_shared<Cannon>(L'C'),
        make_shared<Cannon>(L'c'),
        make_shared<Cannon>(L'c'),
        make_shared<Pawn>(L'P'),
        make_shared<Pawn>(L'P'),
        make_shared<Pawn>(L'P'),
        make_shared<Pawn>(L'P'),
        make_shared<Pawn>(L'P'),
        make_shared<Pawn>(L'p'),
        make_shared<Pawn>(L'p'),
        make_shared<Pawn>(L'p'),
        make_shared<Pawn>(L'p'),
        make_shared<Pawn>(L'p')
        */
    }
{
}
/*
// 一副棋子类
Pieces::Pieces()
    : kings{ King(L'K'), King(L'k') }
    , advisors{ Advisor(L'A'), Advisor(L'A'),
        Advisor(L'a'), Advisor(L'a') }
    , bishops{ Bishop(L'B'), Bishop(L'B'), Bishop(L'b'), Bishop(L'b') }
    , knights{ Knight(L'N'), Knight(L'N'), Knight(L'n'), Knight(L'n') }
    , rooks{ Rook(L'R'), Rook(L'R'), Rook(L'r'), Rook(L'r') }
    , cannons{ Cannon(L'C'), Cannon(L'C'), Cannon(L'c'), Cannon(L'c') }
    , pawns{ Pawn(L'P'), Pawn(L'P'), Pawn(L'P'), Pawn(L'P'), Pawn(L'P'),
        Pawn(L'p'), Pawn(L'p'), Pawn(L'p'), Pawn(L'p'), Pawn(L'p') }
{
    for (auto& pie : kings)
        piePtrs.push_back(&pie);
    for (auto& pie : advisors)
        piePtrs.push_back(&pie);
    for (auto& pie : bishops)
        piePtrs.push_back(&pie);
    for (auto& pie : knights)
        piePtrs.push_back(&pie);
    for (auto& pie : rooks)
        piePtrs.push_back(&pie);
    for (auto& pie : cannons)
        piePtrs.push_back(&pie);
    for (auto& pie : pawns)
        piePtrs.push_back(&pie);
    Piece::curIndex = 0;
}
*/

inline const shared_ptr<Piece> Pieces::getKingPie(const PieceColor color)
{
    return piePtrs[color == PieceColor::RED ? 0 : 1];
}

inline const shared_ptr<Piece> Pieces::getOthPie(const shared_ptr<Piece> pie)
{
    int index{ pie->index() }, othIndex{};
    if (index < 2)
        othIndex = index == 1 ? 0 : 1; //帅0将1
    else if (index > 21)
        othIndex = index > 26 ? index - 5 : index + 5; // 兵22-26卒27-31
    else
        othIndex = index % 4 < 2 ? index - 2 : index + 2; // 士象马车炮，每种棋4子
    return piePtrs[othIndex];
}

const shared_ptr<Piece> Pieces::getFreePie(const wchar_t ch)
{
    if (ch != Piece::nullChar)
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
        L"index color wchar seat chName isBlank isKing isStronge\n"
    }; // index
    for (auto p : piePtrs)
        ws += p->toString();
    return ws + nullPiePtr->toString(); // 关注空棋子的特性!
}

const wstring Pieces::test()
{
    // Pieces apieces = Pieces();
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

    // wss << print_vector_int(getSeats(PieceColor::RED));
    return wss.str();
}
