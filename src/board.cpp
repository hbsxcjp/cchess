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
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace SeatSpace;
using namespace PieceSpace;
namespace BoardSpace {

Board::Board()
    : bottomColor_{ PieceColor::RED }
    , pieces_{ creatPieces() }
    , seats_{ creatSeats() }
{
}

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int row, const int col) const { return seats_.at(RowcolManager::getIndex(row, col)); }

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int rowcol) const { return seats_.at(RowcolManager::getIndex(rowcol)); }

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getSeats(std::vector<std::pair<int, int>> rowcols) const
{
    if (!rowcols.empty()) {
        std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
        for_each(rowcols.begin(), rowcols.end(), [&](std::pair<int, int>& rowcol) {
            seats.push_back(getSeat(rowcol.first, rowcol.second));
        });
        return seats;
    } else
        return seats_;
}

//中文纵线着法->(fseat, tseat)
const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeatFromZh(const std::wstring& zhStr) const
{
    assert(zhStr.size() == 4);
    std::shared_ptr<SeatSpace::Seat> fseat{}, tseat{};
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ CharManager::getColorFromZh(zhStr.back()) };
    bool isBottom{ isBottomSide(color) };
    int index{}, movDir{ CharManager::getMovNum(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (CharManager::isPiece(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name, CharManager::getCol(isBottom, CharManager::getNum(color, zhStr.at(1))));
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = CharManager::isPawn(name) ? __getSortPawnLiveSeats(color, name) : __getLiveSeats(color, name);
        index = CharManager::getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= static_cast<int>(seats.size()) - 1);
    fseat = seats.at(index);
    int num{ CharManager::getNum(color, zhStr.back()) }, toCol{ CharManager::getCol(isBottom, num) };
    if (CharManager::isLineMove(name)) {
        int trow{ fseat->row() + movDir * num };
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(trow, fseat->col());
    } else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }, //  相距1或2列
            trow{ fseat->row() + movDir * (CharManager::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
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
    bool isSameRow{ fromRow == toRow }, isBottom{ isBottomSide(color) };
    auto seats = __getLiveSeats(color, name, fromCol);

    if (seats.size() > 1 && CharManager::isStronge(name)) {
        if (CharManager::isPawn(name))
            seats = __getSortPawnLiveSeats(color, name);
        wss << CharManager::getIndexChar(seats.size(), isBottom, distance(seats.begin(), find(seats.begin(), seats.end(), fseat))) << name;
    } else //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << CharManager::getColChar(color, isBottom, fromCol);
    wss << CharManager::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (CharManager::isLineMove(name) && !isSameRow ? CharManager::getNumChar(color, abs(fromRow - toRow))
                                                        : CharManager::getColChar(color, isBottom, toCol));

    auto& mvSeats = getMoveSeatFromZh(wss.str());
    assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return wss.str();
}

//判断是否将军
const bool Board::isKilled(const PieceColor color) const
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    std::shared_ptr<SeatSpace::Seat> kingSeat{ __getKingSeat(color) }, othKingSeat{ __getKingSeat(othColor) };
    int fcol{ kingSeat->col() };
    if (fcol == othKingSeat->col()) {
        bool isBottom{ isBottomSide(color) };
        int lrow{ isBottom ? kingSeat->row() : othKingSeat->row() }, urow{ isBottom ? othKingSeat->row() : kingSeat->row() };
        bool isBlank{ true };
        for (int row = lrow + 1; row < urow; ++row)
            if (getSeat(row, fcol)->piece()) {
                isBlank = false;
                break;
            }
        if (isBlank)
            return true;
    }
    // '获取某方可杀将棋子全部可走的位置
    for (auto& seat : __getLiveStrongeSeats(othColor)) {
        auto& mvSeats = seat->piece()->moveSeats(*this, *seat);
        if (!mvSeats.empty() && find(mvSeats.begin(), mvSeats.end(), kingSeat) != mvSeats.end())
            return true;
    }
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color) const
{
    for (auto& fseat : __getLiveSeats(color))
        if (!fseat->getMoveSeats(*this).empty())
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
        if ((ch = pieceChars[++chIndex]) != CharManager::nullChar()) {
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
    bottomColor_ = RowcolManager::isBottom(__getKingSeat(PieceColor::RED)->row()) ? PieceColor::RED : PieceColor::BLACK;
}

const std::shared_ptr<SeatSpace::Seat>& Board::__getKingSeat(const PieceColor color) const
{
    auto& liveSeats = __getLiveSeats(color);
    auto pos = std::find_if(liveSeats.begin(), liveSeats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        return CharManager::isKing(seat->piece()->name());
    });
    assert(pos != liveSeats.end());
    return *pos;
}

// '多兵排序'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getSortPawnLiveSeats(const PieceColor color, const wchar_t name) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> pawnSeats{ __getLiveSeats(color, name) }, seats{}; // 最多5个兵
    bool isBottom{ isBottomSide(color) };
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

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__getLiveStrongeSeats(const PieceColor color) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> colorSeats{ __getLiveSeats(color) }, seats{};
    std::copy_if(colorSeats.begin(), colorSeats.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { return CharManager::isStronge(seat->piece()->name()); });
    return seats;
}

const std::wstring Board::getPieceChars() const
{
    std::wstringstream wss{};
    for_each(seats_.begin(), seats_.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << ((seat->piece()) ? seat->piece()->ch() : CharManager::nullChar()); });
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
        textBlankBoard[(RowcolManager::ColNum() - seat->row()) * 2 * (RowcolManager::ColNum() * 2) + seat->col() * 2] = CharManager::getPrintName((*seat->piece()));
    return textBlankBoard;
}

const std::wstring Board::test()
{
    std::wstringstream wss{};
    // Piece test
    wss << L"全部棋子" << pieces_.size() << L"个:\n";
    for (auto& piece : pieces_) {
        wss << piece->toString() << L" putSeats(): " << getSeatsStr(piece->putSeats(*this)) << L'\n';
    }
    wss << L"\n";

    // Board test
    for (const auto& fen : { L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
             L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" }) {
        auto pieceChars = InstanceSpace::FENTopieChars(fen);

        reset(pieceChars);
        wss << "fen:" << fen << "\nget:" << InstanceSpace::pieCharsToFEN(pieceChars)
            << "\ngetChars:" << pieceChars << "\nboardGet:" << getPieceChars() << L'\n';
        //*
        for (auto color : { PieceColor::RED, PieceColor::BLACK })
            wss << L"PieceColor: " << static_cast<int>(color) << L'\t' << __getKingSeat(color)->toString() << L'\n';
        wss << L"__getLiveSeats() " << getSeatsStr(__getLiveSeats()) << L"\n";
        auto getLiveSeatsStr = [&](void) {
            wss << L"\nbottomColor: " << static_cast<int>(bottomColor_) << L'\n' << toString();
            for (auto color : { PieceColor::RED, PieceColor::BLACK })
                for (auto& fseat : __getLiveSeats(color))
                    wss << fseat->toString() << L"=>" << getSeatsStr(fseat->getMoveSeats(*this)) << L'\n';
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