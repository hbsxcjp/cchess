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
    , pieces_{ creatPieces() }
    , seats_{ creatSeats() }
{
}

const std::shared_ptr<SeatSpace::Seat> Board::getSeat(const int rowcol) const
{
    return __getSeat(rowcol / 10, rowcol % 10);
}

const std::shared_ptr<SeatSpace::Seat> Board::getOthSeat(const std::shared_ptr<SeatSpace::Seat>& seat,
    const ChangeType ct) const // ChangeType::ROTATE旋转 // ChangeType::SYMMETRY 对称
{
    return (ct == ChangeType::ROTATE ? __getSeat(RowNum - seat->row() - 1, ColNum - seat->col() - 1)
                                     : __getSeat(seat->row(), ColNum - seat->col() - 1));
}

//判断是否将军
const bool Board::isKilled(const PieceColor color)
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    std::shared_ptr<SeatSpace::Seat> kingSeat{ getKingSeat(color) }, othKingSeat{ getKingSeat(othColor) };
    int fcol{ kingSeat->col() };
    if (fcol == othKingSeat->col()) {
        bool isBottom{ __isBottomSide(color) };
        int uprow{ (isBottom ? othKingSeat->row() : kingSeat->row()) - 1 };
        std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
        for (int row = (isBottom ? kingSeat->row() : othKingSeat->row()) + 1; row <= uprow; ++row)
            seats.push_back(__getSeat(row, fcol));
        if (std::all_of(seats.begin(), seats.end(),
                [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return !seat->piece(); }))
            return true;
    }
    auto& allMoveSeats = __allMoveSeats(othColor);
    if (std::any_of(allMoveSeats.begin(), allMoveSeats.end(),
            [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return seat == kingSeat; }))
        return true;
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
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << ((pie = seat->piece()) ? pie->ch() : nullChar); });
    return wss.str();
}

