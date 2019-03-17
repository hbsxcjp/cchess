#include "board.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <assert.h>
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

vector<shared_ptr<Seat>> Board::getLiveSeats(const PieceColor color, const wchar_t name, const int col) const
{
    vector<shared_ptr<Seat>> someSeats{};
    for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) {
        const shared_ptr<Piece>& pie{ seat->piece() };
        if (((color == PieceColor::BLANK) != (color == pie->color())) // 空则两方棋子全选
            && (name == L'\x00' || name == pie->name()) // 空则各种棋子全选
            && (col == -1 || col == seat->col())) // -1则各列棋子全选
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
            seats.push_back(make_shared<Seat>(r, c, Board::nullPiece));
    return seats;
}

void Board::putPieces(const wstring& chars)
{
    function<const shared_ptr<Piece>&(wchar_t)>
        __getFreePie = [&](wchar_t ch) {
            if (ch == Board::__nullChar)
                return Board::nullPiece;
            for (auto& pie : pieces_)
                if (pie->ch() == ch)
                    if (all_of(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat> seat) { return seat->piece() != pie; }))
                        return pie;
            return Board::nullPiece; // 这一步不应该被执行，只是为满足编译不报警而已
        };

    if (seats_.size() != chars.size())
        cout << "错误：seats_.size() != chars.size()";

    for (int index = seats_.size() - 1; index <= 0; --index)
        seats_[index]->put(__getFreePie(chars[index]));
    setBottomSide();
}

void Board::setBottomSide()
{ //kingPiece->getLiveSeats(PieceColor::RED)
    //for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) { if(shared_ptr<Piece>& pie = seat->piece()) ? pie->ch() : __nullChar; });
    //bottomColor = getKingPie(PieceColor::RED)->seat() < 45 ? PieceColor::RED : PieceColor::BLACK;
}

shared_ptr<Seat>& Board::getOthSeat(const shared_ptr<Seat>& seat, const ChangeType ct)
{
    if (ct == ChangeType::ROTATE) // 旋转
        return seats_[seats_.size() - distance(seats_.begin(), find(seats_.begin(), seats_.end(), seat)) - 1];
    else // ChangeType::SYMMETRY 对称
        return getSeat(seat->row(), ColNum - seat->col());
}

void Board::changeSide(const ChangeType ct)
{
    if (ct == ChangeType::EXCHANGE) // 交换红黑方
        for_each(seats_.begin(), seats_.end(),
            [&](shared_ptr<Seat>& seat) { seat->put(seat->piece() == Board::nullPiece
                                                  ? Board::nullPiece
                                                  : pieces_[(distance(pieces_.begin(), find(pieces_.begin(), pieces_.end(), seat->piece())) + 16) % 32]); });
    else // 旋转或对称
        for_each(seats_.begin(), seats_.end(), [&](shared_ptr<Seat>& seat) { seat->put(getOthSeat(seat, ct)->piece()); });
    setBottomSide();
}

