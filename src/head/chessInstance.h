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
enum class RecFormat;
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

    void setFEN(const wstring& pieceChars);
    const wstring getPieceChars();
    void setBoard();
    void initSetMove(const RecFormat fmt);
    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);

    shared_ptr<Move>& getRootMove() { return prootMove; }
    map<wstring, wstring>& getInfo() { return info; }

    const int getMovCount() const { return movCount; }
    void setMovCount() { ++movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    void setRemData(const int length);
    /*
    void setRemCount() { ++remCount; }
    void setRemLenMax(const int curRemLenMax)
    {
        if (curRemLenMax > remLenMax)
            remLenMax = curRemLenMax;
    }*/
    const int getOthCol() const { return othCol; }
    void setOthCol(const int curOthCol)
    {
        if (othCol < curOthCol)
            othCol = curOthCol;
    }
    const int getMaxRow() const { return maxRow; }
    void setMaxRow(const int curMaxRow)
    {
        if (maxRow < curMaxRow)
            maxRow = curMaxRow;
    }
    const int getMaxCol() const { return maxCol; }
    void setMaxCol() { ++maxCol; }
    const wstring getMoveInfo();

    // void loadViews(views);
    // void notifyViews();

private:
    static const wstring __fenToPieceChars(const wstring fen);
    static const wstring __pieceCharsToFEN(const wstring& pieceChars);

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