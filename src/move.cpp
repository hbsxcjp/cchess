#include "move.h"
#include "piece.h"
#include "seat.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
//#include <vector>
using namespace std;

void Move::setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
}

void Move::setNext(shared_ptr<Move> next)
{
    if (next) {
        next->setStepNo(stepNo_ + 1); // 步序号
        next->setOthCol(othCol_); // 变着层数
        next->setPrev(make_shared<Move>(*this));
    }
    next_ = next;
}

void Move::setOther(shared_ptr<Move> other)
{
    if (other) {
        other->setStepNo(stepNo_); // 与premove的步数相同
        other->setOthCol(othCol_ + 1); // 变着层数
        other->setPrev(make_shared<Move>(*this));
    }
    other_ = other;
}

vector<shared_ptr<Move>> Move::getPrevMoves()
{
    vector<shared_ptr<Move>> moves{};
    shared_ptr<Move> next_move{ make_shared<Move>(*this) }, prev_move{};
    while (prev_move = next_move->prev()) {
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
    fseat_->pop();
}

void Move::undo()
{
    fseat_->put(tseat_->piece());
    tseat_->put(eatPie_);
}

const wstring Move::toString() const
{
    wstringstream wss{};
    wss << fseat_->toString() << L' ' << tseat_->toString() << L' ' << eatPie_->name() << L' '
        << remark_ << L' ' << iccs_ << L' ' << zh_ << L' ' << stepNo_ << L' ' << othCol_ << L' ' << maxCol_ << L'\n';
    return wss.str();
}