const wstring Board::getIccs(const Move& move) const
{
    wstringstream wss{};
    wstring ColChars{ L"abcdefghi" };
    wss << ColChars[move.fseat()->col()] << move.fseat()->row() << ColChars[move.tseat()->col()] << move.tseat()->row();
    return wss.str();
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::getMoveSeats(const int frowcol, const int trowcol)
{
    return make_pair(getSeat(frowcol), getSeat(trowcol));
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::getMoveSeats(const Move& move, const RecFormat fmt)
{
    if (fmt == RecFormat::ICCS)
        return __getSeatFromICCS(move.iccs());
    else //case RecFormat::ZH: //case RecFormat::CC:
        return __getSeatFromZh(move.zh());
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
    auto __getName = [&](const Piece& pie) {
        wchar_t name = pie.name();
        return ((pie.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end())
                ? rcpName[name]
                : name);
    };
    for (auto& seat : getLiveSeats())
        textBlankBoard[(ColNum - seat->row()) * 2 * (ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

//(fseat, tseat)->中文纵线着法
const wstring Board::getZh(const Move& move)
{
    wstringstream wss{};
    const shared_ptr<Seat>&fseat{ move.fseat() }, &tseat{ move.tseat() };
    const shared_ptr<Piece>& fromPiece{ fseat->piece() };
    const PieceColor color{ fromPiece->color() };
    const wchar_t name{ fromPiece->name() };
    const int fromRow{ fseat->row() }, fromCol{ fseat->col() }, toRow{ tseat->row() }, toCol{ tseat->col() };
    vector<shared_ptr<Seat>> seats{ getLiveSeats(color, name, fromCol) };
    bool isSameRow{ toRow == fromRow }, isBottom{ isBottomSide(color) };
    int length = seats.size();
    auto __getNumChar = [&](int col) { return __numChars[color][isBottom ? ColNum - col - 1 : col]; };

    if (length > 1 && fromPiece->isStronge()) {
        if (fromPiece->isPawn()) {
            seats = __sortPawnSeats(color, name);
            length = seats.size();
        } else if (isBottom) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[int(distance(seats.begin(), find(seats.begin(), seats.end(), fseat)))] << name;
    } else //#仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << __getNumChar(fromCol);
    //wss << name << __numChars[color][isBottom ? ColNum - fromCol - 1 : fromCol];

    //wcout << (toRow == fromRow ? L'平' : (isBottom == (toRow > fromRow) ? L'进' : L'退')) << endl;
    wss << (isSameRow ? L'平' : (isBottom == (toRow > fromRow) ? L'进' : L'退'))
        << ((fromPiece->isLineMove() && !isSameRow)
                   ? __numChars[color][abs(fromRow - toRow) - 1]
                   : __getNumChar(toCol));
    //: (isBottom ? MaxCol - getCol(tseat) : getCol(tseat))];

    wcout << wss.str() << endl;

    return wss.str();
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromICCS(const wstring& ICCS)
{
    string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97), getSeat(iccs[3] - 48, iccs[2] - 97)); // 0:48, a:97
}

// '多兵排序'
vector<shared_ptr<Seat>> Board::__sortPawnSeats(const PieceColor color, const wchar_t name)
{
    map<int, vector<shared_ptr<Seat>>> colSeats{};
    vector<shared_ptr<Seat>> seats(5);
    auto liveSeats = getLiveSeats(color, name);
    // 按列建立字典，按列排序
    for_each(liveSeats.begin(), liveSeats.end(),
        [&](const shared_ptr<Seat>& seat) { colSeats[seat->col()].push_back(seat); });
    // 整合成一个数组
    auto pos = seats.begin();
    for_each(colSeats.begin(), colSeats.end(), [&](pair<int, vector<shared_ptr<Seat>>> colSeat) {
        if (colSeat.second.size() > 1) { // 筛除只有一个位置的列
            std::sort(colSeat.second.begin(), colSeat.second.end()); // 每列排序
            pos = copy(colSeat.second.begin(), colSeat.second.end(), pos); //按列存入
        }
    });
    seats = vector<shared_ptr<Seat>>{ seats.begin(), pos };
    if (isBottomSide(color))
        reverse(seats.begin(), seats.end()); // 根据棋盘顶底位置,是否反序
    return seats;
}

//中文纵线着法->(fseat, tseat)
const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromZh(const wstring& zhStr)
{
    int index{};
    shared_ptr<Seat> fseat{}, tseat{};
    vector<shared_ptr<Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ __numChars[PieceColor::RED].find(zhStr.back()) != wstring::npos ? PieceColor::RED : PieceColor::BLACK };
    bool isBottom{ isBottomSide(color) };
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](const wchar_t wch) { return static_cast<int>(__numChars[color].find(wch)) + 1; };
    auto __getCol = [&](const int num) { return isBottom ? ColNum - num : num - 1; };
    static map<wchar_t, int> __wchIndex{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
        { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
        { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };

    if (__wchIndex.find(name) == __wchIndex.end()) { // 首字符为棋子名
        seats = getLiveSeats(color, name, __getCol(__getNum(zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = ((seats.size() == 2 && (wstring(L"仕相士象").find(name) != wstring::npos)
                     && ((zhStr[2] == L'退') == isBottom))
                ? seats.size() - 1
                : 0);
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = __wchIndex[zhStr[0]];
        name = zhStr[1];
        if (name == L'兵' || name == L'卒') {
            seats = __sortPawnSeats(color, name);
            if (seats.size() == 3 && zhStr[0] == L'后')
                index = 2; // 某列有三兵且所指“后兵”时，修正index: 由1改为2
        } else {
            seats = getLiveSeats(color, name);
            if (isBottom) //# 修正index
                index = 1 - index;
        }

        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << toString();
    }
    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ __wchIndex[zhStr[2]] * (isBottom ? 1 : -1) }, num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    //int movDir{ __wchIndex(zhStr[2]) * (isBottom ? 1 : -1) },
    if (wstring(L"帅车炮兵将卒").find(name) != wstring::npos) { //#'获取直线走子toseat'
        int row = fseat->row();
        tseat = (movDir == 0) ? getSeat(row, toCol) : getSeat(row + movDir * num, fseat->col());
    } else { //#'获取斜线走子：仕、相、马toseat'
        int step{ abs(toCol - fseat->col()) }; //  # 相距1或2列
        int inc{ ((wstring(L"仕相士象").find(name) != wstring::npos)) ? step : (step == 1 ? 2 : 1) };
        tseat = getSeat(fseat->row() + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

wchar_t Board::__nullChar{ L'_' };

shared_ptr<Piece> Board::nullPiece{ make_shared<Piece>(Board::__nullChar) };

map<PieceColor, wstring> Board::__numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
