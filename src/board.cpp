#include "board.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <cassert>
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

const std::wstring Board::getPieceChars() const
{
    std::wstringstream wss{};
    std::shared_ptr<PieceSpace::Piece> pie{};
    for_each(seats_.begin(), seats_.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << ((pie = seat->piece()) ? pie->ch() : nullChar); });
    return wss.str();
}

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeatFromIccs(const std::wstring& ICCS) const
{
    std::string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(iccs.at(1) - 48, iccs.at(0) - 97), getSeat(iccs.at(3) - 48, iccs.at(2) - 97)); // 0:48, a:97
}

const std::wstring Board::getIccs(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat) const
{
    std::wstringstream wss{};
    const std::wstring ColChars{ L"abcdefghi" };
    wss << ColChars.at(fseat->col()) << fseat->row() << ColChars.at(tseat->col()) << tseat->row();
    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeatFromZh(const std::wstring& zhStr) const
{
    assert(zhStr.size() == 4);
    std::shared_ptr<SeatSpace::Seat> fseat{}, tseat{};
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ __getColor(zhStr.back()) };
    bool isBottom{ __isBottomSide(color) };
    int index{}, movDir{ getMovNum(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (isPieceName(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name, getCol(isBottom, getNum(color, zhStr.at(1))));
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = isPawn(name) ? __sortPawnSeats(color, name) : __getLiveSeats(color, name);
        index = getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= static_cast<int>(seats.size()) - 1);
    fseat = seats.at(index);
    int num{ getNum(color, zhStr.back()) }, toCol{ getCol(isBottom, num) };
    if (isLineMove(name)) {
        int trow{ fseat->row() + movDir * num };
        assert(movDir == 0 || (trow <= RowUpIndex && trow >= RowLowIndex));
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(trow, fseat->col());
    } else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }, //  相距1或2列
            trow{ fseat->row() + movDir * (isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
        assert(trow <= RowUpIndex && trow >= RowLowIndex);
        tseat = getSeat(trow, toCol);
    }
    assert(zhStr == getZh(fseat, tseat));

    return make_pair(fseat, tseat);
}

//(fseat, tseat)->中文纵线着法
const std::wstring Board::getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat) const
{
    std::wstringstream wss{};
    const std::shared_ptr<PieceSpace::Piece>& fromPiece{ fseat->piece() };
    const PieceColor color{ fromPiece->color() };
    const wchar_t name{ fromPiece->name() };
    const int fromRow{ fseat->row() }, fromCol{ fseat->col() }, toRow{ tseat->row() }, toCol{ tseat->col() };
    bool isSameRow{ fromRow == toRow }, isBottom{ __isBottomSide(color) };
    auto seats = __getLiveSeats(color, name, fromCol);

    if (seats.size() > 1 && isStronge(name)) {
        if (isPawn(name))
            seats = __sortPawnSeats(color, name);
        wss << getIndexChar(seats.size(), isBottom, distance(seats.begin(), find(seats.begin(), seats.end(), fseat))) << name;
    } else //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << getColChar(color, isBottom, fromCol);
    wss << getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (isLineMove(name) && !isSameRow ? getNumChar(color, abs(fromRow - toRow)) : getColChar(color, isBottom, toCol));

    //auto& mvSeats = getMoveSeatFromZh(wss.str());
    //assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return wss.str();
}

//判断是否将军
const bool Board::isKilled(const PieceColor color) const
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    std::shared_ptr<SeatSpace::Seat> kingSeat{ __getKingSeat(color) }, othKingSeat{ __getKingSeat(othColor) };
    int fcol{ kingSeat->col() };
    if (fcol == othKingSeat->col()) {
        bool isBottom{ __isBottomSide(color) };
        int uprow{ isBottom ? othKingSeat->row() : kingSeat->row() };
        std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
        for (int row = (isBottom ? kingSeat->row() : othKingSeat->row()) + 1; row < uprow; ++row)
            seats.push_back(getSeat(row, fcol));
        if (std::all_of(seats.begin(), seats.end(),
                [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return !seat->piece(); }))
            return true;
    }
    // '获取某方可杀将棋子全部可走的位置
    std::vector<std::shared_ptr<SeatSpace::Seat>> strongeMoveSeats{}, liveSeats{ __getLiveSeats(othColor) };
    std::for_each(liveSeats.begin(), liveSeats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& fseat) {
        if (isStronge(fseat->piece()->name())) {
            auto& mvSeats = fseat->piece()->__moveSeats(*this, fseat);
            copy(mvSeats.begin(), mvSeats.end(), std::back_inserter(strongeMoveSeats));
        }
    });
    if (std::any_of(strongeMoveSeats.begin(), strongeMoveSeats.end(),
            [&](const std::shared_ptr<SeatSpace::Seat>& tseat) { return tseat == kingSeat; }))
        return true;
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color) const
{
    for (auto fseat : __getLiveSeats(color))
        if (!moveSeats(fseat).empty())
            return false;
    return true;
}

