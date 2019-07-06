#include "move.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
//#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace SeatSpace;
using namespace PieceSpace;
namespace MoveSpace {

int Move::frowcol() const { return fseat()->rowcol(); }

int Move::trowcol() const { return tseat()->rowcol(); }

const std::wstring Move::iccs() const
{
    std::wstringstream wss{};
    wss << PieceManager::getColICCSChar(fseat()->col()) << fseat()->row()
        << PieceManager::getColICCSChar(tseat()->col()) << tseat()->row();
    return wss.str();
}

const std::wstring Move::zh(const std::shared_ptr<BoardSpace::Board>& board) const
{
    return board->getZh(*this);
}

const std::shared_ptr<Move>& Move::addNext()
{
    auto nextMove = std::make_shared<Move>();
    nextMove->setNextNo(nextNo_ + 1); // 步序号
    nextMove->setOtherNo(otherNo_); // 变着层数
    nextMove->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return next_ = nextMove;
}

const std::shared_ptr<Move>& Move::addOther()
{
    auto otherMove = std::make_shared<Move>();
    otherMove->setNextNo(nextNo_); // 与premove的步数相同
    otherMove->setOtherNo(otherNo_ + 1); // 变着层数
    otherMove->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return other_ = otherMove;
} 

void Move::reset(std::shared_ptr<SeatSpace::Seat>& fseat,
    std::shared_ptr<SeatSpace::Seat>& tseat, std::wstring remark)
{
    setFTSeat({ fseat, tseat });
    setRemark(remark);
}

void Move::reset(const std::shared_ptr<BoardSpace::Board>& board,
    int frowcol, int trowcol, std::wstring remark)
{
    setFTSeat({ board->getSeat(frowcol), board->getSeat(trowcol) });
    setRemark(remark);
}

void Move::reset(const std::shared_ptr<BoardSpace::Board>& board,
    const std::wstring& str, RecFormat fmt, std::wstring remark)
{
    setFTSeat((fmt == RecFormat::PGN_ICCS)
            ? std::make_pair(board->getSeat(PieceManager::getRowFromICCSChar(str.at(1)),
                                 PieceManager::getColFromICCSChar(str.at(0))),
                  board->getSeat(PieceManager::getRowFromICCSChar(str.at(3)),
                      PieceManager::getColFromICCSChar(str.at(2))))
            //(fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC)
            : board->getMoveSeat(str));
    setRemark(remark);
}

std::vector<std::shared_ptr<Move>> Move::getPrevMoves()
{
    std::shared_ptr<Move> this_move{ shared_from_this() }, prev_move{};
    std::vector<std::shared_ptr<Move>> moves{ this_move };
    while (prev_move = this_move->prev()) {
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const std::shared_ptr<Move>& Move::done()
{
    eatPie_ = ftseat_.first->movTo(*ftseat_.second);
    return next_;
}

const std::shared_ptr<Move>& Move::undo()
{
    ftseat_.second->movTo(*ftseat_.first, eatPie_);
    return std::move(prev());
}

const std::wstring Move::toString(const std::shared_ptr<BoardSpace::Board>& board) const
{
    std::wstringstream wss{};
    wss << std::setw(2) << frowcol() << L'_' << std::setw(2) << trowcol()
        << L'-' << std::setw(4) << iccs() << L':' << std::setw(4)
        << zh(board) << L'{' << remark() << L'}';
    return wss.str();
}
}