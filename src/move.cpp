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

int Move::frowcol() const { return fseat_->rowcol(); }

int Move::trowcol() const { return tseat_->rowcol(); }

const std::wstring Move::iccs() const
{
    std::wstringstream wss{};
    wss << PieceManager::getColICCSChar(fseat_->col()) << fseat_->row()
        << PieceManager::getColICCSChar(tseat_->col()) << tseat_->row();
    return wss.str();
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

void Move::done()
{
    eatPie_ = fseat_->movTo(*tseat_);
}

void Move::undo() const
{
    tseat_->movTo(*fseat_, eatPie_);
}

const std::wstring Move::toString() const
{
    std::wstringstream wss{};
    wss << std::setw(2) << frowcol() << L'_' << std::setw(2) << trowcol()
        << L'-' << std::setw(4) << iccs() << L':' << std::setw(4)
        << zh() << L'{' << remark() << L'}';
    return wss.str();
}
}