// '获取棋子可走的位置, 不能被将军'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    assert(fseat->piece());
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    auto color = fseat->piece()->color();
    auto pieMoveSeats = fseat->piece()->__moveSeats(*this, fseat);
    std::copy_if(pieMoveSeats.begin(), pieMoveSeats.end(), std::back_inserter(seats),
        [&](std::shared_ptr<SeatSpace::Seat>& tseat) {
            auto& eatPiece = fseat->movTo(tseat);
            // 移动棋子后，检测是否会被对方将军
            bool killed{ isKilled(color) };
            tseat->movTo(fseat, eatPiece);
            return !killed;
        });
    return seats;
}

void Board::reset(const std::wstring& pieceChars)
{
    assert(seats_.size() == pieceChars.size());
    wchar_t ch{};
    int chIndex{ -1 };
    std::vector<bool> used(pieces_.size(), false);
    std::for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        if ((ch = pieceChars[++chIndex]) != nullChar) {
            for (int index = used.size() - 1; index >= 0; --index)
                if (!used[index] && pieces_[index]->ch() == ch) {
                    seat->put(pieces_[index]);
                    used[index] = true;
                    break;
                }
        } else
            seat->put();
    });
    __setBottomSide();
}

void Board::changeSide(const ChangeType ct)
{
    std::vector<std::shared_ptr<PieceSpace::Piece>> seatPieces{};
    auto getOthPiece = [&](const std::shared_ptr<PieceSpace::Piece>& piece) {
        return pieces_.at((std::distance(pieces_.begin(), std::find(pieces_.begin(), pieces_.end(), piece)) + 16) % 32);
    };
    auto changeRowcol = std::mem_fn(ct == ChangeType::ROTATE ? &Board::getRotate : &Board::getSymmetry);
    std::for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        seatPieces.push_back(ct == ChangeType::EXCHANGE ? (seat->piece() ? getOthPiece(seat->piece()) : nullptr)
                                                        : getSeat(changeRowcol(this, seat->rowcol()))->piece());
    });
    int index{ -1 };
    std::for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        seat->put(seatPieces.at(++index));
    });
    __setBottomSide();
}

void Board::__setBottomSide()
{
    auto pos = std::find_if(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        return seat->piece() == pieces_[0];
    }); // 定位红帅
    assert(pos != seats_.end());
    bottomColor = (*pos)->row() < RowLowUpIndex ? PieceColor::RED : PieceColor::BLACK;
}

