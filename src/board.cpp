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
    : bottomColor_{ PieceColor::RED }
    , pieces_{ creatPieces() }
    , seats_{ creatSeats() }
{
}

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int row, const int col) const { return seats_.at(RowcolManager::getIndex(row, col)); }

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int rowcol) const { return seats_.at(RowcolManager::getIndex(rowcol)); }

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeatFromIccs(const std::wstring& ICCS) const
{
    std::string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(PieceCharManager::getRowFromICCSChar(iccs.at(1)), PieceCharManager::getColFromICCSChar(iccs.at(0))),
        getSeat(PieceCharManager::getRowFromICCSChar(iccs.at(3)), PieceCharManager::getColFromICCSChar(iccs.at(2))));
}

const std::wstring Board::getIccs(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat) const
{
    std::wstringstream wss{};
    wss << PieceCharManager::getColICCSChar(fseat->col()) << fseat->row() << PieceCharManager::getColICCSChar(tseat->col()) << tseat->row();
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
    PieceColor color{ PieceCharManager::getColorFromZh(zhStr.back()) };
    bool isBottom{ __isBottomSide(color) };
    int index{}, movDir{ PieceCharManager::getMovNum(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (PieceCharManager::isPiece(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name, PieceCharManager::getCol(isBottom, PieceCharManager::getNum(color, zhStr.at(1))));
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = PieceCharManager::isPawn(name) ? __getSortPawnLiveSeats(color, name) : __getLiveSeats(color, name);
        index = PieceCharManager::getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= static_cast<int>(seats.size()) - 1);
    fseat = seats.at(index);
    int num{ PieceCharManager::getNum(color, zhStr.back()) }, toCol{ PieceCharManager::getCol(isBottom, num) };
    if (PieceCharManager::isLineMove(name)) {
        int trow{ fseat->row() + movDir * num };
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(trow, fseat->col());
    } else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }, //  相距1或2列
            trow{ fseat->row() + movDir * (PieceCharManager::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
        tseat = getSeat(trow, toCol);
    }
    //assert(zhStr == getZh(fseat, tseat));

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

    if (seats.size() > 1 && PieceCharManager::isStronge(name)) {
        if (PieceCharManager::isPawn(name))
            seats = __getSortPawnLiveSeats(color, name);
        wss << PieceCharManager::getIndexChar(seats.size(), isBottom, distance(seats.begin(), find(seats.begin(), seats.end(), fseat))) << name;
    } else //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << PieceCharManager::getColChar(color, isBottom, fromCol);
    wss << PieceCharManager::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (PieceCharManager::isLineMove(name) && !isSameRow ? PieceCharManager::getNumChar(color, abs(fromRow - toRow))
                                                             : PieceCharManager::getColChar(color, isBottom, toCol));

    auto& mvSeats = getMoveSeatFromZh(wss.str());
    assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return wss.str();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::putSeats(const std::shared_ptr<PieceSpace::Piece>& piece) const
{
    bool isBottom{ __isBottomSide(piece->color()) };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    std::vector<std::pair<int, int>> rowcols{};
    switch (piece->kind()) {
    case PieceKind::KING:
        rowcols = RowcolManager::getKingRowcols(isBottom);
        break;
    case PieceKind::ADVSIOR:
        rowcols = RowcolManager::getAdvisorRowcols(isBottom);
        break;
    case PieceKind::BISHOP:
        rowcols = RowcolManager::getBishopRowcols(isBottom);
        break;
    case PieceKind::PAWN:
        rowcols = RowcolManager::getPawnRowcols(isBottom);
        break;
    default: // PieceKind::KNIGHT, PieceKind::ROOK, PieceKind::CANNON
        return seats_;
    }
    for (auto& rowcol : rowcols)
        seats.push_back(getSeat(rowcol.first, rowcol.second));
    return seats;
}

// '获取棋子可走的位置, 不能被将军'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    assert(fseat->piece());
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    auto color = fseat->piece()->color();
    auto pieMoveSeats = __getMoveSeats(fseat);
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
        if (PieceCharManager::isStronge(fseat->piece()->name())) {
            auto& mvSeats = __getMoveSeats(fseat);
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

void Board::reset(const std::wstring& pieceChars)
{
    assert(seats_.size() == pieceChars.size());
    wchar_t ch{};
    int chIndex{ -1 };
    std::vector<bool> used(pieces_.size(), false);
    std::for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        if ((ch = pieceChars[++chIndex]) != PieceCharManager::nullChar()) {
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
    auto changeRowcol = ct == ChangeType::ROTATE ? &RowcolManager::getRotate : &RowcolManager::getSymmetry;
    std::for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        seatPieces.push_back(ct == ChangeType::EXCHANGE ? (seat->piece() ? getOthPiece(seat->piece()) : nullptr)
                                                        : getSeat(changeRowcol(seat->rowcol()))->piece());
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
    bottomColor_ = RowcolManager::isBottom((*pos)->row()) ? PieceColor::RED : PieceColor::BLACK;
}

const std::shared_ptr<SeatSpace::Seat>& Board::__getKingSeat(const PieceColor color) const
{
    auto rowcols = RowcolManager::getKingRowcols(__isBottomSide(color));
    for (auto& rowcol : rowcols) {
        auto& seat = getSeat(rowcol.first, rowcol.second);
        if (seat->piece() && seat->piece()->kind() == PieceKind::KING)
            return seat;
    }
    assert("King is not in board!");
    return seats_[4];//不会执行到，为满足语法要求设置
}

// '多兵排序'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getSortPawnLiveSeats(const PieceColor color, const wchar_t name) const
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

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    switch (auto kind = fseat->piece()->kind()) {
    case PieceKind::BISHOP:
        return __getBishopMoveSeats(fseat);
    case PieceKind::KNIGHT:
        return __getKnightMoveSeats(fseat);
    case PieceKind::ROOK:
        return __getRookMoveSeats(fseat);
    case PieceKind::CANNON:
        return __getCannonMoveSeats(fseat);
    default: // PieceKind::KING PieceKind::ADVSIOR PieceKind::PAWN
        return __getKAPMoveSeats(kind, fseat);
    }
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getKAPMoveSeats(PieceKind kind, const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    auto rowcols_fun = kind == PieceKind::KING ? &RowcolManager::getKingMoveRowcols
                                               : (kind == PieceKind::ADVSIOR ? &RowcolManager::getAdvisorMoveRowcols
                                                                             : &RowcolManager::getPawnMoveRowcols);
    for (auto& rowcol : rowcols_fun(__isBottomSide(fseat->piece()->color()), fseat->row(), fseat->col())) {
        auto& seat = getSeat(rowcol.first, rowcol.second);
        if (seat->isDiffColor(fseat))
            seats.push_back(seat);
    }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    for (auto& rowcol : RowcolManager::getBishopMoveRowcols(__isBottomSide(fseat->piece()->color()), frow, fcol)) {
        auto& tseat = getSeat(rowcol.first, rowcol.second);
        if (!getSeat((frow + rowcol.first) >> 1, (fcol + rowcol.second) >> 1)->piece()
            && tseat->isDiffColor(fseat))
            seats.push_back(tseat);
    }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    int frow{ fseat->row() }, fcol{ fseat->col() };
    for (auto& rowcol : RowcolManager::getKnightMoveRowcols(frow, fcol)) {
        auto& tseat = getSeat(rowcol.first, rowcol.second);
        bool isHorizontal{ abs(frow - rowcol.first) == 1 }; // 横向马腿
        int legrow{ isHorizontal ? frow : frow + (frow > rowcol.first ? -1 : 1) },
            legcol{ isHorizontal ? fcol + (fcol > rowcol.second ? -1 : 1) : fcol };
        if (!getSeat(legrow, legcol)->piece() && tseat->isDiffColor(fseat))
            seats.push_back(tseat);
    }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getRookMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& rowcols : RowcolManager::getRookCannonMoveRowcol_Lines(fseat->row(), fseat->col()))
        for (auto& rowcol : rowcols) {
            auto& tseat = getSeat(rowcol.first, rowcol.second);
            if (tseat->isDiffColor(fseat))
                seats.push_back(tseat);
            if (tseat->piece())
                break;
        }
    return seats;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getCannonMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const
{

    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& rowcols : RowcolManager::getRookCannonMoveRowcol_Lines(fseat->row(), fseat->col())) {
        bool skip = false;
        for (auto& rowcol : rowcols) {
            auto& tseat = getSeat(rowcol.first, rowcol.second);
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
    }
    return seats;
}

const std::wstring Board::getPieceChars() const
{
    std::wstringstream wss{};
    std::shared_ptr<PieceSpace::Piece> pie{};
    for_each(seats_.begin(), seats_.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << ((pie = seat->piece()) ? pie->ch() : PieceCharManager::nullChar()); });
    return wss.str();
}

const std::wstring Board::toString() const
{ // 文本空棋盘
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

    for (auto& seat : __getLiveSeats())
        textBlankBoard[(RowcolManager::ColNum() - seat->row()) * 2 * (RowcolManager::ColNum() * 2) + seat->col() * 2] = PieceCharManager::getPrintName((*seat->piece()));
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
        wss << L"getAllSeats() " << getSeatsStr(putSeats(pieces_[5])) << L'\n';
        wss << L"__getLiveSeats() " << getSeatsStr(__getLiveSeats()) << L"\n";

        wss << L"PieceColor: " << static_cast<int>(PieceColor::RED) << L'\n';
        wss << L"__getKingSeat() " << __getKingSeat(PieceColor::RED)->toString() << L'\n';
        wss << L"getKingSeats() " << getSeatsStr(putSeats(pieces_[0])) << L'\n';
        wss << L"getAdvisorSeats() " << getSeatsStr(putSeats(pieces_[1])) << L'\n';
        wss << L"getBishopSeats() " << getSeatsStr(putSeats(pieces_[3])) << L'\n';
        wss << L"getPawnSeats() " << getSeatsStr(putSeats(pieces_[14])) << L'\n';
        wss << L"__getLiveSeats() " << getSeatsStr(__getLiveSeats(PieceColor::RED)) << L"\n";

        wss << L"PieceColor: " << static_cast<int>(PieceColor::BLACK) << L'\n';
        wss << L"__getKingSeat() " << __getKingSeat(PieceColor::BLACK)->toString() << L'\n';
        wss << L"getKingSeats() " << getSeatsStr(putSeats(pieces_[16])) << L'\n';
        wss << L"getAdvisorSeats() " << getSeatsStr(putSeats(pieces_[17])) << L'\n';
        wss << L"getBishopSeats() " << getSeatsStr(putSeats(pieces_[19])) << L'\n';
        wss << L"getPawnSeats() " << getSeatsStr(putSeats(pieces_[31])) << L'\n';
        wss << L"__getLiveSeats() " << getSeatsStr(__getLiveSeats(PieceColor::BLACK)) << L"\n";
        //*/
        //*
        auto getLiveSeatsStr = [&](void) {
            wss << L"\nbottomColor: " << static_cast<int>(bottomColor_) << L'\n' << toString();
            for (auto color : { PieceColor::RED, PieceColor::BLACK })
                for (auto fseat : __getLiveSeats(color))
                    wss << fseat->toString() << L"=>" << getSeatsStr(moveSeats(fseat)) << L'\n';
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

const std::vector<std::pair<int, int>> RowcolManager::getAllRowcols()
{
    std::vector<std::pair<int, int>> rowcols{};
    for (int row = 0; row < RowNum_; ++row)
        for (int col = 0; col < ColNum_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getKingRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getAdvisorRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            if ((col + row) % 2 == rmd)
                rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getBishopRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2) {
            int gap{ row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                rowcols.push_back({ row, col });
        }
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getPawnRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int lfrow{ isBottom ? RowLowUpIndex_ - 1 : RowUpLowIndex_ },
        ufrow{ isBottom ? RowLowUpIndex_ : RowUpLowIndex_ + 1 },
        ltrow{ isBottom ? RowUpLowIndex_ : RowLowIndex_ },
        utrow{ isBottom ? RowUpIndex_ : RowLowUpIndex_ };
    for (int row = lfrow; row <= ufrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2)
            rowcols.push_back({ row, col });
    for (int row = ltrow; row <= utrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getKingMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol } },
        moveRowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getAdvisorMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 } },
        moveRowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getBishopMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow - 2, fcol - 2 }, { frow - 2, fcol + 2 },
        { frow + 2, fcol - 2 }, { frow + 2, fcol + 2 } },
        moveRowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColLowIndex_ && rowcol.second <= ColUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getKnightMoveRowcols(int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow - 2, fcol - 1 }, { frow - 2, fcol + 1 },
        { frow - 1, fcol - 2 }, { frow - 1, fcol + 2 },
        { frow + 1, fcol - 2 }, { frow + 1, fcol + 2 },
        { frow + 2, fcol - 1 }, { frow + 2, fcol + 1 } },
        moveRowcols{};
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= RowLowIndex_ && rowcol.first <= RowUpIndex_
                && rowcol.second >= ColLowIndex_ && rowcol.second <= ColUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::vector<std::pair<int, int>>> RowcolManager::getRookCannonMoveRowcol_Lines(int frow, int fcol)
{
    std::vector<std::vector<std::pair<int, int>>> rowcol_Lines(4);
    for (int col = fcol - 1; col >= ColLowIndex_; --col)
        rowcol_Lines[0].push_back({ frow, col });
    for (int col = fcol + 1; col <= ColUpIndex_; ++col)
        rowcol_Lines[1].push_back({ frow, col });
    for (int row = frow - 1; row >= RowLowIndex_; --row)
        rowcol_Lines[2].push_back({ row, fcol });
    for (int row = frow + 1; row <= RowUpIndex_; ++row)
        rowcol_Lines[3].push_back({ row, fcol });
    return rowcol_Lines;
}

const std::vector<std::pair<int, int>> RowcolManager::getPawnMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> moveRowcols{};
    int row{}, col{};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        moveRowcols.push_back({ row, fcol });
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            moveRowcols.push_back({ frow, col });
        if ((col = fcol + 1) <= ColUpIndex_)
            moveRowcols.push_back({ frow, col });
    }
    return moveRowcols;
}

