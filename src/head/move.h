#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include <memory>

using namespace std;
//using namespace Board_base;
enum class PieceColor;

// 着法节点类
class Move {

public:
    int fseat() { return fromseat; }
    int tseat() { return toseat; }
    void setSeat(int fseat, int tseat)
    {
        fromseat = fseat;
        toseat = tseat;
    }
    void setSeat(pair<int, int> seats) { setSeat(seats.first, seats.second); }

    Piece* eatPiece() { return eatPie_ptr; }
    shared_ptr<Move> prev() { return prev_ptr; }
    shared_ptr<Move> next() { return next_ptr; }
    shared_ptr<Move> other() { return other_ptr; }
    void setEatPiece(Piece* pie) { eatPie_ptr = pie; }
    void setPrev(shared_ptr<Move> prev) { prev_ptr = prev; }
    void setNext(shared_ptr<Move> next);
    void setOther(shared_ptr<Move> other);

    wstring toString();

    wstring ICCS{}; // 着法数字字母描述
    wstring zh{}; // 着法中文描述
    wstring remark{}; // 注释
    int stepNo{ 0 }; // 着法深度
    int othCol{ 0 }; // 变着广度
    int maxCol{ 0 }; // 图中列位置（需结合board确定）

private:
    int fromseat{ nullSeat };
    int toseat{ nullSeat };
    Piece* eatPie_ptr{ Pieces::nullPiePtr };
    shared_ptr<Move> prev_ptr{ nullptr };
    shared_ptr<Move> next_ptr{ nullptr };
    shared_ptr<Move> other_ptr{ nullptr };
};

#endif