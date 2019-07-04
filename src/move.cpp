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

Move::Move(std::shared_ptr<SeatSpace::Seat>& fseat, std::shared_ptr<SeatSpace::Seat>& tseat)
{
    ftseat_.first = fseat;
    ftseat_.second = tseat;
}

Move::Move(const std::shared_ptr<BoardSpace::Board>& board, int frowcol, int trowcol)
{
    ftseat_ = std::make_pair(board->getSeat(frowcol), board->getSeat(trowcol));
}

Move::Move(const std::shared_ptr<BoardSpace::Board>& board, const std::wstring& str, RecFormat fmt)
{
    if (fmt == RecFormat::PGN_ICCS)
        ftseat_ = std::make_pair(board->getSeat(PieceManager::getRowFromICCSChar(str.at(1)),
                                     PieceManager::getColFromICCSChar(str.at(0))),
            board->getSeat(PieceManager::getRowFromICCSChar(str.at(3)),
                PieceManager::getColFromICCSChar(str.at(2))));
    else //(fmt == RecFormat::PGN_ZH)
        ftseat_ = board->getMoveSeatFromZh(str);
}

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
    auto next = std::make_shared<Move>();
    next->setNextNo(nextNo_ + 1); // 步序号
    next->setOtherNo(otherNo_); // 变着层数
    next->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return next_ = next;
}

const std::shared_ptr<Move>& Move::addOther()
{
    auto other = std::make_shared<Move>();
    other->setNextNo(nextNo_); // 与premove的步数相同
    other->setOtherNo(otherNo_ + 1); // 变着层数
    other->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return other_ = other;
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