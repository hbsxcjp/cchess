#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include "../json/json.h"
#include "board.h"
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
    void forward();
    void backward();
    void forwardOther();
    // 复合走法
    vector<shared_ptr<Move>> getPrevMoves(shared_ptr<Move> move);
    void backwardTo(shared_ptr<Move> move);
    void to(shared_ptr<Move> move);
    void toFirst();
    void toLast();
    void go(int inc);
    void cutNext();
    void cutOther();

    void write(string filename, RecFormat fmt = RecFormat::ZH);
    static void transDir(string dirfrom, RecFormat fmt = RecFormat::XQF);
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
    void __init_xqf(string filename);
    void __init_pgn(string filename, RecFormat fmt);
    void __init_bin(string filename);
    void __init_json(string filename);

    void fromXQF(istream& is, vector<int>& Keys, vector<int>& F32Keys);
    void fromICCSZH(wstring moveStr, RecFormat fmt);
    void fromCC(wstring fullMoveStr);
    void fromBIN(istream& is);
    void fromJSON(Json::Value& root);
    void __initSet(RecFormat fmt);
    const pair<int, int> getSeat__ICCS(wstring ICCS);
    const wstring getICCS(int fseat, int tseat);
    // 中文纵线着法->(fseat, tseat), 着法未走状态
    const pair<int, int> getSeat__Zh(wstring Zh);
    // (fseat, tseat)->中文纵线着法, 着法未走状态
    const wstring getZh(int fseat, int tseat);

    wstring toString(RecFormat fmt = RecFormat::ZH);
    wstring toString_ICCSZH(RecFormat fmt = RecFormat::ZH);
    wstring toString_CC();
    void toBin(ostream& os);
    void toJson(Json::Value& rootItem);

    shared_ptr<Move> rootMove;
    shared_ptr<Move> currentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
    Info info;
    Board board;
};

#endif