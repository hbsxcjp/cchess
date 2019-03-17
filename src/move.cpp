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

void Move::setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
}

void Move::setSeats(const pair<const shared_ptr<Seat>, const shared_ptr<Seat>>& seats)
{
    setSeats(seats.first, seats.second);
}

void Move::setNext(const shared_ptr<Move>& next)
{
    next_ = next;
    if (next_) {
        next_->setStepNo(stepNo_ + 1); // 步序号
        next_->setOthCol(othCol_); // 变着层数
        next_->setPrev(make_shared<Move>(*this));
    }
}

void Move::setOther(const shared_ptr<Move>& other)
{
    other_ = other;
    if (other_) {
        other_->setStepNo(stepNo_); // 与premove的步数相同
        other_->setOthCol(othCol_ + 1); // 变着层数
        other_->setPrev(make_shared<Move>(*this));
    }
}

vector<shared_ptr<Move>> Move::getPrevMoves()
{
    vector<shared_ptr<Move>> moves{};
    shared_ptr<Move> next_move{ make_shared<Move>(*this) }, prev_move{};
    while (prev_move = next_move->prev()) { // 排除rootMove
        moves.push_back(next_move);
        next_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

void Move::done()
{
    eatPie_ = tseat_->piece();
    tseat_->put(fseat_->piece());
    fseat_->put(Board::nullPiece);
}

void Move::undo()
{
    fseat_->put(tseat_->piece());
    tseat_->put(eatPie_);
}

const wstring Move::toString() const
{
    wstringstream wss{};
    wss << fseat_->toString() << L'>' << tseat_->toString() << L'-' << (bool(eatPie_) ? eatPie_->name() : L'空')
        << remark_ << L' ' << iccs_ << L' ' << zh_ << L' ' << stepNo_ << L' ' << othCol_ << L' ' << maxCol_;
    return wss.str();
}
