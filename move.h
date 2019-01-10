#ifndef MOVE_H
#define MOVE_H

#include <vector>
using std::vector;

enum class PieceColor;
class Board;

#include <c++/functional>
using std::function;
using std::wstring;

enum class InstanceFormat { ICCS, zh, CC };

// 着法节点类
class Move {

  public:
    Move();

    void setNext(Move *next_);
    void setOther(Move *other);

    void setSeat__ICCS();
    void setICCS();

    wstring getStr(InstanceFormat fmt);
    //根据中文纵线着法描述取得源、目标位置: (fseat, tseat)
    void setSeat__ZhStr(Board *board);
    //根据源、目标位置: (fseat, tseat)取得中文纵线着法描述
    void setZhStr(Board *board);

    wstring toString();

    // 以下信息存储时不需保存
    Move *prev;
    int stepNo{0}; // 着法深度
    int othCol{0}; // 变着广度
    int maxCol{0}; // 图中列位置（需结合board确定）

  private:
    // （rootMove）调用, 设置树节点的seat or zhStr'
    void __initSet(function<void()> setFunc, Board *board); // C++primer P512   
    // '多兵排序' 
    vector<int> __sortPawnSeats(bool isBottomSide, vector<int> seats); 

    int fs{0};
    int ts{0};
    wstring rem{}; // 注释
    wstring da{};  // 着法数字字母描述
    wstring zh{};  // 着法中文描述
    Move *nt;
    Move *ot;
};

// 棋局着法树类
class Moves {
  public:
    Moves();

    PieceColor currentColor();
    bool isStart();
    bool isLast();

    void setFrom(wstring moveStr, Board *board, InstanceFormat fmt);

    vector<Move *> getPrevMoves(Move *move);

    // 基本走法
    void forward(Board *board);
    void back(Board *board);
    void forwardOther(Board *board);

    // 复合走法
    void to(Move *move, Board *board);
    void toFirst(Board *board);
    void toLast(Board *board);
    void go(Board *board, int inc);

    // 添加着法，复合走法
    void addMove(int fseat, int tseat, wstring remark, Board *board,
                 bool isOther);
    void cutNext();
    void cutOther();

    void fromJSON(wstring moveJSON);
    // （rootMove）调用
    void fromICCSZh(wstring moveStr, Board *board);
    void fromCC(wstring moveStr, Board *board);

    wstring toJSON(); // JSON
    wstring toString();
    wstring toLocaleString();

  private:
    void __init();
    void __initNums(Board *board);
};

#endif