const std::shared_ptr<SeatSpace::Seat>& Board::__getKingSeat(const PieceColor color) const
{
    auto& seats = getKingSeats(color);
    auto pos = std::find_if(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        return seat->piece() && isKing(seat->piece()->name());
    });
    assert(pos != seats.end());
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
            seats.push_back(getSeat(row, col));
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
                seats.push_back(getSeat(row, col));
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
                seats.push_back(getSeat(row, col));
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
    for (int row = lfrow; row <= ufrow; ++row)
        for (int col = ColLowIndex; col <= ColUpIndex; col += 2)
            seats.push_back(getSeat(row, col));
    for (int row = ltrow; row <= utrow; ++row)
        for (int col = ColLowIndex; col <= ColUpIndex; ++col)
            seats.push_back(getSeat(row, col));
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (((frow == tseat->row() && abs(fcol - tseat->col()) == 1)
                        || (abs(frow - tseat->row()) == 1 && fcol == tseat->col()))
                && tseat->isDiffColor(fseat));
        });
    //std::wcout << fseat->toString() << L"getKingMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvsiorMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(frow - tseat->row()) == 1
                && abs(fcol - tseat->col()) == 1
                && tseat->isDiffColor(fseat));
        });
    //std::wcout << fseat->toString() << L"getAdvisorMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> pieceSeats{ fseat->piece()->getSeats(*this) }, seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    std::copy_if(pieceSeats.begin(), pieceSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            return (abs(frow - tseat->row()) == 2
                && abs(fcol - tseat->col()) == 2
                && !getSeat((frow + tseat->row()) >> 1, (fcol + tseat->col()) >> 1)->piece()
                && tseat->isDiffColor(fseat));
        });
    //std::wcout << fseat->toString() << L"getBishopMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    std::copy_if(seats_.begin(), seats_.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& tseat) {
            int trow{ tseat->row() }, tcol{ tseat->col() }, irow{ frow > trow ? -1 : 1 }, icol{ fcol > tcol ? -1 : 1 };
            return (((abs(frow - trow) == 1 && abs(fcol - tcol) == 2 && !getSeat(frow, fcol + icol)->piece())
                        || (abs(frow - trow) == 2 && abs(fcol - tcol) == 1 && !getSeat(frow + irow, fcol)->piece()))
                && tseat->isDiffColor(fseat));
        });

    assert(seats.size() <= 8);
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getRookMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& seatLine : __getRookCannonMoveSeat_Lines(fseat))
        for (auto& tseat : seatLine) {
            if (tseat->isDiffColor(fseat))
                seats.push_back(tseat);
            if (tseat->piece())
                break;
        }
    //std::wcout << fseat->toString() << L"getRookMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getCannonMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& seatLine : __getRookCannonMoveSeat_Lines(fseat)) {
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
    //std::wcout << fseat->toString() << L"getCannonMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> Board::__getRookCannonMoveSeat_Lines(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> seatLines(4);
    int frow{ fseat->row() }, fcol{ fseat->col() };
    for (int col = fcol - 1; col >= ColLowIndex; --col)
        seatLines[0].push_back(getSeat(frow, col));
    for (int col = fcol + 1; col <= ColUpIndex; ++col)
        seatLines[1].push_back(getSeat(frow, col));
    for (int row = frow - 1; row >= RowLowIndex; --row)
        seatLines[2].push_back(getSeat(row, fcol));
    for (int row = frow + 1; row <= RowUpIndex; ++row)
        seatLines[3].push_back(getSeat(row, fcol));
    return seatLines;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getPawnMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    std::vector<std::pair<int, int>> trowcols{};
    int frow{ fseat->row() }, fcol{ fseat->col() }, row{}, col{};
    bool isBottom{ __isBottomSide(fseat->piece()->color()) };
    if ((isBottom && (row = frow + 1) <= RowUpIndex)
        || (!isBottom && (row = frow - 1) >= RowLowIndex))
        trowcols.push_back({ row, fcol });
    if (isBottom == (frow > RowLowUpIndex)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex)
            trowcols.push_back({ frow, col });
        if ((col = fcol + 1) <= ColUpIndex)
            trowcols.push_back({ frow, col });
    }
    std::for_each(trowcols.begin(), trowcols.end(), [&](std::pair<int, int>& trowcol) {
        auto& seat = getSeat(trowcol.first, trowcol.second);
        if (seat->isDiffColor(fseat))
            seats.push_back(seat);
    });
    //std::wcout << fseat->toString() << L"getPawnMoveSeats: " << __getSeatsStr(seats) << std::endl;
    return seats;
}

