#include "board.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace BoardSpace {

Board::Board()
    : bottomColor{ PieceColor::RED }
    , pieces_{ BoardAide::creatPieces() }
    , seats_{ BoardAide::creatSeats() }
{
}

const std::shared_ptr<SeatSpace::Seat> Board::getSeat(const int rowcol) const
{
    return __getSeat(rowcol / 10, rowcol % 10);
}

const std::shared_ptr<SeatSpace::Seat> Board::getOthSeat(const std::shared_ptr<SeatSpace::Seat>& seat,
    const ChangeType ct) const // ChangeType::ROTATE旋转 // ChangeType::SYMMETRY 对称
{
    return (ct == ChangeType::ROTATE ? __getSeat(BoardAide::RowNum - seat->row() - 1, BoardAide::ColNum - seat->col() - 1)
                                     : __getSeat(seat->row(), BoardAide::ColNum - seat->col() - 1));
}

//判断是否将军
const bool Board::isKilled(const PieceColor color)
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    std::shared_ptr<SeatSpace::Seat> kingSeat{ getKingSeat(color) }, othKingSeat{ getKingSeat(othColor) };
    int fcol{ kingSeat->col() }, tcol{ othKingSeat->col() };
    if (fcol == tcol) {
        bool isBottom{ __isBottomSide(color) }, isFace{ true };
        int frow{ kingSeat->row() }, trow{ othKingSeat->row() }, lrow{ isBottom ? frow : trow }, urow{ isBottom ? trow : frow };
        for (int row = lrow + 1; row < urow; ++row)
            if (__getSeat(row, fcol)->piece()) {
                isFace = false;
                break;
            }
        if (isFace)
            return true;
    }
    for (auto fseat : __getLiveSeats(othColor))
        if (BoardAide::isStronge(fseat->piece()->name())) {
            auto& seats = moveSeats(fseat);
            if (std::find(seats.begin(), seats.end(), kingSeat) != seats.end())
                return true;
        }
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color)
{
    for (auto fseat : __getLiveSeats(color))
        if (moveSeats(fseat).size() > 0)
            return false;
    return true;
}

const std::wstring Board::getPieceChars() const
{
    std::wstringstream wss{};
    std::shared_ptr<PieceSpace::Piece> pie{};
    for_each(seats_.begin(), seats_.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << ((pie = seat->piece()) ? pie->ch() : BoardAide::nullChar); });
    return wss.str();
}

const std::wstring Board::getFEN(const std::wstring& pieceChars) const
{
    //'下划线字符串对应数字字符'
    std::vector<std::pair<std::wstring, std::wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    std::wstring fen{};
    for (int i = 81; i >= 0; i -= 9)
        fen += pieceChars.substr(i, 9) + L"/";
    fen.erase(fen.size() - 1, 1);
    std::wstring::size_type pos;
    for (auto& linenum : line_nums)
        while ((pos = fen.find(linenum.first)) != std::wstring::npos)
            fen.replace(pos, linenum.first.size(), linenum.second);
    return fen;
}

void Board::putPieces(const std::wstring& fen)
{
    std::wstring chars{ __getChars(fen) };
    if (seats_.size() != chars.size())
        std::cout << "错误：seats_.size() != chars.size()" << seats_.size() << "=" << chars.size() << "=" << fen.size() << std::endl;

    wchar_t ch{};
    int chIndex{ -1 };
    std::vector<bool> used(pieces_.size(), false);
    for (auto& seat : seats_) {
        if ((ch = chars[++chIndex]) != BoardAide::nullChar) {
            int pieIndex{ -1 };
            for (auto& pie : pieces_)
                if (!used[++pieIndex] && pie->ch() == ch) {
                    seat->put(pie);
                    used[pieIndex] = true;
                    break;
                }
        } else
            seat->put();
    }
    __setBottomSide();
}

const std::wstring Board::__getChars(const std::wstring& fen) const
{
    std::wstring chars{};
    std::wregex sp{ LR"(/)" };
    for (std::wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != std::wsregex_token_iterator{}; ++wti) {
        std::wstringstream line{};
        for (const auto& wch : std::wstring{ *wti })
            line << (isdigit(wch) ? std::wstring(wch - 48, BoardAide::nullChar) : std::wstring{ wch }); // ASCII: 0:48
        chars.insert(0, line.str());
    }
    return chars;
}

