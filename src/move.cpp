#include "move.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
//#include <vector>
using namespace std;

Move::Move()
    : eatPie_{ Board::nullPiece }
{
}

const shared_ptr<Move>& Move::setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
    return move(shared_from_this());
}

const shared_ptr<Move>& Move::setSeats(const pair<const shared_ptr<Seat>, const shared_ptr<Seat>>& seats)
{
    return setSeats(seats.first, seats.second);
}

const shared_ptr<Move>& Move::setNext(const shared_ptr<Move>& next)
{
    if (next) {
        next->setStepNo(stepNo_ + 1); // 步序号
        next->setOthCol(othCol_); // 变着层数
        next->setPrev(shared_from_this());
    }
    return next_ = next;
}

const shared_ptr<Move>& Move::setOther(const shared_ptr<Move>& other)
{
    if (other) {
        other->setStepNo(stepNo_); // 与premove的步数相同
        other->setOthCol(othCol_ + 1); // 变着层数
        other->setPrev(shared_from_this());
    }
    return other_ = other;
}

vector<shared_ptr<Move>> Move::getPrevMoves()
{
    vector<shared_ptr<Move>> moves{};
    shared_ptr<Move> this_move{ shared_from_this() }, prev_move{};
    while ((prev_move = this_move->prev()) && prev_move->prev()) { // 排除rootMove
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const shared_ptr<Move>& Move::done()
{
    eatPie_ = tseat_->piece();
    tseat_->put(fseat_->piece());
    fseat_->put(Board::nullPiece);
    return next();
}

const shared_ptr<Move>& Move::undo()
{
    fseat_->put(tseat_->piece());
    tseat_->put(eatPie_);
    return prev();
}

const wstring Move::toString() const
{
    wstringstream wss{};
    wss << fseat_->toString() << L'>' << tseat_->toString() << L'-' << (bool(eatPie_) ? eatPie_->name() : L'空')
        << remark_ << L' ' << iccs_ << L' ' << zh_ << L' ' << stepNo_ << L' ' << othCol_ << L' ' << CC_Col_;
    return wss.str();
}
