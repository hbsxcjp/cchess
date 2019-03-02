#include "board.h"
#include "board_base.h"
#include "move.h"
#include "piece.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
using namespace std;
using namespace Board_base;

Board::Board()
    : bottomColor(PieceColor::RED)
    , pPieces(make_shared<Pieces>())
    , pieSeats(vector<shared_ptr<Piece>>(RowNum * ColNum, Pieces::nullPiePtr))
{
}

Board::Board(const wstring& pieceChars)
    : Board()
{
    for (auto seat : allSeats)
        __setPiece(pPieces->getFreePie(pieceChars[seat]), seat);
    if (pPieces->getKingPie(PieceColor::RED)->seat() > 45)
        bottomColor = PieceColor::BLACK;
}

shared_ptr<Piece> Board::getPiece(const int seat) { return pieSeats[seat]; }

shared_ptr<Piece> Board::getOthPie(shared_ptr<Piece> piecep) { return pPieces->getOthPie(piecep); }

vector<shared_ptr<Piece>> Board::getLivePies() { return pPieces->getLivePies(); }

const bool Board::isBlank(const int seat) { return getPiece(seat)->isBlank(); }

const PieceColor Board::getColor(const int seat) { return getPiece(seat)->color(); }

vector<int> Board::getSideNameSeats(const PieceColor color, const wchar_t name)
{
    return __getSeats(pPieces->getNamePies(color, name));
}

vector<int> Board::getSideNameColSeats(const PieceColor color, const wchar_t name, const int col)
{
    return __getSeats(pPieces->getNameColPies(color, name, col));
}

vector<int> Board::__getSeats(vector<shared_ptr<Piece>> pies)
{
    vector<int> res{};
    for_each(pies.begin(), pies.end(), [&](shared_ptr<Piece>& ppie) { res.push_back(ppie->seat()); });
    std::sort(res.begin(), res.end());
    return res;
}

//判断是否将军
const bool Board::isKilled(const PieceColor color)
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    int kingSeat{ pPieces->getKingPie(color)->seat() },
        othKingSeat{ pPieces->getKingPie(othColor)->seat() };
    if (isSameCol(kingSeat, othKingSeat)) {
        vector<int> ss{ getSameColSeats(kingSeat, othKingSeat) };
        if (std::all_of(ss.begin(), ss.end(),
                [this](const int s) { return isBlank(s); }))
            return true;
    }
    for (auto& ppie : pPieces->getLiveStrongePies(othColor)) {
        auto ss = ppie->getFilterMoveSeats(*this);
        if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
            return true;
    }
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color)
{
    for (auto& ppie : pPieces->getLivePies(color))
        if (ppie->getCanMoveSeats(*this).size() > 0)
            return false;
    return true;
}

void Board::go(Move& move) { move.setEatPiece(move_go(move.fseat(), move.tseat())); }

void Board::back(Move& move) { move_back(move.fseat(), move.tseat(), move.eatPiece()); }

shared_ptr<Piece> Board::move_go(const int fseat, const int tseat)
{
    shared_ptr<Piece> eatPiece = pieSeats[tseat];
    eatPiece->setSeat(nullSeat);
    __setPiece(getPiece(fseat), tseat);
    pieSeats[fseat] = Pieces::nullPiePtr;
    return eatPiece;
}

void Board::move_back(const int fseat, const int tseat, shared_ptr<Piece> eatPiece)
{
    __setPiece(getPiece(tseat), fseat);
    __setPiece(eatPiece, tseat);
}

void Board::__setPiece(shared_ptr<Piece> ppie, const int tseat)
{
    ppie->setSeat(tseat);
    pieSeats[tseat] = ppie;
}

const wstring Board::getPieceChars()
{
    wstring pieceChars{};
    for (int row = MaxRow; row >= MinRow; --row) {
        for (int col = MinCol; col <= MaxCol; ++col)
            pieceChars += getPiece(getSeat(row, col))->wchar();
        if (row != MinRow)
            pieceChars += L'/';
    }
    return pieceChars;
}

void Board::setSeatPieces(vector<pair<int, shared_ptr<Piece>>> seatPieces)
{
    for (auto& stPie : seatPieces)
        __setPiece(stPie.second, stPie.first);
    bottomColor = getRow(pPieces->getKingPie(PieceColor::RED)->seat()) < 3 ? PieceColor::RED : PieceColor::BLACK;
}

const wstring Board::toString()
{
    // 文本空棋盘
    wstring textBlankBoard{ L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
                            "┃　│　│　│╲│╱│　│　│　┃\n"
                            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                            "┃　│　│　│╱│╲│　│　│　┃\n"
                            "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                            "┃　　　　　　　　　　　　　　　┃\n"
                            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                            "┃　│　│　│╲│╱│　│　│　┃\n"
                            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                            "┃　│　│　│╱│╲│　│　│　┃\n"
                            "┗━┷━┷━┷━┷━┷━┷━┷━┛\n" }; // 边框粗线
    auto getName = [](Piece& pie) {
        map<wchar_t, wchar_t> rcpName{
            { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' }
        };
        wchar_t name = pie.chName();
        return (pie.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end())
            ? rcpName[name]
            : name;
    };
    for (auto& ppie : pPieces->getLivePies())
        textBlankBoard[(9 - getRow(ppie->seat())) * 2 * 18 + getCol(ppie->seat()) * 2] = getName(*ppie);
    return textBlankBoard + pPieces->toString();
}

const wstring Board::test()
{
    wstringstream wss{};
    wss << L"test "
           L"board.h\n----------------------------------------------------"
           L"-\n";
    wss << L"Board::toString():\n"
        << toString();
    wss << L"Piece::getCanMoveSeats():\n";

    for (auto& ppie : pPieces->getLivePies()) {
        wss << ppie->chName() << L' ' << ppie->wchar() << L'_' << setw(2)
            << ppie->seat() << L'：';
        for (auto s : ppie->getCanMoveSeats(*this))
            wss << setw(2) << s << L' ';
        wss << L'\n';
    }
    wss << pPieces->toString();

    return wss.str();
}
