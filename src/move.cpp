#include "move.h"
//#include "piece.h"
#include "seat.h"
//#include <algorithm>
//#include <vector>
using namespace std;

void Move::setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
}

void Move::setNext(shared_ptr<Move>& next)
{
    next->setStepNo(stepNo_ + 1); // 步序号
    next->setOthCol(othCol_); // 变着层数
    auto prev = make_shared<Move>(*this);
    next->setPrev(prev);
    next_ = next;
}

void Move::setOther(shared_ptr<Move>& other)
{
    other->setStepNo(stepNo_); // 与premove的步数相同
    other->setOthCol(othCol_ + 1); // 变着层数
    auto prev = make_shared<Move>(*this);
    other->setPrev(prev);
    other_ = other;
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