void Board::__setBottomSide()
{
    bottomColor = getKingSeat(PieceColor::RED)->row() < BoardAide::RowLowUpIndex ? PieceColor::RED : PieceColor::BLACK;
}

void Board::changeSide(const ChangeType ct)
{
    if (ct == ChangeType::EXCHANGE) // 交换红黑方
        for_each(seats_.begin(), seats_.end(), [&](std::shared_ptr<SeatSpace::Seat> seat) {
            seat->put(seat->piece() ? pieces_[(distance(pieces_.begin(), find(pieces_.begin(), pieces_.end(), seat->piece())) + 16) % 32] : nullptr);
        });
    else { // 旋转或对称
        std::vector<std::shared_ptr<PieceSpace::Piece>> seatPieces{};
        for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            seatPieces.push_back(getOthSeat(seat, ct)->piece());
        });
        auto pieItr = seatPieces.begin();
        for_each(seats_.begin(), seats_.end(), [&](std::shared_ptr<SeatSpace::Seat> seat) {
            seat->put(*pieItr);
            ++pieItr;
        });
    }
    __setBottomSide();
}

const std::wstring Board::getIccs(const MoveSpace::Move& move) const
{
    std::wstringstream wss{};
    std::wstring ColChars{ L"abcdefghi" };
    wss << ColChars[move.fseat()->col()] << move.fseat()->row()
        << ColChars[move.tseat()->col()] << move.tseat()->row();
    return wss.str();
}

// '获取棋子可走的位置, 不能被将军'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat)
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    auto piece = fseat->piece();
    for (auto tseat : piece->moveSeats(*this, fseat)) {
        auto eatPiece = fseat->to(tseat);
        // 移动棋子后，检测是否会被对方将军
        if (!this->isKilled(piece->color()))
            seats.push_back(tseat);
        tseat->to(fseat, eatPiece);
    }
    return seats;
}

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeat(const MoveSpace::Move& move, const RecFormat fmt) const
{
    if (fmt == RecFormat::ICCS)
        return __getSeatFromICCS(move.iccs());
    else // case RecFormat::ZH: //case RecFormat::CC:
        return __getSeatFromZh(move.zh());
}

const std::wstring Board::toString() const
{
    // 文本空棋盘
    std::wstring textBlankBoard{ L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
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
    std::map<wchar_t, wchar_t> rcpName{ { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    auto __getName = [&](const PieceSpace::Piece& pie) {
        wchar_t name = pie.name();
        return ((pie.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end())
                ? rcpName[name]
                : name);
    };
    for (auto& seat : __getLiveSeats(PieceColor::BLANK))
        textBlankBoard[(BoardAide::ColNum - seat->row()) * 2 * (BoardAide::ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const std::shared_ptr<SeatSpace::Seat> Board::__getSeat(const int row, const int col) const
{
    return seats_[row * BoardAide::ColNum + col];
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats(const PieceColor color,
    const wchar_t name,
    const int col) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> someSeats{};
    std::shared_ptr<PieceSpace::Piece> pie{};
    for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        if ((pie = seat->piece()) //->color() != PieceColor::BLANK // 活的棋子
            && (color == PieceColor::BLANK || color == pie->color()) // 空则两方棋子全选
            && (name == L'\x00' || name == pie->name()) // 空则各种棋子全选
            && (col == -1 || col == seat->col())) // -1则各列棋子全选
            someSeats.push_back(seat);
    });
    return someSeats;
}

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::__getSeatFromICCS(const std::wstring& ICCS) const
{
    std::string iccs{ Tools::ws2s(ICCS) };
    return make_pair(__getSeat(iccs[1] - 48, iccs[0] - 97), __getSeat(iccs[3] - 48, iccs[2] - 97)); // 0:48, a:97
}

// '多兵排序'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__sortPawnSeats(const PieceColor color,
    const wchar_t name) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ __getLiveSeats(color, name) }; // 最多5个兵
    // 按列建立字典，按列排序
    std::map<int, std::vector<std::shared_ptr<SeatSpace::Seat>>> colSeats{};
    for_each(seats.begin(), seats.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            colSeats[seat->col()].push_back(seat);
        }); // 底边则列倒序,每列位置倒序

    // 整合成一个数组
    auto pos = seats.begin();
    for (auto& colSeat : colSeats)
        if (colSeat.second.size() > 1) // 筛除只有一个位置的列
            pos = copy(colSeat.second.begin(), colSeat.second.end(), pos); //按列存入
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ seats.begin(), pos };
}

