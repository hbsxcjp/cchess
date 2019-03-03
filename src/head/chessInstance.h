#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include <string>
#include <vector>
#include <map>
#include <memory>
using namespace std;


class Board;
class Move;
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
    ChessInstance(const string& filename);

    const PieceColor currentColor() const;
    const bool isStart() const;
    const bool isLast() const;

    // 基本走法
    void forward();
    void backward();
    void forwardOther();
    // 复合走法
    const vector<shared_ptr<Move>> getPrevMoves(shared_ptr<Move> pmove) const;
    void backwardTo(shared_ptr<Move> pmove);
    void to(shared_ptr<Move> pmove);
    void toFirst();
    void toLast();
    void go(const int inc = 1);
    void cutNext();
    void cutOther();

    void write(const string& filename, const RecFormat fmt = RecFormat::ZH);
    static void transDir(const string& dirfrom, const RecFormat fmt = RecFormat::XQF);
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
    void readXQF(const string& filename);
    void readPGN(const string& filename, const RecFormat fmt);
    void readBIN(const string& filename);
    void readJSON(const string& filename);

    void __fromICCSZH(const wstring& moveStr, const RecFormat fmt);
    void __fromCC(const wstring& fullMoveStr);
    const pair<int, int> getSeat__ICCS(const wstring& ICCS) const;
    const wstring getICCS(const int fseat, const int tseat) const;
    // 中文纵线着法->(fseat, tseat), 着法未走状态
    const pair<int, int> getSeat__Zh(const wstring& Zh) const;
    // (fseat, tseat)->中文纵线着法, 着法未走状态
    const wstring getZh(const int fseat, const int tseat) const;
    const wstring __pieceCharsToFEN(const wstring& pieceChars) const;
    const wstring __fenToPieChars(const wstring fen) const;    
    void __initSet(const RecFormat fmt);

    static const wstring getNumChars(const PieceColor color);
    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);
    static const bool isKing(const wchar_t name);
    static const bool isPawn(const wchar_t name);
    static const bool isAdvBishop(const wchar_t name);
    static const bool isStronge(const wchar_t name);
    static const bool isLine(const wchar_t name);
    static const bool isPiece(const wchar_t name);
    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);
    
    void writePGN(const string& filename, const RecFormat fmt = RecFormat::ZH) const;
    const wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH) const;
    const wstring toString_CC() const;
    void writeBIN(const string& filename) const;
    void writeJSON(const string& filename) const;

    shared_ptr<Board> pboard;
    shared_ptr<Move> prootMove;
    shared_ptr<Move> pcurrentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
    map<wstring, wstring> info;
};


#endif