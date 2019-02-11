#ifndef MOVE_H
#define MOVE_H

#include "board_base.h"
#include "piece.h"
#include <iostream>
#include <vector>
#include <utility>
#include <memory>

using namespace std;
using namespace Board_base;

enum class PieceColor;
class Info;
class Piece;
class Board;

enum class RecFormat { ICCS,
    zh,
    CC,
    XQF,
    JSON,
    bin };

// 着法节点类
class Move {

public:
    int fseat() { return fromseat; }
    int tseat() { return toseat; }
    void setSeat(pair<int, int> seats)
    {
        fromseat = seats.first;
        toseat = seats.second;
    }
    Piece* eatPiece() { return eatPie_ptr; }
    shared_ptr<Move> prev() { return prev_ptr; }
    shared_ptr<Move> next() { return next_ptr; }
    shared_ptr<Move> other() { return other_ptr; }
    void setEatPiece(Piece* pie) { eatPie_ptr = pie; }
    void setPrev(shared_ptr<Move> prev) { prev_ptr = prev; }
    void setNext(shared_ptr<Move> next);
    void setOther(shared_ptr<Move> other);

    wstring toJSON(); // JSON
    wstring toString();

    wstring ICCS{}; // 着法数字字母描述
    wstring zh{}; // 着法中文描述
    wstring remark{}; // 注释
    // 以下信息存储时不需保存
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

// 棋局着法树类
class Moves {
public:
    Moves();
    Moves(wstring moveStr, Info& info, Board& board);
    Moves(istream& is, vector<int>& Keys, vector<int>& F32Keys, Board& board);
    Moves(istream& is);

    PieceColor currentColor();
    bool isStart() { return currentMove->prev() == nullptr; }
    bool isLast() { return currentMove->next() == nullptr; }

    // 基本走法
    vector<shared_ptr<Move>> getPrevMoves(shared_ptr<Move> move);
    void forward(Board& board);
    void backward(Board& board);
    void forwardOther(Board& board);
    // 复合走法
    void backwardTo(shared_ptr<Move> move, Board& board);
    void to(shared_ptr<Move> move, Board& board);
    void toFirst(Board& board);
    void toLast(Board& board);
    void go(Board& board, int inc);
    void cutNext();
    void cutOther();

    wstring toString_zh(RecFormat fmt = RecFormat::zh);
    wstring toString_ICCS();
    wstring toString_CC();
    void toBin(ostream& os);

    static wstring test();
    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int othCol{ 0 }; //# 存储最大变着层数
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数

private:
    const wstring getICCS(int fseat, int tseat);
    const pair<int, int> getSeat__ICCS(wstring ICCS);
    const wstring getZh(int fseat, int tseat, Board& board) const; //(fseat, tseat)->中文纵线着法, 着法未走状态
    const pair<int, int> getSeat__Zh(wstring Zh, Board& board) const; //中文纵线着法->(fseat, tseat), 着法未走状态

    void fromICCSZh(wstring moveStr, RecFormat fmt);
    void fromJSON(wstring moveJSON);
    void fromCC(wstring moveStr);
    void fromXQF(std::istream& is, vector<int>& Keys, vector<int>& F32Keys);
    void __initSet(RecFormat fmt, Board& board);
    void __init__();

    //vector<shared_ptr<Move>> moves;
    shared_ptr<Move> rootMove;
    shared_ptr<Move> currentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
};

#endif