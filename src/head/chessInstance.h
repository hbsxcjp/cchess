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

    /*
    // 基本走法
    void forward();
    void backward();
    void forwardOther();
    // 复合走法
    void backwardTo(shared_ptr<Move> pmove);
    void to(shared_ptr<Move> pmove);
    void toFirst();
    void toLast();
    void go(const int inc = 1);
    void cutNext();
    void cutOther();
    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);

    //const wstring getMoveInfo();

    void write(const string& filename, const RecFormat fmt = RecFormat::ZH);
    static void transDir(const string& dirfrom, const RecFormat fmt = RecFormat::XQF);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

    */
    // void loadViews(views);
    // void notifyViews();
    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    //const int getOthCol() const { return othMaxCol; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

private:
    /*
    void setFEN(const wstring& pieceChars);
    void setBoard();
    void initSetMove(const RecFormat fmt);
    void readPGN(const string& filename, const RecFormat fmt);
    void readBIN(const string& filename);
    void readJSON(const string& filename);
    void __fromICCSZH(const wstring& moveStr, const RecFormat fmt);
    void __fromCC(const wstring& fullMoveStr);

    void writePGN(const string& filename, const RecFormat fmt = RecFormat::ZH);
    const wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH);
    const wstring toString_CC();
    void writeBIN(const string& filenameconst);
    void writeJSON(const string& filenameconst);
    */

    void readXQF(const string& filename);

    const wstring creatFEN(const wstring& pieceChars);
    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);

    map<wstring, wstring> info_;
    shared_ptr<Board> board_;
    shared_ptr<Move> rootMove_;
    shared_ptr<Move> currentMove_; // board对应该着已执行的状态
    PieceColor firstColor_;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    //int othMaxCol{ 0 }; //# 存储最大变着层数
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

#endif