const std::vector<std::shared_ptr<PieceSpace::Piece>> Board::creatPieces() const
{
    return std::vector<std::shared_ptr<PieceSpace::Piece>>{
        std::make_shared<PieceSpace::King>(L'K', nameChars.at(0)),
        std::make_shared<PieceSpace::Advisor>(L'A', nameChars.at(2)),
        std::make_shared<PieceSpace::Advisor>(L'A', nameChars.at(2)),
        std::make_shared<PieceSpace::Bishop>(L'B', nameChars.at(4)),
        std::make_shared<PieceSpace::Bishop>(L'B', nameChars.at(4)),
        std::make_shared<PieceSpace::Knight>(L'N', nameChars.at(6)),
        std::make_shared<PieceSpace::Knight>(L'N', nameChars.at(6)),
        std::make_shared<PieceSpace::Rook>(L'R', nameChars.at(7)),
        std::make_shared<PieceSpace::Rook>(L'R', nameChars.at(7)),
        std::make_shared<PieceSpace::Cannon>(L'C', nameChars.at(8)),
        std::make_shared<PieceSpace::Cannon>(L'C', nameChars.at(8)),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9)),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9)),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9)),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9)),
        std::make_shared<PieceSpace::Pawn>(L'P', nameChars.at(9)),
        std::make_shared<PieceSpace::King>(L'k', nameChars.at(1)),
        std::make_shared<PieceSpace::Advisor>(L'a', nameChars.at(3)),
        std::make_shared<PieceSpace::Advisor>(L'a', nameChars.at(3)),
        std::make_shared<PieceSpace::Bishop>(L'b', nameChars.at(5)),
        std::make_shared<PieceSpace::Bishop>(L'b', nameChars.at(5)),
        std::make_shared<PieceSpace::Knight>(L'n', nameChars.at(6)),
        std::make_shared<PieceSpace::Knight>(L'n', nameChars.at(6)),
        std::make_shared<PieceSpace::Rook>(L'r', nameChars.at(7)),
        std::make_shared<PieceSpace::Rook>(L'r', nameChars.at(7)),
        std::make_shared<PieceSpace::Cannon>(L'c', nameChars.at(8)),
        std::make_shared<PieceSpace::Cannon>(L'c', nameChars.at(8)),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10)),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10)),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10)),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10)),
        std::make_shared<PieceSpace::Pawn>(L'p', nameChars.at(10))
    };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::creatSeats() const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = 0; row < RowNum; ++row)
        for (int col = 0; col < ColNum; ++col)
            seats.push_back(std::make_shared<SeatSpace::Seat>(row, col));
    return seats;
}

// '多兵排序'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__sortPawnSeats(const PieceColor color, const wchar_t name) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> pawnSeats{ __getLiveSeats(color, name) }, seats{}; // 最多5个兵
    bool isBottom{ __isBottomSide(color) };
    // 按列建立字典，按列排序
    std::map<int, std::vector<std::shared_ptr<SeatSpace::Seat>>> colSeats{};
    for_each(pawnSeats.begin(), pawnSeats.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            colSeats[isBottom ? -seat->col() : seat->col()].push_back(seat);
        }); // 底边则列倒序,每列位置倒序

    // 整合成一个数组
    for_each(colSeats.begin(), colSeats.end(),
        [&](const std::pair<int, std::vector<std::shared_ptr<SeatSpace::Seat>>>& colSeat) {
            if (colSeat.second.size() > 1) // 筛除只有一个位置的列
                copy(colSeat.second.begin(), colSeat.second.end(), std::back_inserter(seats));
        }); //按列存入
    return seats;
}

