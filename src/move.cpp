#include "move.h"
#include "board_base.h"
#include "piece.h"
//#include "pieces.h"
#include <sstream>
using namespace std;
using namespace Board_base;

Move::Move()
    : fromseat{ nullSeat }
    , toseat{ nullSeat }
    , eatPie_ptr{ nullptr } //Pieces::nullPiePtr
{
}

void Move::setNext(shared_ptr<Move> next)
{
    next_ptr = next;
    if (next) {
        next->stepNo = stepNo + 1; // 步数
        next->othCol = othCol; // 变着层数
        next->setPrev(make_shared<Move>(*this)); // 是否构成环形指针，造成不能自动析构？
    }
}

void Move::setOther(shared_ptr<Move> other)
{
    other_ptr = other;
    if (other) {
        other->stepNo = stepNo; // 与premove的步数相同
        other->othCol = othCol + 1; // 变着层数
        other->setPrev(make_shared<Move>(*this)); // 是否构成环形指针，造成不能自动析构？
    }
}

const wstring Move::toString() const
{
    wstringstream wss{};
    wss << L"<rcm:" << stepNo << L' ' << othCol << L' '
        << maxCol << L" f_t:" << fromseat << L' ' << toseat << L" e:" << eatPie_ptr->chName() << L" I:" << ICCS << L" z:" << zh
        << L",r:" << remark << L">";
    return wss.str();
}