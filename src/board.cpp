#include "board.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

Board::Board()
    : bottomColor{ PieceColor::RED }
    , pieces_{ __creatPieces() }
    , seats_{ __creatSeats() }
{
}

vector<shared_ptr<Seat>> Board::getSeats(const PieceColor color, const wchar_t name, const int col) const
{
    vector<shared_ptr<Seat>> someSeats{};
    for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) {
        const shared_ptr<Piece>& pie{ seat->piece() };
        if (pie && pie->color() == color
            && (name == L'\x00' || name == pie->name())
            && (col == -1 || col == seat->col()))
            someSeats.push_back(seat);
    });
    return someSeats;
}

//判断是否将军
const bool Board::isKilled(const PieceColor color)
{
    /*
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    int kingSeat{ pPieces->getKingPie(color)->seat() },
        othKingSeat{ pPieces->getKingPie(othColor)->seat() };
    if (isSameCol(kingSeat, othKingSeat)) {
        vector<shared_ptr<Seat>> ss{ getSameColSeats(kingSeat, othKingSeat) };
        if (std::all_of(ss.begin(), ss.end(),
                [this](const int s) { return isBlank(s); }))
            return true;
    }
    for (auto& ppie : pPieces->getLiveStrongePies(othColor)) {
        auto ss = ppie->filterMoveSeats(*this);
        if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
            return true;
    }
    */
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color)
{
    /*
    for (auto& ppie : pPieces->getLivePies(color))
        if (ppie->getCanMoveSeats(*this).size() > 0)
            return false;
            */
    return true;
}

const vector<shared_ptr<Piece>> Board::__creatPieces()
{
    vector<shared_ptr<Piece>> pieces{};
    wstring pieChars{ L"KAABBNNRRCCPPPPPkaabbnnrrccppppp" };
    for (auto& ch : pieChars)
        pieces.push_back(make_shared<Piece>(ch));
    return pieces;
}

vector<shared_ptr<Seat>> Board::__creatSeats()
{
    vector<shared_ptr<Seat>> seats{};
    for (int r = 0; r < RowNum; ++r)
        for (int c = 0; c < ColNum; ++c)
            seats.push_back(make_shared<Seat>(static_cast<char>(r), static_cast<char>(c)));
    return seats;
}

const wstring Board::getChars() const
{
    wstring chars{};
    for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) { chars += seat->piece() ? seat->piece()->ch() : nullChar; });
    return chars;
}

void Board::putPieces(const wstring& chars)
{
    function<const shared_ptr<Piece>&(wchar_t)>
        __getFreePie = [&](wchar_t ch) {
            for (auto& pie : pieces_)
                if (pie->ch() == ch)
                    if (all_of(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat> seat) { return seat->piece() != pie; }))
                        return pie;
            return pieces_[0]; // 这一步不应该被执行，只是为满足编译不报警而已
        };

    for (int id = chars.size(); id >= 0; --id) {
        wchar_t ch = chars[id];
        if (ch != nullChar)
            getSeat(id / 9, id % 9)->put(__getFreePie(ch));
    }
    setBottomSide();
}

void Board::setBottomSide()
{ //kingPiece->getSeats(PieceColor::RED)
    //for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) { if(shared_ptr<Piece>& pie = seat->piece()) ? pie->ch() : nullChar; });
    //bottomColor = getKingPie(PieceColor::RED)->seat() < 45 ? PieceColor::RED : PieceColor::BLACK;
}

shared_ptr<Seat>& Board::getOthSeat(const shared_ptr<Seat>& seat, const ChangeType ct)
{
    if (ct == ChangeType::ROTATE) {
        int index{ 0 };
        for (auto& st : seats_) {
            if (seat == st)
                break;
            ++index;
        }
        return seats_[seats_.size() - index - 1];
    } else
        return getSeat(seat->row(), ColNum - seat->col());
}

void Board::changeSide(const ChangeType ct)
{
    function<const shared_ptr<Piece>&(const shared_ptr<Piece>&)>
        __getOthPie = [&](const shared_ptr<Piece>& piece) {
            if (!piece)
                return piece;
            int index{ 0 };
            for (auto& pie : pieces_) {
                if (pie == piece)
                    break;
                ++index;
            }
            return pieces_[(index + 16) % 32];
        };

    if (ct == ChangeType::EXCHANGE)
        for_each(seats_.begin(), seats_.end(), [&](shared_ptr<Seat>& seat) { seat->put(__getOthPie(seat->piece())); });
    else
        for_each(seats_.begin(), seats_.end(), [&](shared_ptr<Seat>& seat) { getOthSeat(seat, ct)->put(seat->piece()); });
    setBottomSide();
}

/*
void Board::set(vector<pair<shared_ptr<Seat>, shared_ptr<Piece>>> seatPieces)
{
    for (auto& seatPie : seatPieces)
        seatPie.first->put(seatPie.second);
    setBottomSide();
}*/

const wstring Board::getIccs(const Move& move) const
{
    wstringstream wss{};
    wstring ColChars{ L"abcdefghi" };
    wss << ColChars[move.fseat()->col()] << move.fseat()->row() << ColChars[move.tseat()->col()] << move.tseat()->row();
    return wss.str();
}