//(fseat, tseat)->中文纵线着法
const std::wstring Board::getZh(const MoveSpace::Move& move) const
{
    std::wstringstream wss{};
    const std::shared_ptr<SeatSpace::Seat>&fseat{ move.fseat() }, &tseat{ move.tseat() };
    const std::shared_ptr<PieceSpace::Piece>& fromPiece{ fseat->piece() };
    const PieceColor color{ fromPiece->color() };
    const wchar_t name{ fromPiece->name() };
    const int fromRow{ fseat->row() }, fromCol{ fseat->col() }, toRow{ tseat->row() }, toCol{ tseat->col() };
    bool isSameRow{ fromRow == toRow }, isBottom{ __isBottomSide(color) };
    auto seats = __getLiveSeats(color, name, fromCol);

    if (seats.size() > 1 && BoardAide::isStronge(name)) {
        if (BoardAide::isPawn(name))
            seats = __sortPawnSeats(color, name);
        int index = distance(seats.begin(), find(seats.begin(), seats.end(), fseat));
        wss << BoardAide::getIndexChar(seats.size(), isBottom, index) << name;
    } else //将帅 , 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << BoardAide::getColChar(color, isBottom, fromCol);

    wss << BoardAide::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (BoardAide::isLineMove(fromPiece->name()) && !isSameRow
                   ? BoardAide::getNumChar(color, abs(fromRow - toRow))
                   : BoardAide::getColChar(color, isBottom, toCol));

    //*
    auto mvSeats = __getSeatFromZh(wss.str());
    if (mvSeats.first != fseat || mvSeats.second != tseat) {
        std::wcout << L"fseat:" << fseat->toString() << L"tseat:" << tseat->toString()
                   << L'\n' << L"mvSeats.first:" << mvSeats.first->toString()
                   << L"mvSeats.second:" << mvSeats.second->toString() << std::endl;
    }
    //*/

    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::__getSeatFromZh(const std::wstring& zhStr) const
{
    std::shared_ptr<SeatSpace::Seat> fseat{}, tseat{};
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ BoardAide::getColor(zhStr.back()) };
    bool isBottom{ __isBottomSide(color) };
    int index{}, movDir{ BoardAide::getMovDir(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (BoardAide::isPieceName(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name, BoardAide::getCol(isBottom, BoardAide::getNum(color, zhStr.at(1))));
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && (isBottom == (movDir == -1))) ? 1 : 0; // && BoardAide::isAdvBish(name) 可不用
    } else {
        name = zhStr.at(1);
        seats = BoardAide::isPawn(name) ? __sortPawnSeats(color, name) : __getLiveSeats(color, name);
        index = BoardAide::getIndex(seats.size(), isBottom, zhStr.front());
    }
    fseat = seats.at(index);

    int num{ BoardAide::getNum(color, zhStr.back()) }, toCol{ BoardAide::getCol(isBottom, num) };
    if (BoardAide::isLineMove(name))
        tseat = movDir == 0 ? __getSeat(fseat->row(), toCol) : __getSeat(fseat->row() + movDir * num, fseat->col());
    else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }; //  相距1或2列
        tseat = __getSeat(fseat->row() + movDir * (BoardAide::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)), toCol);
    }

    /*
    MoveSpace::Move mv{};
    mv.setSeats(fseat, tseat);
    if (zhStr != getZh(mv))
        std::wcout << L"zhStr:" << zhStr << L" getZh(mv):" << getZh(mv) << std::endl;
    //*/
    return make_pair(fseat, tseat);
}