void Board::putPieces(const std::wstring& pieceChars)
{
    if (seats_.size() != pieceChars.size())
        std::cout << "错误：seats_.size() != pieceChars.size()" << seats_.size() << "=" << pieceChars.size() << std::endl;

    wchar_t ch{};
    int chIndex{ -1 };
    std::vector<bool> used(pieces_.size(), false);
    for (auto& seat : seats_) {
        if ((ch = pieceChars[++chIndex]) != nullChar) {
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

void Board::__setBottomSide()
{
    bottomColor = getKingSeat(PieceColor::RED)->row() < RowLowUpIndex ? PieceColor::RED : PieceColor::BLACK;
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
    std::wcout << toString() << fseat->toString() << "moveSeats: " << std::endl;

    const auto& piece = fseat->piece();
#ifndef NDEBUG
    if (!piece)
        std::cerr << "Error! " << __FILE__ << ": in function: " << __func__ << ", at line: " << __LINE__ << std::endl;
#endif

    auto color = piece->color();
    for (auto tseat : piece->__moveSeats(*this, fseat)) {
        const auto& eatPiece = fseat->to(tseat);
        // 移动棋子后，检测是否会被对方将军
        if (!isKilled(color))
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
        textBlankBoard[(ColNum - seat->row()) * 2 * (ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const std::shared_ptr<SeatSpace::Seat> Board::__getSeat(const int row, const int col) const
{
    return seats_[row * ColNum + col];
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats(const PieceColor color,
    const wchar_t name,
    const int col) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats(seats_.size());
    std::shared_ptr<PieceSpace::Piece> pie{};
    auto pos = std::copy_if(seats_.begin(), seats_.end(), seats.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            return ((pie = seat->piece()) //->color() != PieceColor::BLANK // 活的棋子
                && (color == PieceColor::BLANK || color == pie->color()) // 空则两方棋子全选
                && (name == L'\x00' || name == pie->name()) // 空则各种棋子全选
                && (col == -1 || col == seat->col())); // -1则各列棋子全选
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ seats.begin(), pos };
}

// '获取某方棋子全部可走的位置, 只用来检查是否可杀对方将帅，不考虑被将军的情况'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__allMoveSeats(const PieceColor color) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{}, liveSeats{ __getLiveSeats(color) };
    std::for_each(liveSeats.begin(), liveSeats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        if (isStronge(seat->piece()->name())) {
            auto& mvSeats = seat->piece()->__moveSeats(*this, seat);
            copy(mvSeats.begin(), mvSeats.end(), std::back_inserter(seats));
        }
    });
    return seats;
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

    if (seats.size() > 1 && isStronge(name)) {
        if (isPawn(name))
            seats = __sortPawnSeats(color, name);
        int index = distance(seats.begin(), find(seats.begin(), seats.end(), fseat));
        wss << getIndexChar(seats.size(), isBottom, index) << name;
    } else //将帅 , 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << getColChar(color, isBottom, fromCol);

    wss << getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (isLineMove(fromPiece->name()) && !isSameRow
                   ? getNumChar(color, abs(fromRow - toRow))
                   : getColChar(color, isBottom, toCol));

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
    PieceColor color{ getColor(zhStr.back()) };
    bool isBottom{ __isBottomSide(color) };
    int index{}, movDir{ getMovDir(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (isPieceName(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name, getCol(isBottom, getNum(color, zhStr.at(1))));
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && (isBottom == (movDir == -1))) ? 1 : 0; // && isAdvBish(name) 可不用
    } else {
        name = zhStr.at(1);
        seats = isPawn(name) ? __sortPawnSeats(color, name) : __getLiveSeats(color, name);
        index = getIndex(seats.size(), isBottom, zhStr.front());
    }
    fseat = seats.at(index);

    int num{ getNum(color, zhStr.back()) }, toCol{ getCol(isBottom, num) };
    if (isLineMove(name))
        tseat = movDir == 0 ? __getSeat(fseat->row(), toCol) : __getSeat(fseat->row() + movDir * num, fseat->col());
    else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }; //  相距1或2列
        tseat = __getSeat(fseat->row() + movDir * (isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)), toCol);
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
        return (pie = seat->piece()) && isKing(pie->name());
    });
    return *pos;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? RowLowIndex : RowUpMidIndex },
        rowUp{ isBottom ? RowLowMidIndex : RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex; col <= ColMidUpIndex; ++col)
            seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvisorSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? RowLowIndex : RowUpMidIndex },
        rowUp{ isBottom ? RowLowMidIndex : RowUpIndex }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex; col <= ColMidUpIndex; ++col)
            if ((col + row) % 2 == rmd)
                seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopSeats(const PieceColor color) const
{
    bool isBottom{ __isBottomSide(color) };
    int rowLow{ isBottom ? RowLowIndex : RowUpLowIndex },
        rowUp{ isBottom ? RowLowUpIndex : RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = ColLowIndex; col <= ColUpIndex; col += 2) {
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
    int lfrow{ isBottom ? RowLowUpIndex - 1 : RowUpLowIndex },
        ufrow{ isBottom ? RowLowUpIndex : RowUpLowIndex + 1 },
        ltrow{ isBottom ? RowUpLowIndex : RowLowIndex },
        utrow{ isBottom ? RowUpIndex : RowLowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int col = ColLowIndex; col <= ColUpIndex; col += 2)
        for (int row = lfrow; row <= ufrow; ++row)
            seats.push_back(__getSeat(row, col));
    for (int col = ColLowIndex; col <= ColUpIndex; ++col)
        for (int row = ltrow; row <= utrow; ++row)
            seats.push_back(__getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), seats.begin(),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (((fseat->row() == tseat->row() && abs(fseat->col() - tseat->col()) == 1)
                        || (fseat->col() == tseat->col() && abs(fseat->row() - tseat->row()) == 1))
                && tseat->isDiffColor(fseat));
        });
    std::wcout << fseat->toString() << L"getKingMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvsiorMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(fseat->row() - tseat->row()) == 1
                && abs(fseat->col() - tseat->col()) == 1
                && tseat->isDiffColor(fseat));
        });
    std::wcout << fseat->toString() << L"getAdvisorMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(fseat->row() - tseat->row()) == 2
                && abs(fseat->col() - tseat->col()) == 2
                && !__getSeat((fseat->row() + tseat->row()) / 2, (fseat->col() + tseat->col()) / 2)->piece()
                && tseat->isDiffColor(fseat));
        });
    std::wcout << fseat->toString() << L"getBishopMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    std::copy_if(seats_.begin(), seats_.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            int trow{ tseat->row() }, tcol{ tseat->col() }, irow{ frow > trow ? -1 : 1 }, icol{ fcol > tcol ? -1 : 1 };
            return (((abs(frow - trow) == 1 && abs(fcol - tcol) == 2 && !__getSeat(frow, fcol + icol)->piece())
                        || (abs(frow - trow) == 2 && abs(fcol - tcol) == 1 && !__getSeat(frow + irow, fcol)->piece()))
                && tseat->isDiffColor(fseat));
        });

