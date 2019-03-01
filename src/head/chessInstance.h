#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include "../json/json.h"
#include "board_base.h"
#include <memory>
#include <string>

class Info;
class Board;
class Move;

using namespace std;

enum class PieceColor;

enum class RecFormat {
    XQF,
    ICCS,
    ZH,
    CC,
    BIN,
    JSON
};

enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

class ChessInstance {
public:
    ChessInstance();
    ChessInstance(const string filename);

    const PieceColor currentColor() const;
    const bool isStart() const;
    const bool isLast() const;

    // 基本走法
    void forward();
    void backward();
    void forwardOther();
    // 复合走法
    vector<shared_ptr<Move>> getPrevMoves(shared_ptr<Move> pmove);
    void backwardTo(shared_ptr<Move> pmove);
    void to(shared_ptr<Move> pmove);
    void toFirst();
    void toLast();
    void go(const int inc);
    void cutNext();
    void cutOther();

    void write(const string filename, const RecFormat fmt = RecFormat::ZH);
    static void transDir(const string dirfrom, const RecFormat fmt = RecFormat::XQF);
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
    void fromXQF(const string filename);
    void fromPGN(const string filename, const RecFormat fmt);
    void fromBIN(const string filename);
    void fromJSON(const string filename);

    void __fromICCSZH(const wstring moveStr, const RecFormat fmt);
    void __fromCC(const wstring fullMoveStr);
    void __setFEN(const wstring& pieceChars);
    const wstring __getPieChars();    
    void __initSet(const RecFormat fmt);

    const pair<int, int> getSeat__ICCS(const wstring ICCS) const;
    const wstring getICCS(const int fseat, const int tseat) const;
    // 中文纵线着法->(fseat, tseat), 着法未走状态
    const pair<int, int> getSeat__Zh(const wstring Zh) const;
    // (fseat, tseat)->中文纵线着法, 着法未走状态
    const wstring getZh(const int fseat, const int tseat) const;

    static const bool find_char(const wstring& ws, const wchar_t ch) { return ws.find(ch) != wstring::npos; }
    static const wstring getNumChars(const PieceColor color);
    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string ext);
    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);
    
    const wstring toString(const RecFormat fmt = RecFormat::ZH);
    const wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH);
    const wstring toString_CC();
    void toBin(ostream& os);
    void toJson(Json::Value& rootItem);

    shared_ptr<Board> pboard;
    shared_ptr<Move> prootMove;
    shared_ptr<Move> pcurrentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
    map<wstring, wstring> info;
};


#endif