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
    , pieces_{ std::make_shared<PieceSpace::Pieces>() }
    , seats_{ std::make_shared<SeatSpace::Seats>() }
{
}

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int row, const int col) const
{
    return seats_->getSeat(row, col);
}

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int rowcol) const
{
    return seats_->getSeat(rowcol);
}

const std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const std::pair<int, int>& rowcol) const
{
    return seats_->getSeat(rowcol);
}

//中文纵线着法->(fseat, tseat)
const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeatFromZh(const std::wstring& zhStr) const
{
    assert(zhStr.size() == 4);
    std::shared_ptr<SeatSpace::Seat> fseat{}, tseat{};
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ PieceManager::getColorFromZh(zhStr.back()) };
    bool isBottom{ isBottomSide(color) };
    int index{}, movDir{ PieceManager::getMovNum(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (PieceManager::isPiece(name)) { // 首字符为棋子名
        seats = seats_->getLiveSeats(color, name, PieceManager::getCol(isBottom, PieceManager::getNum(color, zhStr.at(1))));
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = zhStr.at(1);
        seats = (PieceManager::isPawn(name)
                ? seats_->getSortPawnLiveSeats(isBottom, color, name)
                : seats_->getLiveSeats(color, name));
        index = PieceManager::getIndex(seats.size(), isBottom, zhStr.front());
    }

    assert(index <= static_cast<int>(seats.size()) - 1);
    fseat = seats.at(index);
    int num{ PieceManager::getNum(color, zhStr.back()) }, toCol{ PieceManager::getCol(isBottom, num) };
    if (PieceManager::isLineMove(name)) {
        int trow{ fseat->row() + movDir * num };
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(trow, fseat->col());
    } else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }, //  相距1或2列
            trow{ fseat->row() + movDir * (PieceManager::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)) };
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
    auto seats = seats_->getLiveSeats(color, name, fromCol);

    if (seats.size() > 1 && PieceManager::isStronge(name)) {
        if (PieceManager::isPawn(name))
            seats = seats_->getSortPawnLiveSeats(isBottom, color, name);
        wss << PieceManager::getIndexChar(seats.size(), isBottom, distance(seats.begin(), find(seats.begin(), seats.end(), fseat))) << name;
    } else //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << PieceManager::getColChar(color, isBottom, fromCol);
    wss << PieceManager::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (PieceManager::isLineMove(name) && !isSameRow ? PieceManager::getNumChar(color, abs(fromRow - toRow))
                                                         : PieceManager::getColChar(color, isBottom, toCol));

    auto& mvSeats = getMoveSeatFromZh(wss.str());
    assert(fseat == mvSeats.first && tseat == mvSeats.second);

    return wss.str();
}

//判断是否将军
const bool Board::isKilled(const PieceColor color) const
{
    PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED : PieceColor::BLACK;
    std::shared_ptr<SeatSpace::Seat> kingSeat{ seats_->getKingSeat(color) }, othKingSeat{ seats_->getKingSeat(othColor) };
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
    for (auto& seat : seats_->getLiveSeats(othColor, L'\x00', -1, true)) {
        //for (auto& seat : __getLiveStrongeSeats(othColor)) {
        auto& mvSeats = seat->piece()->moveSeats(*this, *seat);
        if (!mvSeats.empty() && find(mvSeats.begin(), mvSeats.end(), kingSeat) != mvSeats.end())
            return true;
    }
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color) const
{
    for (auto& fseat : seats_->getLiveSeats(color))
        if (!fseat->getMoveSeats(*this).empty())
            return false;
    return true;
}

void Board::reset()
{
    seats_->reset();
    bottomColor_ = PieceColor::RED;
}

void Board::reset(const std::wstring& pieceChars)
{
    seats_->reset(pieces_->getBoardPieces(pieceChars));
    __setBottomSide();
}

void Board::changeSide(const ChangeType ct)
{
    std::vector<std::shared_ptr<PieceSpace::Piece>> boardPieces{};
    auto changeRowcol = (ct == ChangeType::ROTATE
            ? &SeatManager::getRotate
            : &SeatManager::getSymmetry);
    auto allSeats = seats_->getAllSeats();
    std::for_each(allSeats.begin(), allSeats.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            boardPieces.push_back(ct == ChangeType::EXCHANGE
                    ? pieces_->getOtherPiece(seat->piece())
                    : getSeat(changeRowcol(seat->rowcol()))->piece());
        });
    seats_->reset(boardPieces);
    __setBottomSide();
}

void Board::__setBottomSide()
{
    bottomColor_ = (SeatManager::isBottom(seats_->getKingSeat(PieceColor::RED)->row())
            ? PieceColor::RED
            : PieceColor::BLACK);
}

const std::wstring Board::getPieceChars() const
{
    return seats_->getPieceChars();
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

    for (auto color : { PieceColor::BLACK, PieceColor::RED })
        for (auto& seat : seats_->getLiveSeats(color))
            textBlankBoard[(SeatManager::ColNum() - seat->row())
                * 2 * (SeatManager::ColNum() * 2)
                * +seat->col() * 2]
                = PieceManager::getPrintName(*seat->piece());
    return textBlankBoard;
}

const std::wstring Board::test()
{
    std::wstringstream wss{};

    // Piece test
    wss << L"全部棋子: " << pieces_->toString() << L"\n";

    // Board test
    for (const auto& fen : { L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
             L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" }) {
        auto pieceChars = InstanceSpace::FENTopieChars(fen);

        reset(pieceChars);
        wss << "fen:" << fen
            << "\nget:" << InstanceSpace::pieCharsToFEN(pieceChars)
            << "\ngetChars:" << pieceChars
            << "\nboardGet:" << getPieceChars() << L'\n';
        //*
        for (auto color : { PieceColor::RED, PieceColor::BLACK })
            wss << L"PieceColor: " << static_cast<int>(color) << L'\t'
                << seats_->getKingSeat(color)->toString() << L'\n';
        wss << L"seats_->getLiveSeats() "
            << SeatManager::getSeatsStr(seats_->getLiveSeats(PieceColor::RED))
            << SeatManager::getSeatsStr(seats_->getLiveSeats(PieceColor::BLACK)) << L"\n";
        auto getLiveSeatsStr = [&](void) {
            wss << L"\nbottomColor: " << static_cast<int>(bottomColor_) << L'\n' << toString();
            for (auto color : { PieceColor::RED, PieceColor::BLACK })
                for (auto& fseat : seats_->getLiveSeats(color))
                    wss << fseat->toString() << L"=>"
                        << SeatManager::getSeatsStr(fseat->getMoveSeats(*this)) << L'\n';
        };
        getLiveSeatsStr();
        //*/
        //*
        for (const auto chg : { ChangeType::EXCHANGE,
                 ChangeType::ROTATE, ChangeType::SYMMETRY }) { //
            changeSide(chg);
            getLiveSeatsStr();
        }
        //*/
        wss << L"\n";
    }
    return wss.str();
}
}