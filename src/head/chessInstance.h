#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include "board.h"
#include "info.h"
#include "move.h"

#include <string>
using std::string;
using std::wstring;

class ChessInstance {
public:
    ChessInstance();
    ChessInstance(string filename);

    PieceColor currentColor();
    bool isStart() { return currentMove->prev() == nullptr; }
    bool isLast() { return currentMove->next() == nullptr; }

    // 基本走法
    vector<shared_ptr<Move>> getPrevMoves(shared_ptr<Move> move);
    void forward();
    void backward();
    void forwardOther();
    // 复合走法
    void backwardTo(shared_ptr<Move> move);
    void to(shared_ptr<Move> move);
    void toFirst();
    void toLast();
    void go(int inc);
    void cutNext();
    void cutOther();

    wstring toString(RecFormat fmt = RecFormat::ZH);
    void toBin(ostream& os);
    void toJson(Json::Value& rootItem);

    void write(string filename, string ext, RecFormat fmt = RecFormat::ZH);

    static void transDir(string dirfrom, string ext, RecFormat fmt = RecFormat::ZH);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

    // void loadViews(views);
    // void notifyViews();

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int othCol{ 0 }; //# 存储最大变着层数
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数

private:
    const wstring getICCS(int fseat, int tseat);
    const pair<int, int> getSeat__ICCS(wstring ICCS);
    const wstring getZh(int fseat, int tseat);// const; //(fseat, tseat)->中文纵线着法, 着法未走状态
    const pair<int, int> getSeat__Zh(wstring Zh);// const; //中文纵线着法->(fseat, tseat), 着法未走状态

    void fromICCSZH(wstring moveStr, RecFormat fmt);
    void fromCC(wstring fullMoveStr);
    void fromBIN(istream& is);
    void fromJson(Json::Value& root);
    void fromXQF(istream& is, vector<int>& Keys, vector<int>& F32Keys);
    void __initSet(RecFormat fmt);

    void __init(istream& is, vector<int>& Keys, vector<int>& F32Keys);
    void __init(wstring moveStr, Info& info);
    void __init(istream& is);
    void __init(Json::Value& root);

    wstring toString_ICCSZH(RecFormat fmt = RecFormat::ZH);
    wstring toString_CC();

    //vector<shared_ptr<Move>> moves;
    shared_ptr<Move> rootMove;
    shared_ptr<Move> currentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！

    //private:
    Info info;
    Board board;
    //Moves moves;
};

#endif