const std::shared_ptr<SeatSpace::Seat> Board::getKingSeat(const PieceColor color) const
{
    std::shared_ptr<PieceSpace::Piece> pie{};
    auto seats = getKingSeats(color);
    auto pos = std::find_if(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        return (pie = seat->piece()) && BoardAide::isKing(pie->name());
    });
    return *pos;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpMidIndex },
        rowUp{ isBottom ? BoardAide::RowLowMidIndex : BoardAide::RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = BoardAide::ColMidLowIndex; col <= BoardAide::ColMidUpIndex; ++col)
            seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvisorSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpMidIndex },
        rowUp{ isBottom ? BoardAide::RowLowMidIndex : BoardAide::RowUpIndex }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = BoardAide::ColMidLowIndex; col <= BoardAide::ColMidUpIndex; ++col)
            if ((col + row) % 2 == rmd)
                seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpLowIndex },
        rowUp{ isBottom ? BoardAide::RowLowUpIndex : BoardAide::RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = BoardAide::ColLowIndex; col <= BoardAide::ColUpIndex; col += 2) {
            int gap{ row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                seats.push_back(__getSeat(row, col));
        }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getPawnSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int lfrow{ isBottom ? BoardAide::RowLowUpIndex - 1 : BoardAide::RowUpLowIndex },
        ufrow{ isBottom ? BoardAide::RowLowUpIndex : BoardAide::RowUpLowIndex + 1 },
        ltrow{ isBottom ? BoardAide::RowUpLowIndex : BoardAide::RowLowIndex },
        utrow{ isBottom ? BoardAide::RowUpIndex : BoardAide::RowLowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int col = BoardAide::ColLowIndex; col <= BoardAide::ColUpIndex; col += 2)
        for (int row = lfrow; row <= ufrow; ++row)
            seats.push_back(__getSeat(row, col));
    for (int col = BoardAide::ColLowIndex; col <= BoardAide::ColUpIndex; ++col)
        for (int row = ltrow; row <= utrow; ++row)
            seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ fseat->piece()->getSeats(*this) }, result(4);
    auto p = std::copy_if(seats.begin(), seats.end(), result.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (((fseat->row() == tseat->row() && abs(fseat->col() - tseat->col()) == 1)
                        || (fseat->col() == tseat->col() && abs(fseat->row() - tseat->row()) == 1))
                && tseat->isDiffColor(fseat));
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ result.begin(), p };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvsiorMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ fseat->piece()->getSeats(*this) }, result(4);
    auto p = std::copy_if(seats.begin(), seats.end(), result.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(fseat->row() - tseat->row()) == 1
                && abs(fseat->col() - tseat->col()) == 1
                && tseat->isDiffColor(fseat));
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ result.begin(), p };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ fseat->piece()->getSeats(*this) }, result(4);
    auto p = std::copy_if(seats.begin(), seats.end(), result.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(fseat->row() - tseat->row()) == 2
                && abs(fseat->col() - tseat->col()) == 2
                && !__getSeat((fseat->row() + tseat->row()) / 2, (fseat->col() + tseat->col()) / 2)->piece()
                && tseat->isDiffColor(fseat));
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ result.begin(), p };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ fseat->piece()->getSeats(*this) }, result(8);
    auto p = std::copy_if(seats.begin(), seats.end(), result.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (((abs(fseat->row() - tseat->row()) == 1 && abs(fseat->col() - tseat->col()) == 2
                         && !__getSeat(fseat->row(), fseat->col() + (fseat->col() > tseat->col() ? -1 : 1))->piece())
                        || (abs(fseat->row() - tseat->row()) == 2 && abs(fseat->col() - tseat->col()) == 1
                               && !__getSeat(fseat->row() + (fseat->row() > tseat->row() ? -1 : 1), fseat->col())->piece()))
                && tseat->isDiffColor(fseat));
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ result.begin(), p };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getRookMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto seatLine : getRookCannonMoveSeat_Lines(fseat))
        for (auto& tseat : seatLine) {
            if (tseat->isDiffColor(fseat))
                seats.push_back(tseat);
            if (tseat->piece())
                break;
        }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getCannonMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto seatLine : getRookCannonMoveSeat_Lines(fseat)) {
        bool skip = false;
        for (auto& tseat : seatLine)
            if (!skip) {
                if (!tseat->piece())
                    seats.push_back(tseat);
                else
                    skip = true;
            } else if (tseat->piece()) {
                if (tseat->isDiffColor(fseat))
                    seats.push_back(tseat);
                break;
            }
    }
    return seats;
}