const wchar_t PieceCharManager::getName(const wchar_t ch)
{
    std::map<int, int> chIndex_nameIndex{
        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    };
    return nameChars_.at(chIndex_nameIndex.at(chStr_.find(ch)));
}

const wchar_t PieceCharManager::getPrintName(const PieceSpace::Piece& piece)
{
    std::map<wchar_t, wchar_t> rcpName{ { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    wchar_t name = piece.name();
    return (piece.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end()) ? rcpName[name] : name;
}

const PieceColor PieceCharManager::getColor(const wchar_t ch) { return islower(ch) ? PieceColor::BLACK : PieceColor::RED; }

const PieceKind PieceCharManager::getKind(const wchar_t ch)
{
    switch (std::tolower(ch)) {
    case L'k':
        return PieceKind::KING;
    case L'a':
        return PieceKind::ADVSIOR;
    case L'b':
        return PieceKind::BISHOP;
    case L'n':
        return PieceKind::KNIGHT;
    case L'r':
        return PieceKind::ROOK;
    case L'c':
        return PieceKind::CANNON;
    default: // L'p':
        return PieceKind::PAWN;
    }
}

const std::vector<wchar_t> PieceCharManager::getAllChs()
{
    std::vector<wchar_t> chs{};
    for (auto i : { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
             7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13 })
        chs.push_back(chStr_.at(i));
    return chs; //L"KAABBNNRRCCPPPPPkaabbnnrrccppppp"
};

const PieceColor PieceCharManager::getColorFromZh(const wchar_t numZh)
{
    return numChars_.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const int PieceCharManager::getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index{ static_cast<int>(__getPreChars(seatsLen).find(preChar)) };
    return isBottom ? seatsLen - 1 - index : index;
}

const wchar_t PieceCharManager::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return __getPreChars(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

const std::wstring PieceCharManager::nameChars_{ L"帅将仕士相象马车炮兵卒" };
const std::wstring PieceCharManager::movChars_{ L"退平进" };
const std::map<PieceColor, std::wstring> PieceCharManager::numChars_{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
const std::wstring PieceCharManager::chStr_{ L"KABNRCPkabnrcp" };

const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces()
{
    std::vector<std::shared_ptr<PieceSpace::Piece>> pieces{};
    for (auto ch : PieceCharManager::getAllChs())
        pieces.push_back(std::make_shared<PieceSpace::Piece>(ch));
    return pieces;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats()
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& rowcol : RowcolManager::getAllRowcols())
        seats.push_back(std::make_shared<SeatSpace::Seat>(rowcol.first, rowcol.second));
    return seats;
}

const std::wstring getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats)
{
    std::wstringstream wss{};
    wss << seats.size() << L"个: ";
    std::for_each(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << seat->toString() << L' '; });
    return wss.str();
}
}