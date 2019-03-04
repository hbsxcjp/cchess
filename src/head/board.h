#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <string>
#include <vector>
using namespace std;

class Piece;
class Pieces;
class Move;
enum class PieceColor;
enum class BoardSide {
    BOTTOM,
    TOP
};

class Board {
public:
    Board();

    shared_ptr<Piece> getPiece(const int seat) const { return pieSeats[seat]; }
    shared_ptr<Piece> getOthPie(const shared_ptr<Piece>& piecep) const;
    vector<shared_ptr<Piece>> getLivePies() const;
    const bool isBlank(const int seat) const;
    const PieceColor getColor(const int seat) const;
    const bool isBottomSide(const PieceColor color) const { return bottomColor == color; }
    const BoardSide getSide(const PieceColor color) const { return isBottomSide(color) ? BoardSide::BOTTOM : BoardSide::TOP; }
    vector<int> getSideNameSeats(const PieceColor color, const wchar_t name) const;
    vector<int> getSideNameColSeats(const PieceColor color, const wchar_t name, const int col) const;

    void go(Move& move);
    void back(Move& move);
    shared_ptr<Piece> move_go(const int fseat, const int tseat);
    void move_back(const int fseat, const int tseat, shared_ptr<Piece> eatPiece);
    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const wstring getPieceChars() const;
    void set(const wstring& pieceChars);
    void set(vector<pair<int, shared_ptr<Piece>>> seatPieces);

    const wstring toString() const;
    const wstring test();

    PieceColor bottomColor; // 底端棋子颜色
private:
    const vector<int> __getSeats(const vector<shared_ptr<Piece>>& pies) const;
    void __setPiece(shared_ptr<Piece> pie, const int tseat);

    shared_ptr<Pieces> pPieces; // 一副棋子类
    vector<shared_ptr<Piece>> pieSeats; // 棋盘容器，顺序号即为位置seat
}; // Board class end.

#endif