#ifndef MOVE_H
#define MOVE_H

#include "board_base.h"
#include "piece.h"
using namespace Board_base;

#include <vector>
using std::vector;
#include <c++/functional>
using std::function;
using std::wstring;
#include <utility>
using std::pair;

enum class PieceColor;
class Piece;
class Board;

enum class RecFormat { ICCS,
    zh,
    JSON,
    CC };

// 着法节点类
class Move {

public:
    Move(int fseat = nullSeat, int tseat = nullSeat, wstring aremark = L"")
        : remark{ aremark }
        , fs{ fseat }
        , ts{ tseat }
    {
    }
    int fseat() { return fs; }
    int tseat() { return ts; }
    void setSeat(pair<int, int> seats)
    {
        fs = seats.first;
        ts = seats.second;
    }
    Piece* eatp() { return ep; }
    Move* prev() { return pr; }
    Move* next() { return nt; }
    Move* other() { return ot; }
    void setEatp(Piece* pie) { ep = pie; }
    void setPrev(Move* prev) { pr = prev; }
    void setNext(Move* next);
    void setOther(Move* other);

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
    int fs;
    int ts;
    Piece* ep{ Pieces::nullPiePtr };
    Move* pr{ nullptr };
    Move* nt{ nullptr };
    Move* ot{ nullptr };
};

// 棋局着法树类
class Moves {
public:
    Moves();
    Moves(wstring moveStr, RecFormat fmt, Board& board);

    PieceColor currentColor();
    bool isStart() { return currentMove == &rootMove; }
    bool isLast() { return currentMove->next() == nullptr; }

    // 基本走法
    vector<Move*> getPrevMoves(Move* move);
    void forward(Board& board);
    void backward(Board& board);
    void forwardOther(Board& board);
    // 复合走法
    void backwardTo(Move* move, Board& board);
    void to(Move* move, Board& board);
    void toFirst(Board& board);
    void toLast(Board& board);
    void go(Board& board, int inc);

    void cutNext();
    void cutOther();
    const wstring getICCS(int fseat, int tseat);
    const wstring getZh(int fseat, int tseat, Board& board) const; //(fseat, tseat)->中文纵线着法, 着法未走状态
    const pair<int, int> getSeat__ICCS(wstring ICCS, Board& board);
    const pair<int, int> getSeat__Zh(wstring Zh, Board& board) const; //中文纵线着法->(fseat, tseat), 着法未走状态

    void fromICCSZh(wstring moveStr, RecFormat fmt, Board& board);
    void fromJSON(wstring moveJSON, Board& board);
    void fromCC(wstring moveStr, Board& board);
    void setFrom(wstring moveStr, RecFormat fmt, Board& board);

    wstring toString();
    wstring toLocaleString();

    static wstring test();

private:
    void __clear();
    void __initSet(RecFormat fmt, Board& board);
    void __initNums(Board& board);

    vector<Move> moves;
    Move rootMove;
    Move* currentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
    int movCount; //着法数量
    int remCount; //注解数量
    int remLenMax; //注解最大长度
    int othCol; //# 存储最大变着层数
    int maxRow; //# 存储最大着法深度
    int maxCol; //# 存储视图最大列数
};

#endif