//(fseat, tseat)->中文纵线着法
const wstring Board::getZh(const Move& move)
{
    auto __find_index = [](const vector<shared_ptr<Seat>>& seats, const shared_ptr<Seat>& seat) {
        int index{ 0 };
        for (auto st : seats) {
            if (seat == st)
                break;
            ++index;
        }
        return index;
    };

    wstringstream wss{};
    const shared_ptr<Seat>&fseat{ move.fseat() }, &tseat{ move.tseat() };
    const int fromRow{ fseat->row() }, fromCol{ fseat->col() };
    //int fseat{ move.fseat() }, tseat{ move.tseat() }, fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    const shared_ptr<Piece>& fromPiece{ fseat->piece() };
    //shared_ptr<Piece> fromPiece{ getPiece(fseat) };
    const PieceColor color{ fromPiece->color() };
    const wchar_t name{ fromPiece->name() };
    vector<shared_ptr<Seat>> seats{ getSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };

    if (length > 1 && wstring(L"马车炮兵卒").find(name) != wstring::npos) {
        if (name == L'兵' || name == L'卒') {
            //seats = sortPawnSeats(isBottomSide(color), getSeats(color, name));
            //seats = sortPawnSeats(isBottomSide(color), getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide(color)) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[__find_index(seats, fseat)] << name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << __numChars[color][isBottomSide(color) ? ColNum - fromCol - 1 : fromCol];

    int toRow{ tseat->row() };
    //int toRow{ getRow(tseat) };
    //wcout << (toRow == fromRow ? L'平' : (isBottomSide(color) == (toRow > fromRow) ? L'进' : L'退')) << endl;

    wss << (toRow == fromRow ? L'平' : (isBottomSide(color) == (toRow > fromRow) ? L'进' : L'退'))
        << __numChars[color][(wstring(L"帅车炮兵将卒").find(name) != wstring::npos) && toRow != fromRow
                   ? (toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1)
                   : (isBottomSide(color) ? ColNum - tseat->col() - 1 : tseat->col())];
    //: (isBottomSide(color) ? MaxCol - getCol(tseat) : getCol(tseat))];
    return wss.str();
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::getMoveSeats(const Move& move, const RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::ICCS:
        return __getSeatFromICCS(move.iccs());
    default:
        //case RecFormat::ZH:
        //case RecFormat::CC:
        return __getSeatFromZh(move.zh());
    }
}

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
    map<wchar_t, wchar_t> rcpName{
        { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' }
    };
    auto __getName = [&](Piece& pie) {
        wchar_t name = pie.name();
        return (pie.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end())
            ? rcpName[name]
            : name;
    };
    for (auto& seat : seats_)
        textBlankBoard[(9 - seat->row()) * 2 * 18 + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromICCS(const wstring& ICCS)
{
    string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97), getSeat(iccs[3] - 48, iccs[2] - 97)); // 0:48, a:97
}

//中文纵线着法->(fseat, tseat)
const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromZh(const wstring& zhStr)
{
    int index{};
    shared_ptr<Seat> fseat{}, tseat{};
    vector<shared_ptr<Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ __numChars[PieceColor::RED].find(zhStr.back()) != wstring::npos
            ? PieceColor::RED
            : PieceColor::BLACK };
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](const wchar_t ch) { return static_cast<int>(__numChars[color].find(ch)) + 1; };
    auto __getCol = [&](const int num) { return isBottomSide(color) ? ColNum - num : num - 1; };
    static map<wchar_t, int> __getIndex{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
        { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
        { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
    /*
    auto __getIndex = [](const wchar_t ch) {
        static map<wchar_t, int> ChNum_Indexs{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
            { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
            { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
        return ChNum_Indexs[ch];
    };*/

    if (wstring(L"帅仕相马车炮兵将士象卒").find(name) != wstring::npos) {
        seats = getSeats(color, name, __getCol(__getNum(zhStr[1])));
        //seats = getSideNameColSeats(color, name, __getCol(__getNum(zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && (wstring(L"仕相士象").find(name) != wstring::npos) && (zhStr[2] == L'退') == isBottomSide(color))
            ? seats.size() - 1
            : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = __getIndex[zhStr[0]];
        //index = __getIndex(zhStr[0]);
        name = zhStr[1];
        seats = getSeats(color, name);
        //seats = getSideNameSeats(color, name);

        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << toString();

        if (name == L'兵' || name == L'卒') {
            //seats = sortPawnSeats(isBottomSide(color), seats);
            //seats = sortPawnSeats(isBottomSide(color), seats);
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
    int movDir{ __getIndex[zhStr[2]] * (isBottomSide(color) ? 1 : -1) },
        //int movDir{ __getIndex(zhStr[2]) * (isBottomSide(color) ? 1 : -1) },
        num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    if (wstring(L"帅车炮兵将卒").find(name) != wstring::npos) {
        //#'获取直线走子toseat'
        int row = fseat->row();
        //int row = getRow(fseat);
        tseat = (movDir == 0) ? getSeat(row, toCol) : getSeat(row + movDir * num, fseat->col());
        //tseat = (movDir == 0) ? getSeat(row, toCol) : getSeat(row + movDir * num, getCol(fseat));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step{ abs(toCol - fseat->col()) }; //  # 相距1或2列
        //int step{ abs(toCol - getCol(fseat)) }; //  # 相距1或2列
        int inc{ ((wstring(L"仕相士象").find(name) != wstring::npos)) ? step : (step == 1 ? 2 : 1) };
        tseat = getSeat(fseat->row() + movDir * inc, toCol);
        //tseat = getSeat(getRow(fseat) + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

map<PieceColor, wstring> Board::__numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};

/*
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
*/