#ifndef NDEBUG
    if (seats.size() > 8)
        std::cerr << "Error! " << __FILE__ << ": in function: " << __func__ << ", at line: " << __LINE__ << std::endl;
#endif

    std::wcout << fseat->toString() << L"getKnightMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
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
    std::wcout << fseat->toString() << L"getRookMoveSeats: " << __seatsStr(seats) << std::endl;
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
    std::wcout << fseat->toString() << L"getCannonMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> Board::getRookCannonMoveSeat_Lines(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> seats(4);
    int frow{ fseat->row() }, fcol{ fseat->col() };
    for (int col = fcol - 1; col >= ColLowIndex; --col)
        seats[0].push_back(__getSeat(frow, col));
    for (int col = fcol + 1; col <= ColUpIndex; ++col)
        seats[1].push_back(__getSeat(frow, col));
    for (int row = frow - 1; row >= RowLowIndex; --row)
        seats[2].push_back(__getSeat(row, fcol));
    for (int row = frow + 1; row <= RowUpIndex; ++row)
        seats[3].push_back(__getSeat(row, fcol));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getPawnMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() }, row{}, col{};
    std::shared_ptr<SeatSpace::Seat> tseat{};
    if ((col = fcol - 1) >= ColLowIndex && (tseat = __getSeat(frow, col))->isDiffColor(fseat))
        seats.push_back(tseat);
    if ((col = fcol + 1) <= ColUpIndex && (tseat = __getSeat(frow, col))->isDiffColor(fseat))
        seats.push_back(tseat);
    if (__isBottomSide(fseat->piece()->color())) {
        if ((row = frow + 1) <= RowUpIndex && (tseat = __getSeat(row, fcol))->isDiffColor(fseat))
            seats.push_back(tseat);
    } else {
        if ((row = frow - 1) >= RowLowIndex && (tseat = __getSeat(row, fcol))->isDiffColor(fseat))
            seats.push_back(tseat);
    }
    std::wcout << fseat->toString() << L"getPawnMoveSeats: " << __seatsStr(seats) << std::endl;
    return seats;
}

const std::wstring movChars{ L"退平进" };
const std::map<PieceColor, std::wstring> numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
const std::wstring nameChars{ L"帅将仕士相象马车炮兵卒" };

const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats()
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = 0; row < RowNum; ++row)
        for (int col = 0; col < ColNum; ++col)
            seats.push_back(std::make_shared<SeatSpace::Seat>(row, col));
    return seats;
}

const wchar_t getChar(const PieceColor color, const int index) { return numChars.at(color)[index]; };

const std::wstring getPreChars(const int length)
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

