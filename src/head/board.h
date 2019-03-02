#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <vector>
#include <memory>
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
    Board(const wstring& pieceChars);

    shared_ptr<Piece> getPiece(const int seat);
    shared_ptr<Piece> getOthPie(const shared_ptr<Piece> piecep);
    vector<shared_ptr<Piece>> getLivePies();
    const bool isBlank(const int seat);
    const PieceColor getColor(const int seat);
    const bool isBottomSide(const PieceColor color) { return bottomColor == color; }
    const BoardSide getSide(const PieceColor color) { return isBottomSide(color) ? BoardSide::BOTTOM : BoardSide::TOP; }
    vector<int> getSideNameSeats(const PieceColor color, const wchar_t name);
    vector<int> getSideNameColSeats(const PieceColor color, const wchar_t name, const int col);

    void go(Move& move);
    void back(Move& move);
    shared_ptr<Piece> move_go(const int fseat, const int tseat);
    void move_back(const int fseat, const int tseat, shared_ptr<Piece> eatPiece);
    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const wstring getPieceChars();
    void setSeatPieces(vector<pair<int, shared_ptr<Piece>>> seatPieces);

    const wstring toString();
    const wstring test();

    PieceColor bottomColor; // 底端棋子颜色
private:
    vector<int> __getSeats(vector<shared_ptr<Piece>> pies);
    void __setPiece(shared_ptr<Piece> pie, const int tseat);

    shared_ptr<Pieces> pPieces; // 一副棋子类
    vector<shared_ptr<Piece>> pieSeats; // 棋盘容器，顺序号即为位置seat
}; // Board class end.

#endif