const PieceColor Board::__getColor(const wchar_t numZh) const
{
    return numChars.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats() const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    std::copy_if(seats_.begin(), seats_.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return bool(seat->piece()); }); // 活的棋子
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats(const PieceColor color) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> liveSeats{ __getLiveSeats() }, seats{};
    std::copy_if(liveSeats.begin(), liveSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return color == seat->piece()->color(); });
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats(const PieceColor color, const wchar_t name) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> colorSeats{ __getLiveSeats(color) }, seats{};
    std::copy_if(colorSeats.begin(), colorSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return name == seat->piece()->name(); });
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveSeats(const PieceColor color, const wchar_t name, const int col) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> colorNameSeats{ __getLiveSeats(color, name) }, seats{};
    std::copy_if(colorNameSeats.begin(), colorNameSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return col == seat->col(); });
    return seats;
}

const std::wstring Board::__getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats) const
{
    std::wstringstream wss{};
    wss << seats.size() << L"个: ";
    std::for_each(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << seat->toString() << L' '; });
    return wss.str();
}

//const wchar_t Board::nullChar;
const std::wstring Board::nameChars{ L"帅将仕士相象马车炮兵卒" };
const std::wstring Board::movChars{ L"退平进" };
const std::map<PieceColor, std::wstring> Board::numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};

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
    for (auto& seat : __getLiveSeats())
        textBlankBoard[(ColNum - seat->row()) * 2 * (ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const std::wstring Board::test()
{
    std::wstringstream wss{};
    // Piece test
    wss << L"全部棋子" << pieces_.size() << L"个:";
    for (auto& pie : pieces_)
        wss << pie->toString() << L' ';
    wss << L"\n";

    // Board test
    for (const auto& fen : { L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
             L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" }) {
        auto pieceChars = InstanceSpace::getPieceChars(fen);

        reset(pieceChars);
        wss << "fen:" << fen << "\nget:" << InstanceSpace::getFEN(pieceChars)
            << "\ngetChars:" << pieceChars << "\nboardGet:" << getPieceChars() << L'\n';

        //*
        std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
        wss << L"getAllSeats() " << __getSeatsStr(getAllSeats()) << L'\n';
        wss << L"__getLiveSeats() " << __getSeatsStr(__getLiveSeats()) << L"\n";
        for (auto color : { PieceColor::RED, PieceColor::BLACK }) {
            wss << L"PieceColor: " << static_cast<int>(color) << L'\n';
            wss << L"__getKingSeat() " << __getKingSeat(color)->toString() << L'\n';
            wss << L"getKingSeats() " << __getSeatsStr(getKingSeats(color)) << L'\n';
            wss << L"getAdvisorSeats() " << __getSeatsStr(getAdvisorSeats(color)) << L'\n';
            wss << L"getBishopSeats() " << __getSeatsStr(getBishopSeats(color)) << L'\n';
            wss << L"getPawnSeats() " << __getSeatsStr(getPawnSeats(color)) << L'\n';
            wss << L"__getLiveSeats() " << __getSeatsStr(__getLiveSeats(color)) << L"\n";
        }
        //*/
        //*
        auto getLiveSeatsStr = [&](void) {
            wss << L"\nbottomColor: " << static_cast<int>(bottomColor) << L'\n' << toString();
            for (auto color : { PieceColor::RED, PieceColor::BLACK })
                for (auto fseat : __getLiveSeats(color))
                    wss << fseat->toString() << L"=>" << __getSeatsStr(moveSeats(fseat)) << L'\n';
        };
        getLiveSeatsStr();
        //*/
        //*
        for (const auto chg : { ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) { //
            changeSide(chg);
            getLiveSeatsStr();
        }
        //*/
        wss << L"\n";
    }
    return wss.str();
}
}