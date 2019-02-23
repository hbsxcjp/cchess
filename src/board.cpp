#include "board.h"
#include "info.h"
#include "move.h"
#include "piece.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;
using namespace Board_base;

Board::Board() // 定义时不要指定默认实参
    : bottomColor{ PieceColor::red }
    , pieces{ Pieces() }
{
    pieSeats.resize(RowNum * ColNum);
}

Board::Board(Info& info) // 定义时不要指定默认实参
    : Board()
{
    setFrom(info);
}

vector<int> Board::getSideNameSeats(PieceColor color, wchar_t name)
{
    return __getSeats(pieces.getNamePies(color, name));
}

vector<int> Board::getSideNameColSeats(PieceColor color, wchar_t name, int col)
{
    return __getSeats(pieces.getNameColPies(color, name, col));
}

vector<int> Board::__getSeats(vector<Piece*> pies)
{
    vector<int> res{};
    for_each(pies.begin(), pies.end(), [&](Piece*& p) { res.push_back(p->seat()); });
    std::sort(res.begin(), res.end());
    return res;
}

//判断是否将军
bool Board::isKilled(PieceColor color)
{
    PieceColor othColor = color == PieceColor::black ? PieceColor::red : PieceColor::black;
    int kingSeat{ pieces.getKingPie(color)->seat() },
        othKingSeat{ pieces.getKingPie(othColor)->seat() };
    if (isSameCol(kingSeat, othKingSeat)) {
        vector<int> ss{ getSameColSeats(kingSeat, othKingSeat) };
        if (std::all_of(ss.begin(), ss.end(),
                [this](int s) { return isBlank(s); }))
            return true;
    }
    for (auto pie : pieces.getLiveStrongePies(othColor)) {
        auto ss = pie->getFilterMoveSeats(*this);
        if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
            return true;
    }
    return false;
}

//判断是否被将死
bool Board::isDied(PieceColor color)
{
    for (auto pie : pieces.getLivePies(color))
        if (pie->getCanMoveSeats(*this).size() > 0)
            return false;
    return true;
}

void Board::go(Move& move) { move.setEatPiece(move_go(move.fseat(), move.tseat())); }

void Board::back(Move& move) { move_back(move.fseat(), move.tseat(), move.eatPiece()); }

Piece* Board::move_go(int fseat, int tseat)
{
    Piece* eatPiece = pieSeats[tseat];
    eatPiece->setSeat(nullSeat);
    __setPiece(getPiece(fseat), tseat);
    pieSeats[fseat] = Pieces::nullPiePtr;
    //__setPiece(Pieces::nullPiePtr, fseat);
    return eatPiece;
}

void Board::move_back(int fseat, int tseat, Piece* eatPiece)
{
    __setPiece(getPiece(tseat), fseat);
    __setPiece(eatPiece, tseat);
}

void Board::__setPiece(Piece* pie, int tseat)
{
    pie->setSeat(tseat);
    pieSeats[tseat] = pie;
}

void Board::setFEN(Info& info)
{
    wstring pieceChars{};
    for (int row = MaxRow; row >= MinRow; --row) {
        for (int col = MinCol; col <= MaxCol; ++col)
            pieceChars += getPiece(getSeat(row, col))->wchar();
        if (row != MinRow)
            pieceChars += L'/';
    }
    info.setFEN(pieceChars);
}

void Board::setFrom(Info& info)
{
    wstring chars{ info.getPieChars() };
    pieces.clear();
    std::fill(pieSeats.begin(), pieSeats.end(), Pieces::nullPiePtr);
    for (int s = 0; s != 90; ++s)
        __setPiece(pieces.getFreePie(chars[s]), s);
    bottomColor = pieces.getKingPie(PieceColor::red)->seat() < 45
        ? PieceColor::red
        : PieceColor::black;
}

void Board::setSeatPieces(vector<pair<int, Piece*>> seatPieces)
{
    for (auto& stPie : seatPieces)
        __setPiece(stPie.second, stPie.first);
    bottomColor = getRow(pieces.getKingPie(PieceColor::red)->seat()) < 3 ? PieceColor::red : PieceColor::black;
}

const wstring Board::toString()
{
    auto getName = [](Piece& p) {
        map<wchar_t, wchar_t> rcpName{
            { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' }
        };
        wchar_t name = p.chName();
        return (p.color() == PieceColor::black && rcpName.find(name) != rcpName.end())
            ? rcpName[name]
            : name;
    };
    wstring res{ Board_base::TextBlankBoard };
    for (auto p : pieces.getLivePies())
        res[(9 - getRow(p->seat())) * 2 * 18 + getCol(p->seat()) * 2] = getName(*p);
    return res;
}

wstring Board::test()
{
    wstringstream wss{};
    wss << L"test "
           L"board.h\n----------------------------------------------------"
           L"-\n";
    //wss << L"setFEN():" << setFEN() << L'\n';
    wss << L"Board::toString():\n"
        << toString();
    wss << L"Piece::getCanMoveSeats():\n";

    for (auto p : pieces.getLivePies()) {
        wss << p->chName() << L' ' << p->wchar() << L'_' << setw(2)
            << p->seat() << L'：';
        for (auto s : p->getCanMoveSeats(*this))
            wss << setw(2) << s << L' ';
        wss << L'\n';
    }
    wss << pieces.toString();
    //wss << pieces.test();

    wss << L"Board::changeSide():" << L'\n';

    return wss.str();
}
