#include "board.h"
#include "board_base.h"
#include "chessInstanceIO.h"
#include "move.h"
#include "piece.h"
#include "pieces.h"
#include "tools.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
using namespace std;
using namespace Tools;
using namespace Board_base;

Board::Board()
    : bottomColor(PieceColor::RED)
    , pPieces(make_shared<Pieces>())
    , pieSeats(vector<shared_ptr<Piece>>(RowNum * ColNum, Pieces::nullPiePtr))
{
}

shared_ptr<Piece> Board::getOthPie(const shared_ptr<Piece>& piecep) const { return pPieces->getOthPie(piecep); }

vector<shared_ptr<Piece>> Board::getLivePies() const { return pPieces->getLivePies(); }

const bool Board::isBlank(const int seat) const { return getPiece(seat)->isBlank(); }

const PieceColor Board::getColor(const int seat) const { return getPiece(seat)->color(); }

vector<int> Board::getSideNameSeats(const PieceColor color, const wchar_t name) const
{
    return __getSeats(pPieces->getNamePies(color, name));
}

vector<int> Board::getSideNameColSeats(const PieceColor color, const wchar_t name, const int col) const
{
    return __getSeats(pPieces->getNameColPies(color, name, col));
}

const vector<int> Board::__getSeats(const vector<shared_ptr<Piece>>& pies) const
{
    vector<int> seats{};
    for_each(pies.begin(), pies.end(), [&](const shared_ptr<Piece>& ppie) { seats.push_back(ppie->seat()); });
    std::sort(seats.begin(), seats.end());
    return seats;
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
        auto ss = ppie->filterMoveSeats(*this);
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

void Board::go(Move& move) { move.setEatPiece(go(move.fseat(), move.tseat())); }

shared_ptr<Piece> Board::go(const int fseat, const int tseat)
{
    shared_ptr<Piece> eatPiece = pieSeats[tseat];
    eatPiece->setSeat(nullSeat);
    __setPiece(getPiece(fseat), tseat);
    pieSeats[fseat] = Pieces::nullPiePtr;
    return eatPiece;
}

void Board::back(Move& move) { back(move.fseat(), move.tseat(), move.eatPiece()); }

void Board::back(const int fseat, const int tseat, shared_ptr<Piece> eatPiece)
{
    __setPiece(getPiece(tseat), fseat);
    __setPiece(eatPiece, tseat);
}

void Board::__setPiece(shared_ptr<Piece> ppie, const int tseat)
{
    ppie->setSeat(tseat);
    pieSeats[tseat] = ppie;
}

const wstring Board::getPieceChars() const
{
    wstringstream wss{};
    for (int row = MaxRow; row >= MinRow; --row) {
        for (int col = MinCol; col <= MaxCol; ++col)
            wss << getPiece(getSeat(row, col))->wchar();
        wss << L'/';
    }
    wstring pieceChars{ wss.str() };
    pieceChars.erase(pieceChars.size() - 1);
    return pieceChars;
}

void Board::setBottomSide()
{
    bottomColor = pPieces->getKingPie(PieceColor::RED)->seat() < 45 ? PieceColor::RED : PieceColor::BLACK;
}

void Board::set(vector<pair<int, shared_ptr<Piece>>> seatPieces)
{
    for (auto& stPie : seatPieces)
        __setPiece(stPie.second, stPie.first);
    setBottomSide();
}

void Board::set(const wstring& pieceChars)
{
    shared_ptr<Piece> pp;
    for (auto seat : allSeats)
        if ((pp = pPieces->getFreePie(pieceChars[seat])) != Pieces::nullPiePtr)
            __setPiece(pp, seat);
    setBottomSide();
}

const wstring Board::getIccs(const Move& move)
{
    wstringstream wss{};
    wstring ColChars{ L"abcdefghi" };
    wss << ColChars[getCol(move.fseat())] << getRow(move.fseat()) << ColChars[getCol(move.tseat())] << getRow(move.tseat());
    return wss.str();
}

//(fseat, tseat)->中文纵线着法
const wstring Board::getZh(const Move& move)
{
    auto __find_index = [](const vector<int>& seats, const int seat) {
        int index{ 0 };
        for (auto st : seats) {
            if (seat == st)
                break;
            ++index;
        }
        return index;
    };

    wstringstream wss{};
    int fseat{ move.fseat() }, tseat{ move.tseat() }, fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    shared_ptr<Piece> fromPiece{ getPiece(fseat) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    vector<int> seats{ getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };

    if (length > 1 && wstring(L"马车炮兵卒").find(name) != wstring::npos) {
        if (name == L'兵' || name == L'卒') {
            seats = sortPawnSeats(isBottomSide(color),
                getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide(color)) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[__find_index(seats, fseat)] << name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << numChars[color][isBottomSide(color) ? MaxCol - fromCol : fromCol];

    int toRow{ getRow(tseat) };
    //wcout << (toRow == fromRow ? L'平' : (isBottomSide(color) == (toRow > fromRow) ? L'进' : L'退')) << endl;

    wss << (toRow == fromRow ? L'平' : (isBottomSide(color) == (toRow > fromRow) ? L'进' : L'退'))
        << numChars[color][(wstring(L"帅车炮兵将卒").find(name) != wstring::npos) && toRow != fromRow
                   ? (toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1)
                   : (isBottomSide(color) ? MaxCol - getCol(tseat) : getCol(tseat))];
    return wss.str();
}

const pair<int, int> Board::getSeats(const Move& move, RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::ICCS:
        return __getSeatFromICCS(move.iccsStr());
    default:
        //case RecFormat::ZH:
        //case RecFormat::CC:
        return __getSeatFromZh(move.zhStr());
    }
}

const pair<int, int> Board::__getSeatFromICCS(const wstring& ICCS)
{
    string iccs{ ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97), getSeat(iccs[3] - 48, iccs[2] - 97));
}

//中文纵线着法->(fseat, tseat)
const pair<int, int> Board::__getSeatFromZh(const wstring& zhStr)
{
    int index, fseat, tseat;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ numChars[PieceColor::RED].find(zhStr.back()) != wstring::npos
            ? PieceColor::RED
            : PieceColor::BLACK };
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](const wchar_t ch) { return static_cast<int>(numChars[color].find(ch)) + 1; };
    auto __getCol = [&](const int num) { return isBottomSide(color) ? ColNum - num : num - 1; };
    auto __getIndex = [](const wchar_t ch) {
        static map<wchar_t, int> ChNum_Indexs{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
            { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
            { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
        return ChNum_Indexs[ch];
    };

    if (wstring(L"帅仕相马车炮兵将士象卒").find(name) != wstring::npos) {
        seats = getSideNameColSeats(color, name, __getCol(__getNum(zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && (wstring(L"仕相士象").find(name) != wstring::npos) && (zhStr[2] == L'退') == isBottomSide(color))
            ? seats.size() - 1
            : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = __getIndex(zhStr[0]);
        name = zhStr[1];
        seats = getSideNameSeats(color, name);

        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << toString();

        if (name == L'兵' || name == L'卒') {
            seats = sortPawnSeats(isBottomSide(color), seats);
            //#获取多兵的列
            if (seats.size() == 3 && zhStr[0] == L'后')
                index += 1;
        } else {
            if (isBottomSide(color)) //# 修正index
                index = seats.size() - index - 1;
        }
    }
    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ __getIndex(zhStr[2]) * (isBottomSide(color) ? 1 : -1) },
        num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    if (wstring(L"帅车炮兵将卒").find(name) != wstring::npos) {
        //#'获取直线走子toseat'
        int row = getRow(fseat);
        tseat = (movDir == 0) ? getSeat(row, toCol) : getSeat(row + movDir * num, getCol(fseat));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step{ abs(toCol - getCol(fseat)) }; //  # 相距1或2列
        int inc{ ((wstring(L"仕相士象").find(name) != wstring::npos)) ? step : (step == 1 ? 2 : 1) };
        tseat = getSeat(getRow(fseat) + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

map<PieceColor, wstring> Board::numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};

const wstring Board::toString() const
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
