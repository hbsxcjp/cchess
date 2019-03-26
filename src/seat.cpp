#include "seat.h"
#include "board.h"
#include "piece.h"
#include <iomanip>
#include <sstream>
#include <string>

namespace SeatSpace {

const bool Seat::isSameColor(const std::shared_ptr<PieceSpace::Piece>& piece) { return piece->color() == piece_->color(); }

// '获取棋子可走的位置, 不能被将军'
const std::vector<std::shared_ptr<Seat>> Seat::getMoveSeats(BoardSpace::Board& board)
{
    std::vector<std::shared_ptr<Seat>> seats{};
    auto fseat = shared_from_this();
    for (auto& tseat : piece_->filterSelfMoveSeats(board, shared_from_this())) {
        auto eatPiece = SeatSpace::move(fseat, tseat, PieceSpace::nullPiece);
        // 移动棋子后，检测是否会被对方将军
        if (!board.isKilled(piece_->color()))
            seats.push_back(tseat);
        SeatSpace::move(tseat, fseat, eatPiece);
    }
    return seats;
}

const std::wstring Seat::toString() const
{
    std::wstringstream wss{};
    wss << std::boolalpha;
    wss << std::setw(2) << row_ << std::setw(2) << col_ << std::setw(2) << piece_->name();
    return wss.str();
}

const std::shared_ptr<PieceSpace::Piece>& move(std::shared_ptr<Seat>& fseat, std::shared_ptr<Seat>& tseat,
    const std::shared_ptr<PieceSpace::Piece>& fillPiece)
{
    auto& eatPiece = tseat->piece();
    tseat->put(fseat->piece());
    fseat->put(fillPiece);
    return eatPiece;
}

/*
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <codecvt>
#include <cstdlib>
#include <direct.h>
#include <fstream>
#include <functional>
#include <io.h>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <sstream>
#include <std::vector>
#include <string>
#include <utility>

using namespace std;
using namespace std::chrono;
using namespace Board_base;

Seat::Seat(int col, int row)
    : c(col)
    , r(row)
{
}

Seats::Seats()
    : seats(ColNum, std::vector<Seat>(RowNum, Seat(-1, -1)))
{
    for (int c = MinCol; c <= MaxCol; ++c)
        for (int r = MinRow; r <= MaxRow; ++r)
            seats[c][r] = Seat(c, r);
}

const std::vector<const Seat*> Seats::allSeats() const
{
    std::vector<const Seat*> seatv{};
    for (int c = MinCol; c <= MaxCol; ++c)
        for (int r = MinRow; r <= MaxRow; ++r)
            seatv.push_back(&getSeat(c, r));
    return seatv;
}

const std::vector<const Seat*> Seats::kingSeats(BoardSide bs) const
{
    std::vector<const Seat*> seatv{};
    int lrow{ bs == BoardSide::BOTTOM ? MinRow : 7 }, urow{ bs == BoardSide::BOTTOM ? 2 : MaxRow };
    for (int col = 3; col != 6; ++col)
        for (int row = lrow; row != urow; ++row)
            seatv.push_back(&getSeat(col, row));
    return seatv;
}

const std::vector<const Seat*> Seats::advisorSeats(BoardSide bs) const
{
    std::vector<const Seat*> seatv{};
    int lrow{ bs == BoardSide::BOTTOM ? MinRow : 7 }, urow{ bs == BoardSide::BOTTOM ? 2 : MaxRow }, rmd{ bs == BoardSide::BOTTOM ? 1 : 0 }; // 行列和的奇偶值
    for (int col = 3; col != 6; ++col)
        for (int row = lrow; row != urow; ++row)
            if ((col + row) % 2 == rmd)
                seatv.push_back(&getSeat(col, row));
    return seatv;
}

const std::vector<const Seat*> Seats::bishopSeats(BoardSide bs) const
{
    std::vector<const Seat*> seatv{};
    int lrow{ bs == BoardSide::BOTTOM ? MinRow : 5 }, urow{ bs == BoardSide::BOTTOM ? 4 : MaxRow };
    for (int col = MinCol; col != MaxCol; col += 2)
        for (int row = lrow; row != urow; row += 2) {
            int gap{ row - col };
            if ((bs == BoardSide::BOTTOM && (gap == 2 || gap == -2 || gap == -6))
                || (bs == BoardSide::TOP && (gap == 7 || gap == 3 || gap == -1)))
                seatv.push_back(&getSeat(col, row));
        }
    return seatv;
}

const std::vector<const Seat*> Seats::pawnSeats(BoardSide bs) const
{
    std::vector<const Seat*> seatv{};
    int lfrow{ bs == BoardSide::BOTTOM ? 3 : 5 }, ufrow{ bs == BoardSide::BOTTOM ? 4 : 6 },
        ltrow{ bs == BoardSide::BOTTOM ? 5 : MinRow }, utrow{ bs == BoardSide::BOTTOM ? MaxRow : 4 };
    for (int col = MinCol; col <= MaxCol; col += 2)
        for (int row = lfrow; row <= ufrow; ++row)
            seatv.push_back(&getSeat(col, row));
    for (int col = MinCol; col <= MaxCol; ++col)
        for (int row = ltrow; row <= utrow; ++row)
            seatv.push_back(&getSeat(col, row));
    return seatv;
}

const std::vector<const Seat*> Seats::getSameColSeats(const Seat& aseat, const Seat& bseat) const
{
    std::vector<const Seat*> seatv{};
    if (!isSameCol(aseat, bseat))
        return seatv;

    int col{ aseat.c }, lrow{ min(aseat.r, bseat.r) }, urow{ max(aseat.r, bseat.r) };
    for (int r = lrow + 1; r != urow; ++r)
        seatv.push_back(&getSeat(col, r));
    return seatv;
}

// 位置行走函数
const std::vector<const Seat*> Seats::getKingMoveSeats(const Seat& seat) const
{
    int row{ seat.r }, col{ seat.c };
    if (col == 3) {
        if (row == MinRow || row == 7)
            return (std::vector<const Seat*>{ &getSeat(col + 1, row), &getSeat(col, row + 1) });
        else if (row == 1 || row == 8)
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col + 1, row), &getSeat(col, row + 1) });
        else
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col + 1, row) });
    } else if (col == 4) {
        if (row == MinRow || row == 7)
            return (std::vector<const Seat*>{ &getSeat(col - 1, row), &getSeat(col + 1, row), &getSeat(col, row + 1) });
        else if (row == 1 || row == 8)
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col - 1, row), &getSeat(col + 1, row), &getSeat(col, row + 1) });
        else
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col - 1, row), &getSeat(col + 1, row) });
    } else {
        if (row == MinRow || row == 7)
            return (std::vector<const Seat*>{ &getSeat(col - 1, row), &getSeat(col, row + 1) });
        else if (row == 1 || row == 8)
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col - 1, row), &getSeat(col, row + 1) });
        else
            return (std::vector<const Seat*>{ &getSeat(col, row - 1), &getSeat(col - 1, row) });
    }
}

const std::vector<const Seat*> Seats::getAdvisorMoveSeats(const Seat& seat) const
{
    int row{ seat.r }, col{ seat.c };
    if (col == 3) {
        if (row == MinRow || row == 7)
            return (std::vector<const Seat*>{ &getSeat(col + 1, row + 1) });
        else
            return (std::vector<const Seat*>{ &getSeat(col + 1, row - 1) });
    } else if (col == 4) {
        return (std::vector<const Seat*>{ &getSeat(col + 1, row - 1), &getSeat(col - 1, row - 1),
            &getSeat(col - 1, row + 1), &getSeat(col + 1, row + 1) });
    } else {
        if (row == 0 || row == 7)
            return (std::vector<const Seat*>{ &getSeat(col - 1, row + 1) });
        else
            return (std::vector<const Seat*>{ &getSeat(col - 1, row - 1) });
    }
}

// 获取移动、象心行列值
const std::vector<pair<const Seat*, const Seat*>> Seats::getBishopMove_CenSeats(const Seat& seat) const
{
    std::vector<pair<const Seat*, const Seat*>> seatsv{};
    function<const Seat&(const Seat&)> __cenSeat = [&](const Seat& st) { return getSeat((seat.c + st.c) / 2, (seat.r + st.r) / 2); };

    std::vector<pair<int, int>> vcr{ { seat.c - 2, seat.r - 2 }, { seat.c - 2, seat.r + 2 },
        { seat.c + 2, seat.r - 2 }, { seat.c + 2, seat.r + 2 } };
    for (auto& cr : vcr)
        if (cr.first >= MinCol && cr.first <= MaxCol && cr.second >= MinRow && cr.second <= MaxRow) {
            const Seat aSeat{ getSeat(cr.first, cr.second) };
            seatsv.push_back(make_pair(&aSeat, &__cenSeat(aSeat)));
        }
    return seatsv;
}

// 获取移动、马腿行列值
const std::vector<pair<const Seat*, const Seat*>> Seats::getKnightMove_LegSeats(const Seat& seat) const
{
    std::vector<pair<const Seat*, const Seat*>> seatsv{};
    function<const Seat&(const Seat&)> __legSeat = [&](const Seat& st) {
        int acol{ seat.c }, arow{ seat.r }, gap{ st.r - arow };
        if (gap == 2)
            ++arow;
        else if (gap == -2)
            --arow;
        else if (st.c - acol == 2)
            ++acol;
        else
            --acol;
        return getSeat(acol, arow);
    };

    std::vector<pair<int, int>> vcr{
        { seat.c - 2, seat.r - 1 }, { seat.c - 2, seat.r + 1 },
        { seat.c + 2, seat.r - 1 }, { seat.c + 2, seat.r + 1 },
        { seat.c - 1, seat.r - 2 }, { seat.c - 1, seat.r + 2 },
        { seat.c + 1, seat.r - 2 }, { seat.c + 1, seat.r + 2 }
    };
    for (auto& cr : vcr)
        if (cr.first >= MinCol && cr.first <= MaxCol && cr.second >= MinRow && cr.second <= MaxRow) {
            const Seat aSeat{ getSeat(cr.first, cr.second) };
            seatsv.push_back(make_pair(&aSeat, &__legSeat(aSeat)));
        }
    return seatsv;
}

// 车炮可走的四个方向位置
const std::vector<std::vector<const Seat*>> Seats::getRookCannonMoveSeat_Lines(const Seat& seat) const
{
    std::vector<std::vector<const Seat*>> vseatv{};

    return vseatv;
}

const std::vector<const Seat*> Seats::getPawnMoveSeats(const bool isBottomSide, const Seat& seat) const
{
    std::vector<const Seat*> seatv{};

    return seatv;
}

// '多兵排序'
const std::vector<const Seat*> Seats::sortPawnSeats(const bool isBottomSide, std::vector<Seat> pawnSeats) const
{
    std::vector<const Seat*> seatv{};

    return seatv;
}
*/

////

/*

const std::vector<int> Board_base::getKingMoveSeats(const int seat)
{
    int S{ seat - 9 }, W{ seat - 1 }, E{ seat + 1 }, N{ seat + 9 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == 3) {
        if (row == 0 || row == 7)
            return (std::vector<int>{ E, N });
        else if (row == 1 || row == 8)
            return (std::vector<int>{ S, E, N });
        else
            return (std::vector<int>{ S, E });
    } else if (col == 4) {
        if (row == 0 || row == 7)
            return (std::vector<int>{ W, E, N });
        else if (row == 1 || row == 8)
            return (std::vector<int>{ S, W, E, N });
        else
            return (std::vector<int>{ S, W, E });
    } else {
        if (row == 0 || row == 7)
            return (std::vector<int>{ W, N });
        else if (row == 1 || row == 8)
            return (std::vector<int>{ S, W, N });
        else
            return (std::vector<int>{ S, W });
    }
}

const std::vector<int> Board_base::getAdvisorMoveSeats(const int seat)
{
    int WS{ seat - 9 - 1 }, ES{ seat - 9 + 1 }, WN{ seat + 9 - 1 }, EN{ seat + 9 + 1 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == 3) {
        if (row == 0 || row == 7)
            return (std::vector<int>{ EN });
        else
            return (std::vector<int>{ ES });
    } else if (col == 4) {
        return (std::vector<int>{ WS, ES, WN, EN });
    } else {
        if (row == 0 || row == 7)
            return (std::vector<int>{ WN });
        else
            return (std::vector<int>{ WS });
    }
}

// 获取移动、象心行列值
const std::vector<pair<int, int>> Board_base::getBishopMove_CenSeats(const int seat)
{
    auto cen = [seat](int s) { return (seat + s) / 2; };

    int EN{ seat + 2 * 9 + 2 }, ES{ seat - 2 * 9 + 2 }, WS{ seat - 2 * 9 - 2 },
        WN{ seat + 2 * 9 - 2 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == MaxCol)
        return (std::vector<pair<int, int>>{ { WS, cen(WS) }, { WN, cen(WN) } });
    else if (col == MinCol)
        return (std::vector<pair<int, int>>{ { EN, cen(EN) }, { ES, cen(ES) } });
    else if (row == 0 || row == 5)
        return (std::vector<pair<int, int>>{ { WN, cen(WN) }, { EN, cen(EN) } });
    else if (row == 4 || row == 9)
        return (std::vector<pair<int, int>>{ { WS, cen(WS) }, { ES, cen(ES) } });
    else
        return (std::vector<pair<int, int>>{
            { WS, cen(WS) }, { WN, cen(WN) }, { ES, cen(ES) }, { EN, cen(EN) } });
}

// 获取移动、马腿行列值
const std::vector<pair<int, int>> Board_base::getKnightMove_LegSeats(const int seat)
{
    auto leg = [seat](int to) {
        switch (to - seat) {
        case 17:
        case 19:
            return seat + 9;
        case -17:
        case -19:
            return seat - 9;
        case 11:
        case -7:
            return seat + 1;
        default:
            return seat - 1;
        }
    };

    int EN{ seat + 11 }, ES{ seat - 7 }, SE{ seat - 17 }, SW{ seat - 19 },
        WS{ seat - 11 }, WN{ seat + 7 }, NW{ seat + 17 }, NE{ seat + 19 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    switch (col) {
    case MaxCol:
        switch (row) {
        case MaxRow:
            return (std::vector<pair<int, int>>{ { WS, leg(WS) }, { SW, leg(SW) } });
        case MaxRow - 1:
            return (std::vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { WN, leg(WN) } });
        case MinRow:
            return (std::vector<pair<int, int>>{ { WN, leg(WN) }, { NW, leg(NW) } });
        case MinRow + 1:
            return (std::vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { WS, leg(WS) } });
        default:
            return (std::vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { WS, leg(WS) }, { SW, leg(SW) } });
        }
    case MaxCol - 1:
        switch (row) {
        case MaxRow:
            return (std::vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (std::vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) }, { WN, leg(WN) } });
        case MinRow:
            return (std::vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (std::vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { NE, leg(NE) }, { WS, leg(WS) } });
        default:
            return (std::vector<pair<int, int>>{ { WN, leg(WN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WS, leg(WS) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    case MinCol:
        switch (row) {
        case MaxRow:
            return (std::vector<pair<int, int>>{ { ES, leg(ES) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (std::vector<pair<int, int>>{
                { ES, leg(ES) }, { SE, leg(SE) }, { EN, leg(EN) } });
        case MinRow:
            return (std::vector<pair<int, int>>{ { EN, leg(EN) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (std::vector<pair<int, int>>{
                { EN, leg(EN) }, { NE, leg(NE) }, { ES, leg(ES) } });
        default:
            return (std::vector<pair<int, int>>{
                { EN, leg(EN) }, { NE, leg(NE) }, { ES, leg(ES) }, { SE, leg(SE) } });
        }
    case MinCol + 1:
        switch (row) {
        case MaxRow:
            return (std::vector<pair<int, int>>{
                { ES, leg(ES) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (std::vector<pair<int, int>>{
                { ES, leg(ES) }, { SW, leg(SW) }, { SE, leg(SE) }, { EN, leg(EN) } });
        case MinRow:
            return (std::vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (std::vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { NE, leg(NE) }, { ES, leg(ES) } });
        default:
            return (std::vector<pair<int, int>>{ { EN, leg(EN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { ES, leg(ES) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    default:
        switch (row) {
        case MaxRow:
            return (std::vector<pair<int, int>>{
                { ES, leg(ES) }, { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (std::vector<pair<int, int>>{ { ES, leg(ES) },
                { WS, leg(WS) },
                { WN, leg(WN) },
                { SW, leg(SW) },
                { SE, leg(SE) },
                { EN, leg(EN) } });
        case MinRow:
            return (std::vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { WN, leg(WN) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (std::vector<pair<int, int>>{ { EN, leg(EN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WN, leg(WN) },
                { WS, leg(WS) },
                { ES, leg(ES) } });
        default:
            return (std::vector<pair<int, int>>{ { EN, leg(EN) },
                { ES, leg(ES) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WN, leg(WN) },
                { WS, leg(WS) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    }
}

// 车炮可走的四个方向位置
const std::vector<std::vector<int>> Board_base::getRookCannonMoveSeat_Lines(const int seat)
{
    std::vector<std::vector<int>> seats{ std::vector<int>{}, std::vector<int>{}, std::vector<int>{},
        std::vector<int>{} };
    int row{ getRow(seat) }, left{ row * 9 - 1 }, right{ row * 9 + 9 };
    for (int i = seat - 1; i != left; --i)
        seats[0].push_back(i);
    for (int i = seat + 1; i != right; ++i)
        seats[1].push_back(i);
    for (int i = seat - 9; i > -1; i -= 9)
        seats[2].push_back(i);
    for (int i = seat + 9; i < 90; i += 9)
        seats[3].push_back(i);
    return seats;
}

const std::vector<int> Board_base::getPawnMoveSeats(const bool isBottomSide, const int seat)
{
    int E{ seat + 1 }, S{ seat - 9 }, W{ seat - 1 }, N{ seat + 9 }, row{ getRow(seat) };
    switch (getCol(seat)) {
    case MaxCol:
        if (isBottomSide) {
            if (row > 4)
                return (std::vector<int>{ W, N });
            else
                return (std::vector<int>{ N });
        } else {
            if (row < 5)
                return (std::vector<int>{ W, S });
            else
                return (std::vector<int>{ S });
        }
    case MinCol:
        if (isBottomSide) {
            if (row > 4)
                return (std::vector<int>{ E, N });
            else
                return (std::vector<int>{ N });
        } else {
            if (row < 5)
                return (std::vector<int>{ E, S });
            else
                return (std::vector<int>{ S });
        }
    default:
        if (isBottomSide) {
            if (row > 4)
                return (std::vector<int>{ E, W, N });
            else
                return (std::vector<int>{ N });
        } else {
            if (row < 5)
                return (std::vector<int>{ E, W, S });
            else
                return (std::vector<int>{ S });
        }
    }
}

// '多兵排序'
const std::vector<int> Board_base::sortPawnSeats(const bool isBottomSide, std::vector<int> seats)
{
    map<int, std::vector<int>> temp{};
    std::vector<int> seats(5);
    // 按列建立字典，按列排序
    for_each(seats.begin(), seats.end(),
        [&](int s) { temp[getCol(s)].push_back(s); });
    // 筛除只有一个位置的列, 整合成一个数组
    auto pos = seats.begin();
    for_each(temp.begin(), temp.end(), [&](pair<int, std::vector<int>> col_seats) {
        if (col_seats.second.size() > 1) {
            std::sort(col_seats.second.begin(), col_seats.second.end()); // 每列排序
            pos = copy(col_seats.second.begin(), col_seats.second.end(), pos); //按列存入
        }
    });
    seats = std::vector<int>{ seats.begin(), pos };
    if (isBottomSide)
        reverse(seats.begin(), seats.end()); // 根据棋盘顶底位置,是否反序
    return seats;
}

// 测试
const std::wstring Board_base::test()
{
    wstringstream wss{};
    wss << L"test "
           L"board_base.h\n----------------------------------------------------"
           L"-\n";
    wss << boolalpha << L"NumCols: " << L"ColNum " << ColNum << L" RowNum " << RowNum
        << L" MinCol " << MinCol << L" MaxCol " << MaxCol << L'\n';
    wss << L"multiSeats:\n";
    // std::vector<const std::vector<int>> multiSeats = ;
    for (auto aseats : { allSeats, bottomKingSeats, topKingSeats,
             bottomAdvisorSeats, topAdvisorSeats, bottomBishopSeats,
             topBishopSeats, bottomPawnSeats, topPawnSeats }) {
        for (auto seat : aseats)
            wss << std::setw(2) << seat << L' ';
        wss << L'\n';
    }
    
    wss << L"allSeats rotateSeat symmetrySeat isSameCol\n";
    for (auto s : allSeats)
        wss << std::setw(5) << s << std::setw(10) << rotateSeat(s) << std::setw(12)
            << symmetrySeat(s) << std::setw(14) << isSameCol(s, s + 8) << L'\n';
    wss << L"getSameColSeats:\n";
    std::vector<pair<int, int>> vp{ { 84, 21 }, { 86, 23 }, { 66, 13 } };
    for (auto ss : vp) {
        for (auto s : getSameColSeats(ss.first, ss.second))
            wss << std::setw(3) << s << L' ';
        wss << L'\n';
    }

    wss << L"getKingMoveSeats:\n";
    std::vector<std::vector<int>> testSeats = { bottomKingSeats, topKingSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << std::setw(3) << seat << L"->";
            for (auto toseat : getKingMoveSeats(seat))
                wss << L' ' << std::setw(3) << toseat;
            wss << L'\n';
        }
    wss << L"getAdvisorMoveSeats:\n";
    testSeats = { bottomAdvisorSeats, topAdvisorSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << std::setw(3) << seat << L"->";
            for (auto toseat : getAdvisorMoveSeats(seat))
                wss << L' ' << std::setw(3) << toseat;
            wss << L'\n';
        }
    wss << L"getBishopMove_CenSeats:\n";
    testSeats = { bottomBishopSeats, topBishopSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << std::setw(3) << seat << L"->";
            for (auto toseat_cen : getBishopMove_CenSeats(seat))
                wss << L' ' << std::setw(3) << toseat_cen.first << L'_' << std::setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }
    wss << L"getKnightMove_LegSeats:\n";
    testSeats = { allSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << std::setw(3) << seat << L"->";
            for (auto toseat_cen : getKnightMove_LegSeats(seat))
                wss << L' ' << std::setw(3) << toseat_cen.first << L'_' << std::setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }

    // 获取全部行列的seat值
    auto getSeats = []() {
        for (int row = 0; row != 10; ++row)
            for (int col = 0; col != 9; ++col)
                getSeat(row, col);
    };
    // 获取全部seat值的行列
    auto getRowCols = []() {
        for (int seat = 0; seat != 90; ++seat) {
            getRow(seat);
            getCol(seat);
        }
    };
    int count{ 10000 };
    auto t0 = steady_clock::now();
    for (int i = 0; i != count; ++i) {
        getSeats();
        getRowCols();
    }
    auto d = steady_clock::now() - t0;
    wss << "getSeats: use time -> " << duration_cast<milliseconds>(d).count()
        << "ms" << L'\n';

    return wss.str();
}
*/
}