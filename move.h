#ifndef MOVE_H
#define MOVE_H

#include <vector>
using std::vector;

enum class PieceColor;
class Piece;
class Board;

#include <c++/functional>
using std::function;
using std::wstring;

enum class RecFormat { ICCS,
    zh,
    JSON,
    CC };

// 着法节点类
class Move {

public:
    Move(int fseat, int tseat, wstring aremark);

    int fseat() { return fs; }
    int tseat() { return ts; }
    void setFseat(int fseat) { fs = fseat; }
    void setTseat(int tseat) { ts = tseat; }

    Piece* eatPiece() { return eatPiePtr; }
    Piece* setEatPiece(Piece* pie) { return eatPiePtr = pie; }

    Move* prev() { return pr; }
    Move* next() { return nt; }
    Move* other() { return ot; }
    void setPrev(Move* prev) { pr = prev; }
    void setNext(Move* next_);
    void setOther(Move* other);

    void setSeat__ICCS(Board* board);
    void setICCS();
    wstring getStr(RecFormat fmt);
    //根据中文纵线着法描述取得源、目标位置: (fseat, tseat)
    void setSeat__ZhStr(Board* board);
    //根据源、目标位置: (fseat, tseat)取得中文纵线着法描述
    void setZhStr(Board* board);
    void setZhStr(wstring ws);

    // （rootMove）调用
    void fromJSON(wstring moveJSON, Board* board);
    void fromICCSZh(wstring moveStr, Board* board);
    void fromCC(wstring moveStr, Board* board);
    // （rootMove）调用, 设置树节点的seat or zhStr'
    void initSet(function<void(Move*, Board*)> setFunc, Board* board); // C++primer P512
    wstring toJSON(); // JSON
    wstring toString();

    wstring remark{}; // 注释
    // 以下信息存储时不需保存
    int stepNo{ 0 }; // 着法深度
    int othCol{ 0 }; // 变着广度
    int maxCol{ 0 }; // 图中列位置（需结合board确定）

private:
    int fs;
    int ts;
    wstring da{}; // 着法数字字母描述
    wstring zh{}; // 着法中文描述
    Piece* eatPiePtr{ nullptr };
    Move* pr{ nullptr };
    Move* nt{ nullptr };
    Move* ot{ nullptr };
};

// 棋局着法树类
class Moves {
public:
    Moves() { __clear(); }

    PieceColor currentColor();
    bool isStart() { return currentMove == rootMove; }
    bool isLast() { return currentMove->next() == nullptr; }

    // 基本走法
    vector<Move*> getPrevMoves(Move* move);
    void forward(Board* board);
    void backward(Board* board);
    void forwardOther(Board* board);

    // 复合走法
    void to(Move* move, Board* board);
    void toFirst(Board* board);
    void toLast(Board* board);
    void go(Board* board, int inc);

    // 添加着法，复合走法
    void addMove(Move move) { moves.push_back(move); }
    void addMove(int fseat, int tseat, wstring remark, Board* board,
        bool isOther);
    void cutNext();
    void cutOther();
    void setFrom(wstring moveStr, Board* board, RecFormat fmt);

    wstring toString();
    wstring toLocaleString();

    wstring test_moves();

private:
    void __clear();
    void __initNums(Board* board);

    vector<Move> moves;
    Move* rootMove;
    Move* currentMove;
    PieceColor firstColor; // 棋局载入时需要设置此属性！
    int movCount; //着法数量
    int remCount; //注解数量
    int remLenMax; //注解最大长度
    int othCol; //# 存储最大变着层数
    int maxRow; //# 存储最大着法深度
    int maxCol; //# 存储视图最大列数
};

#endif