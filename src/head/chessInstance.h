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

    void setBoard(const wstring& pieceChars);
    shared_ptr<Board>& getBoard() { return pboard; }
    shared_ptr<Move>& getRootMove() { return prootMove; }
    map<wstring, wstring>& getInfo() { return info; }
    const wstring getFEN();

    // void loadViews(views);
    // void notifyViews();

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int othCol{ 0 }; //# 存储最大变着层数
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数

private:
    void changeSide(const ChangeType ct = ChangeType::EXCHANGE);

    map<wstring, wstring> info;
    shared_ptr<Board> pboard;
    shared_ptr<Move> prootMove;
    shared_ptr<Move> pcurrentMove; // board对应状态：该着已执行
    PieceColor firstColor; // 棋局载入时需要设置此属性！
};

#endif