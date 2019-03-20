#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class Board;
class Move;
enum class PieceColor;
enum class ChangeType;
enum class RecFormat {
    XQF,
    ICCS,
    ZH,
    CC,
    BIN,
    JSON
};

class Instance {
public:
    Instance();
    Instance(const string& filename);
    void write(const string& fname, const RecFormat fmt);

    const PieceColor currentColor() const;
    const bool isStart() const;
    const bool isLast() const;
    void forward();
    void backward();
    void forwardOther();
    void backwardTo(shared_ptr<Move> move);
    void to(shared_ptr<Move> move);
    void backFirst();
    void forLast();
    void go(const int inc);
    void cutNext();
    void cutOther();
    void changeSide(const ChangeType ct);

    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);

    const wstring toString() const;
    const wstring test() const;

private:
    void readXQF(const string& filename);
    void readPGN(const string& filename, const RecFormat fmt);
    void readBIN(const string& filename);
    void readJSON(const string& filename);
    void __readICCSZH(const wstring& moveStr, const RecFormat fmt);
    void __readCC(const wstring& fullMoveStr);

    const wstring __moveInfo() const;
    void writePGN(const string& filename, const RecFormat fmt = RecFormat::ZH) const;
    const wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH) const;
    const wstring toString_CC() const;
    void writeBIN(const string& filenameconst) const;
    void writeJSON(const string& filenameconst) const;

    void setFEN(const wstring& pieceChars);
    void setBoard();
    void setMoves(const RecFormat fmt);

    map<wstring, wstring> info_;
    shared_ptr<Board> board_;
    shared_ptr<Move> rootMove_;
    shared_ptr<Move> currentMove_; // board对应该着已执行的状态
    PieceColor firstColor_;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

#endif