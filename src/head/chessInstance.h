#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include <map>
#include <memory>
#include <string>
#include <vector>
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

    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);

    //shared_ptr<Move>& getRootMove() { return prootMove; }
    //map<wstring, wstring>& getInfo() { return info; }

    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getOthCol() const { return othCol; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }
    const wstring getMoveInfo();

    void write(const string& filename, const RecFormat fmt = RecFormat::ZH);
    static void transDir(const string& dirfrom, const RecFormat fmt = RecFormat::XQF);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
    // void loadViews(views);
    // void notifyViews();

private:
    void setFEN(const wstring& pieceChars);
    void setBoard();
    void initSetMove(const RecFormat fmt);

    void readXQF(const string& filename);
    void readPGN(const string& filename, const RecFormat fmt);
    void readBIN(const string& filename);
    void readJSON(const string& filename);
    void __fromICCSZH(const wstring& moveStr, const RecFormat fmt);
    void __fromCC(const wstring& fullMoveStr);
    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);

    void writePGN(const string& filename, const RecFormat fmt = RecFormat::ZH);
    const wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH);
    const wstring toString_CC();
    void writeBIN(const string& filenameconst);
    void writeJSON(const string& filenameconst);

    map<wstring, wstring> info;
    shared_ptr<Board> pboard;
    shared_ptr<Move> prootMove;
    shared_ptr<Move> pcurrentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int othCol{ 0 }; //# 存储最大变着层数
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

#endif