const PieceColor getColor(const wchar_t numZh)
{
    return numChars.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const wchar_t getIndexChar(const int length, const bool isBottom, const int index)
{
    return getPreChars(length)[isBottom ? length - index - 1 : index];
}

const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isForward)
{
    return movChars.at(isSameRow ? 1 : (isBottom == isForward ? 2 : 0));
}

const wchar_t getNumChar(const PieceColor color, const int num)
{
    return getChar(color, num - 1);
};

const wchar_t getColChar(const PieceColor color, bool isBottom, const int col)
{
    return getChar(color, isBottom ? ColNum - col - 1 : col);
};

const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index{ static_cast<int>(getPreChars(seatsLen).find(preChar)) };
    return isBottom ? seatsLen - index - 1 : index;
}

const int getMovDir(const bool isBottom, const wchar_t movChar)
{
    return static_cast<int>(movChars.find(movChar) - 1) * (isBottom ? 1 : -1);
}

const int getNum(const PieceColor color, const wchar_t numChar)
{
    return static_cast<int>(numChars.at(color).find(numChar)) + 1;
}

const int getCol(bool isBottom, const int num)
{
    return isBottom ? ColNum - num : num - 1;
}

const std::wstring __seatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats)
{
    std::wstringstream wss{};
    for (const auto& seat : seats)
        wss << seat->toString() << L' ';
    return wss.str();
}

const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces()
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

const bool isKing(const wchar_t name) { return nameChars.substr(0, 2).find(name) != std::wstring::npos; }
const bool isAdvBish(const wchar_t name) { return nameChars.substr(2, 4).find(name) != std::wstring::npos; }
const bool isStronge(const wchar_t name) { return nameChars.substr(6, 5).find(name) != std::wstring::npos; }
const bool isLineMove(const wchar_t name) { return isKing(name) || nameChars.substr(7, 4).find(name) != std::wstring::npos; }
const bool isPawn(const wchar_t name) { return nameChars.substr(nameChars.size() - 2, 2).find(name) != std::wstring::npos; }
const bool isPieceName(const wchar_t name) { return nameChars.find(name) != std::wstring::npos; }

const std::wstring Board::test()
{
    std::wstringstream wss{};
    // Piece test
    for (auto& pie : pieces_)
        wss << pie->toString() << L' ';
    wss << L"\n";

    // Board test
    wss << L"getAllSeats(): " << __seatsStr(getAllSeats()) << L'\n';
    //*
    for (auto color : { PieceColor::RED, PieceColor::BLACK }) {
        wss << L"PieceColor:: " << static_cast<int>(color) << L'\n';
        wss << L"getKingSeat(): " << getKingSeat(color)->toString() << L'\n';
        wss << L"getKingSeats(): " << __seatsStr(getKingSeats(color)) << L'\n';
        wss << L"getAdvisorSeats(): " << __seatsStr(getAdvisorSeats(color)) << L'\n';
        wss << L"getBishopSeats(): " << __seatsStr(getBishopSeats(color)) << L'\n';
        wss << L"getPawnSeats(): " << __seatsStr(getPawnSeats(color)) << L'\n';
        wss << L"__getLiveSeats(): " << __seatsStr(__getLiveSeats(color)) << L'\n';
    }
    wss << L"__getLiveSeats(): " << __seatsStr(__getLiveSeats(PieceColor::BLANK)) << L'\n';
    //*/
    auto getSeatsStr = [&](void) {
        for (auto color : { PieceColor::RED, PieceColor::BLACK })
            for (auto fseat : __getLiveSeats(color)) {
                wss << fseat->toString() << L':';
                for (const auto& seat : moveSeats(fseat))
                    wss << seat->toString() << L' ';
                wss << L'\n';
            }
    };
    getSeatsStr();
    for (const auto chg : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) {
        changeSide(chg);
        //getSeatsStr();
    }
    return wss.str();
}
}