const std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> Board::getRookCannonMoveSeat_Lines(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> result(4);
    int frow{ fseat->row() }, fcol{ fseat->col() };
    for (int col = fcol - 1; col >= BoardAide::ColLowIndex; --col)
        result[0].push_back(__getSeat(frow, col));
    for (int col = fcol + 1; col <= BoardAide::ColUpIndex; ++col)
        result[1].push_back(__getSeat(frow, col));
    for (int row = frow - 1; row >= BoardAide::RowLowIndex; --row)
        result[2].push_back(__getSeat(row, fcol));
    for (int row = frow + 1; row <= BoardAide::RowUpIndex; ++row)
        result[3].push_back(__getSeat(row, fcol));
    return result;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getPawnMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> result{};
    int frow{ fseat->row() }, fcol{ fseat->col() }, row{}, col{};
    std::shared_ptr<SeatSpace::Seat> tseat{};
    if ((col = fcol - 1) >= BoardAide::ColLowIndex && (tseat = __getSeat(frow, col))->isDiffColor(fseat))
        result.push_back(tseat);
    if ((col = fcol + 1) <= BoardAide::ColUpIndex && (tseat = __getSeat(frow, col))->isDiffColor(fseat))
        result.push_back(tseat);
    if (__isBottomSide(fseat->piece()->color())) {
        if ((row = frow + 1) <= BoardAide::RowUpIndex && (tseat = __getSeat(row, fcol))->isDiffColor(fseat))
            result.push_back(tseat);
    } else {
        if ((row = frow - 1) >= BoardAide::RowLowIndex && (tseat = __getSeat(row, fcol))->isDiffColor(fseat))
            result.push_back(tseat);
    }
    return result;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::BoardAide::creatSeats()
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = 0; row < RowNum; ++row)
        for (int col = 0; col < ColNum; ++col)
            seats.push_back(std::make_shared<SeatSpace::Seat>(row, col));
    return seats;
}

const wchar_t Board::BoardAide::getChar(const PieceColor color, const int index) { return numChars.at(color)[index]; };

const std::wstring Board::BoardAide::getPreChars(const int length)
{
    switch (length) {
    case 2:
        return L"前后";
    case 3:
        return L"前中后";
    default:
        return L"一二三四五";
    }
}

const PieceColor Board::BoardAide::getColor(const wchar_t numZh)
{
    return numChars.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const wchar_t Board::BoardAide::getIndexChar(const int length, const bool isBottom, const int index)
{
    return getPreChars(length)[isBottom ? length - index - 1 : index];
}

const wchar_t Board::BoardAide::getMovChar(const bool isSameRow, bool isBottom, bool isForward)
{
    return movChars.at(isSameRow ? 1 : (isBottom == isForward ? 2 : 0));
}

const wchar_t Board::BoardAide::getNumChar(const PieceColor color, const int num)
{
    return getChar(color, num - 1);
};

const wchar_t Board::BoardAide::getColChar(const PieceColor color, bool isBottom, const int col)
{
    return getChar(color, isBottom ? ColNum - col - 1 : col);
};

const int Board::BoardAide::getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index{ static_cast<int>(getPreChars(seatsLen).find(preChar)) };
    return isBottom ? seatsLen - index - 1 : index;
}

const int Board::BoardAide::getMovDir(const bool isBottom, const wchar_t movChar)
{
    return static_cast<int>(movChars.find(movChar) - 1) * (isBottom ? 1 : -1);
}

const int Board::BoardAide::getNum(const PieceColor color, const wchar_t numChar)
{
    return static_cast<int>(numChars.at(color).find(numChar)) + 1;
}

const int Board::BoardAide::getCol(bool isBottom, const int num)
{
    return isBottom ? ColNum - num : num - 1;
}

const int Board::BoardAide::RowNum{ 10 };
const int Board::BoardAide::RowLowIndex{ 0 };
const int Board::BoardAide::RowLowMidIndex{ 2 };
const int Board::BoardAide::RowLowUpIndex{ 4 };
const int Board::BoardAide::RowUpLowIndex{ 5 };
const int Board::BoardAide::RowUpMidIndex{ 7 };
const int Board::BoardAide::RowUpIndex{ 9 };
const int Board::BoardAide::ColNum{ 9 };
const int Board::BoardAide::ColLowIndex{ 0 };
const int Board::BoardAide::ColMidLowIndex{ 3 };
const int Board::BoardAide::ColMidUpIndex{ 5 };
const int Board::BoardAide::ColUpIndex{ 8 };
const std::wstring Board::BoardAide::movChars{ L"退平进" };
const std::map<PieceColor, std::wstring> Board::BoardAide::numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};

