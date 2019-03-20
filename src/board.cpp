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
#include <regex>
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

const vector<shared_ptr<Piece>> Board::__creatPieces()
{
    vector<shared_ptr<Piece>> pieces{};
    wstring pieChars{ L"KAABBNNRRCCPPPPPkaabbnnrrccppppp" };
    for (auto& ch : pieChars)
        pieces.push_back(make_shared<Piece>(ch));
    return pieces;
}

const vector<shared_ptr<Seat>> Board::__creatSeats()
{
    vector<shared_ptr<Seat>> seats{};
    for (int r = 0; r < PieceAide::RowNum; ++r)
        for (int c = 0; c < PieceAide::ColNum; ++c)
            seats.push_back(make_shared<Seat>(r, c, Piece::nullPiece));
    return seats;
}

vector<shared_ptr<Seat>> Board::getLiveSeats(const PieceColor color, const wchar_t name, const int col) const
{
    vector<shared_ptr<Seat>> someSeats{};
    for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) {
        const shared_ptr<Piece>& pie{ seat->piece() };
        if (pie->color() != PieceColor::BLANK
            && (color == PieceColor::BLANK || color == pie->color()) // 空则两方棋子全选
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

const wstring Board::getFEN(const wstring& pieceChars) const
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
    for (auto& linenum : line_nums)
        while ((pos = fen.find(linenum.first)) != wstring::npos)
            fen.replace(pos, linenum.first.size(), linenum.second);
    return fen;
}

void Board::putPieces(const wstring& fen)
{
    wstring chars{ __getChars(fen) };
    if (seats_.size() != chars.size())
        cout << "错误：seats_.size() != chars.size()";

    wchar_t ch{};
    int chIndex{ -1 };
    vector<bool> used(pieces_.size(), false);
    for (auto& seat : seats_) {
        int pieIndex{ -1 };
        if ((ch = chars[++chIndex]) != PieceAide::getNullChar()) {
            for (auto& pie : pieces_)
                if (!used[++pieIndex] && pie->ch() == ch) {
                    seat->put(pie);
                    used[pieIndex] = true;
                    break;
                }
        } else
            seat->put(Piece::nullPiece);
    }
    setBottomSide();
}

const wstring Board::__getChars(const wstring& fen) const
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

void Board::setBottomSide()
{
    for (auto& seat : seats_) {
        auto pie = seat->piece();
        if (PieceAide::isKing(pie->name()))
            bottomColor = pie->color();
    }
}

shared_ptr<Seat>& Board::getOthSeat(const shared_ptr<Seat>& seat, const ChangeType ct)
{
    if (ct == ChangeType::ROTATE) // 旋转
        return seats_[seats_.size() - distance(seats_.begin(), find(seats_.begin(), seats_.end(), seat)) - 1];
    else // ChangeType::SYMMETRY 对称
        return getSeat(seat->row(), PieceAide::ColNum - seat->col());
}

const wstring Board::changeSide(const ChangeType ct)
{
    wstringstream wss{};
    if (ct == ChangeType::EXCHANGE) // 交换红黑方
        for_each(seats_.begin(), seats_.end(),
            [&](shared_ptr<Seat>& seat) { seat->put(seat->piece() == Piece::nullPiece
                                                  ? Piece::nullPiece
                                                  : pieces_[(distance(pieces_.begin(), find(pieces_.begin(), pieces_.end(), seat->piece())) + 16) % 32]); });
    else // 旋转或对称
        for_each(seats_.begin(), seats_.end(), [&](shared_ptr<Seat>& seat) { seat->put(getOthSeat(seat, ct)->piece()); });
    setBottomSide();
    for_each(seats_.begin(), seats_.end(), [&](const shared_ptr<Seat>& seat) { wss << seat->piece()->ch(); });
    return wss.str();
}

const wstring Board::getIccs(const Move& move) const
{
    wstringstream wss{};
    wstring ColChars{ L"abcdefghi" };
    wss << ColChars[move.fseat()->col()] << move.fseat()->row() << ColChars[move.tseat()->col()] << move.tseat()->row();
    return wss.str();
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
        textBlankBoard[(PieceAide::ColNum - seat->row()) * 2 * (PieceAide::ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromICCS(const wstring& ICCS)
{
    string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97), getSeat(iccs[3] - 48, iccs[2] - 97)); // 0:48, a:97
}

// '多兵排序'
const vector<shared_ptr<Seat>> Board::__sortPawnSeats(const PieceColor color, const wchar_t name)
{
    vector<shared_ptr<Seat>> seats{ getLiveSeats(color, name) }; // 最多5个兵
    // 按列建立字典，按列排序
    bool isBottom{ isBottomSide(color) };
    map<int, vector<shared_ptr<Seat>>> colSeats{};
    for_each(seats.begin(), seats.end(),
        [&](const shared_ptr<Seat>& seat) { colSeats[isBottom ? -seat->col() : seat->col()].push_back(seat); }); // 底边则列倒序,每列位置倒序

    // 整合成一个数组
    auto pos = seats.begin();
    for (auto& colSeat : colSeats)
        if (colSeat.second.size() > 1) { // 筛除只有一个位置的列
            if (isBottom) // 根据棋盘顶底位置,是否反序
                reverse(colSeat.second.begin(), colSeat.second.end());
            pos = copy(colSeat.second.begin(), colSeat.second.end(), pos); //按列存入
        }
    seats = vector<shared_ptr<Seat>>{ seats.begin(), pos };
    return seats;
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
    bool isSameRow{ toRow == fromRow }, isBottom{ isBottomSide(color) };
    vector<shared_ptr<Seat>> seats{ getLiveSeats(color, name, fromCol) };
    int length = seats.size();
    //auto __getNumChar = [&](int col) { return __numChars.at(color)[isBottom ? PieceAide::ColNum - col - 1 : col]; };

    if (length > 1 && PieceAide::isStronge(fromPiece->name())) {
        if (PieceAide::isPawn(fromPiece->name())) {
            seats = __sortPawnSeats(color, name);
            length = seats.size();
        } else if (isBottom) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        //wstring preStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        //wss << preStr[static_cast<int>(distance(seats.begin(), find(seats.begin(), seats.end(), fseat)))] << name;
        wss << PieceAide::getIndexChar(length,
                   static_cast<int>(distance(seats.begin(), find(seats.begin(), seats.end(), fseat))))
            << name;
    } else //将帅 , 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << PieceAide::getColChar(color, isBottom, fromCol);

    //wcout << (toRow == fromRow ? L'平' : (isBottom == (toRow > fromRow) ? L'进' : L'退')) << endl;
    wss << PieceAide::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (PieceAide::isLineMove(fromPiece->name()) && !isSameRow
                   ? PieceAide::getNumChar(color, abs(fromRow - toRow))
                   : PieceAide::getColChar(color, isBottom, toCol));
    //: (isBottom ? MaxCol - getCol(tseat) : getCol(tseat))];

    /*
    auto mvSeats = __getSeatFromZh(wss.str());
    if (mvSeats.first != fseat || mvSeats.second != tseat) {
        wcout << L"fseat:" << fseat->toString() << L"tseat:" << tseat->toString() << L'\n'
              << L"mvSeats.first:" << mvSeats.first->toString()
              << L"mvSeats.second:" << mvSeats.second->toString() << endl;
    }
    if (wss.str().size() != 4)
        cout << "move.zh().size()!=4" << endl;
    //*/

    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> Board::__getSeatFromZh(const wstring& zhStr)
{
    int index{};
    shared_ptr<Seat> fseat{}, tseat{};
    vector<shared_ptr<Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ PieceAide::getColor_wch(zhStr.back()) };
    bool isBottom{ isBottomSide(color) };
    wchar_t name{ zhStr[0] };
    //auto __getNum = [&](const wchar_t wch) { return static_cast<int>(__numChars.at(color).find(wch)) + 1; };
    //auto __getCol = [&](const int num) { return isBottom ? ColNum - num : num - 1; };

    if (PieceAide::isPieceName(name)) { // 首字符为棋子名
        seats = getLiveSeats(color, name, PieceAide::getCol(isBottom, PieceAide::getNum(color, zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << PieceAide::getCol(isBottom, PieceAide::getNum(color, zhStr[1])) << L' ' << L'\n' << toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && PieceAide::isAdvBish(name) && (zhStr[2] == L'退') == isBottom) ? 1 : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        const map<wchar_t, int> __preIndex{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 }, { L'四', 3 }, { L'五', 4 },
            { L'前', 0 }, { L'中', 1 }, { L'后', 1 } };
        index = __preIndex.at(zhStr[0]);
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
    if (index > static_cast<int>(seats.size()) - 1)
        wcout << L"index > seats.size()-1:" << zhStr << L' ' << name << L'\n' << toString();

    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    //const map<wchar_t, int> __movIndex{ { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
    int movDir{ PieceAide::getMovDir(isBottom, zhStr[2]) },
        num{ PieceAide::getNum(color, zhStr[3]) }, toCol{ PieceAide::getCol(isBottom, num) };
    //int movDir{ __wchIndex(zhStr[2]) * (isBottom ? 1 : -1) },
    if (PieceAide::isLineMove(name))
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(fseat->row() + movDir * num, fseat->col());
    else { // 斜线走子：仕、相、马
        int step{ abs(toCol - fseat->col()) }; //  相距1或2列
        tseat = getSeat(fseat->row() + movDir * (PieceAide::isAdvBish(name) ? step : (step == 1 ? 2 : 1)), toCol);
    }

    //*
    Move mv{};
    mv.setSeats(fseat, tseat);
    if (zhStr != getZh(mv))
        wcout << L"zhStr:" << zhStr << L" getZh(mv):" << getZh(mv) << endl;
    //*/
    return make_pair(fseat, tseat);
}

const wstring Board::test() //const
{
    wstringstream wss{};
    // Piece test
    wss << setw(4) << "color" << setw(6)
        << "char" << setw(5) << "name" << setw(8) << "isKing"
        << setw(8) << "isPawn" << setw(8) << "Stronge" << setw(8) << "Line\n";
    for (auto& pie : pieces_)
        wss << pie->toString() << L'\n';
    wss << Piece::nullPiece->toString() << L'\n';

    // Board test
    wstring fen{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
    //wstring fen{ L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" };
    wstring chars{ __getChars(fen) };
    wss << fen << L'\n' << chars << L'\n';
    putPieces(fen);

    wss << toString();
    return wss.str();
}