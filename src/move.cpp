#include "move.h"
#include "piece.h"
#include "seat.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace MoveSpace {

Move::Move()
    : eatPie_{ PieceSpace::nullPiece }
{
}

const std::shared_ptr<Move>& Move::setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat, const std::shared_ptr<SeatSpace::Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
    return move(shared_from_this());
}

const std::shared_ptr<Move>& Move::setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>& seats)
{
    return setSeats(seats.first, seats.second);
}

const std::shared_ptr<Move>& Move::addNext(const std::shared_ptr<Move>& next)
{
    if (next) {
        next->setStepNo(stepNo_ + 1); // 步序号
        next->setOthCol(othCol_); // 变着层数
        next->setPrev(shared_from_this());
    }
    return next_ = next;
}

const std::shared_ptr<Move>& Move::addOther(const std::shared_ptr<Move>& other)
{
    if (other) {
        other->setStepNo(stepNo_); // 与premove的步数相同
        other->setOthCol(othCol_ + 1); // 变着层数
        other->setPrev(shared_from_this());
    }
    return other_ = other;
}

std::vector<std::shared_ptr<Move>> Move::getPrevMoves()
{
    std::vector<std::shared_ptr<Move>> moves{};
    std::shared_ptr<Move> this_move{ shared_from_this() }, prev_move{};
    while ((prev_move = this_move->prev()) && prev_move->prev()) { // 排除rootMove
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const std::shared_ptr<Move>& Move::done()
{
    eatPie_ = SeatSpace::move(fseat_, tseat_, PieceSpace::nullPiece);
    //eatPie_ = tseat_->piece();
    //tseat_->put(fseat_->piece());
    //fseat_->put(PieceSpace::nullPiece);
    return next();
}

const std::shared_ptr<Move>& Move::undo()
{
    SeatSpace::move(tseat_, fseat_, eatPie_);
    //fseat_->put(tseat_->piece());
    //tseat_->put(eatPie_);
    return prev();
}

const std::wstring Move::toString() const
{
    std::wstringstream wss{};
    wss << fseat_->toString() << L'>' << tseat_->toString() << L'-' << (bool(eatPie_) ? eatPie_->name() : L'空')
        << remark_ << L' ' << iccs_ << L' ' << zh_ << L' ' << stepNo_ << L' ' << othCol_ << L' ' << CC_Col_;
    return wss.str();
}
}