const std::vector<std::shared_ptr<PieceSpace::Piece>> Board::BoardAide::creatPieces()
{
    std::vector<std::shared_ptr<PieceSpace::Piece>> pieces{
        std::make_shared<PieceSpace::King>(L'K', nameChars.at(0), PieceColor::RED),
        std::make_shared<PieceSpace::Advisor>(L'A', nameChars.at(2), PieceColor::RED),
        std::make_shared<PieceSpace::Advisor>(L'A', nameChars.at(2), PieceColor::RED),
        std::make_shared<PieceSpace::Bishop>(L'B', nameChars.at(4), PieceColor::RED),
        std::make_shared<PieceSpace::Bishop>(L'B', nameChars.at(4), PieceColor::RED),
        std::make_shared<PieceSpace::Knight>(L'N', nameChars.at(6), PieceColor::RED),
        std::make_shared<PieceSpace::Knight>(L'N', nameChars.at(6), PieceColor::RED),
        std::make_shared<PieceSpace::Rook>(L'R', nameChars.at(7), PieceColor::RED),
        std::make_shared<PieceSpace::Rook>(L'R', nameChars.at(7), PieceColor::RED),
        std::make_shared<PieceSpace::Cannon>(L'C', nameChars.at(8), PieceColor::RED),
        std::make_shared<PieceSpace::Cannon>(L'C', nameChars.at(8), PieceColor::RED),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<PieceSpace::King>(L'k', nameChars.at(1), PieceColor::BLACK),
        std::make_shared<PieceSpace::Advisor>(L'a', nameChars.at(3), PieceColor::BLACK),
        std::make_shared<PieceSpace::Advisor>(L'a', nameChars.at(3), PieceColor::BLACK),
        std::make_shared<PieceSpace::Bishop>(L'b', nameChars.at(5), PieceColor::BLACK),
        std::make_shared<PieceSpace::Bishop>(L'b', nameChars.at(5), PieceColor::BLACK),
        std::make_shared<PieceSpace::Knight>(L'n', nameChars.at(6), PieceColor::BLACK),
        std::make_shared<PieceSpace::Knight>(L'n', nameChars.at(6), PieceColor::BLACK),
        std::make_shared<PieceSpace::Rook>(L'r', nameChars.at(7), PieceColor::BLACK),
        std::make_shared<PieceSpace::Rook>(L'r', nameChars.at(7), PieceColor::BLACK),
        std::make_shared<PieceSpace::Cannon>(L'c', nameChars.at(8), PieceColor::BLACK),
        std::make_shared<PieceSpace::Cannon>(L'c', nameChars.at(8), PieceColor::BLACK),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10), PieceColor::BLACK)
    };
    return pieces;
}

const bool Board::BoardAide::isKing(const wchar_t name) { return nameChars.substr(0, 2).find(name) != std::wstring::npos; }
const bool Board::BoardAide::isAdvBish(const wchar_t name) { return nameChars.substr(2, 4).find(name) != std::wstring::npos; }
const bool Board::BoardAide::isStronge(const wchar_t name) { return nameChars.substr(6, 5).find(name) != std::wstring::npos; }
const bool Board::BoardAide::isLineMove(const wchar_t name) { return isKing(name) || nameChars.substr(7, 4).find(name) != std::wstring::npos; }
const bool Board::BoardAide::isPawn(const wchar_t name) { return nameChars.substr(nameChars.size() - 2, 2).find(name) != std::wstring::npos; }
const bool Board::BoardAide::isPieceName(const wchar_t name) { return nameChars.find(name) != std::wstring::npos; }
const wchar_t Board::BoardAide::nullChar{ L'_' };
const std::wstring Board::BoardAide::nameChars{ L"帅将仕士相象马车炮兵卒" };

const std::wstring Board::test()
{
    std::wstringstream wss{};
    // Piece test
    for (auto& pie : pieces_)
        wss << pie->toString() << L' ';
    wss << L"\n";

    // Board test
    //std::wstring fen{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
    std::wstring fen{ L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" };
    putPieces(fen);
    wss << fen << L'\n' << getFEN(getPieceChars()) << L'\n' << __getChars(fen) << L'\n' << getPieceChars() << L'\n';

    for (auto& fseat : __getLiveSeats(PieceColor::BLANK)) {
        wss << fseat->toString() << L':';
        //for (auto& seat : moveSeats(fseat))
        //   wss << seat->toString() << L' ';
        wss << L'\n';
    }

    wss << toString() << L'\n';
    changeSide(ChangeType::EXCHANGE);
    wss << toString() << L'\n';
    changeSide(ChangeType::ROTATE);
    wss << toString() << L'\n';
    changeSide(ChangeType::SYMMETRY);
    wss << toString() << L'\n';

    return wss.str();
}
}