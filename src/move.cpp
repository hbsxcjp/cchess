#include "move.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
#include <algorithm>
//#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
//#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace MoveSpace {

const std::shared_ptr<Move>& Move::setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat)
{
    fseat_ = fseat;
    //frowcol_ = fseat->rowcol();
    tseat_ = tseat;
    //trowcol_ = tseat->rowcol();
    return std::move(shared_from_this());
}

const std::shared_ptr<Move>& Move::setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>,
    const std::shared_ptr<SeatSpace::Seat>>& seats)
{
    return setSeats(seats.first, seats.second);
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
    eatPie_ = fseat_->movTo(tseat_);
    return next_;
}

const std::shared_ptr<Move>& Move::undo()
{
    tseat_->movTo(fseat_, eatPie_);
    return std::move(prev());
}

const std::wstring Move::toString() const
{
    std::wstringstream wss{};
    wss << frowcol_ / 10 << frowcol_ % 10 << L'_' << trowcol_ / 10 << trowcol_ % 10
        << L'-' << std::setw(4) << iccs_ << L':' << std::setw(4) << zh_ << L'{' << remark_ << L'}';
    return wss.str();
}
}