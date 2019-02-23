#ifndef BOARD_H
#define BOARD_H

#include "info.h"
#include "piece.h"
#include <memory>
#include <string>
using namespace std;

// 棋盘端部
enum class BoardSide {
    bottom,
    top
};

class Move;

class Board {
public:
    Board();
    Board(Info& info);

    Piece* getPiece(int seat) { return pieSeats[seat]; }
    Piece* getOthPie(Piece* piecep) { return pieces.getOthPie(piecep); }
    vector<Piece*> getLivePies() { return pieces.getLivePies(); }
    bool isBlank(int seat) { return getPiece(seat)->isBlank(); }
    PieceColor getColor(int seat) { return getPiece(seat)->color(); }
    bool isBottomSide(PieceColor color) { return bottomColor == color; }
    BoardSide getSide(PieceColor color) { return isBottomSide(color) ? BoardSide::bottom : BoardSide::top; }
    vector<int> getSideNameSeats(PieceColor color, wchar_t name);
    vector<int> getSideNameColSeats(PieceColor color, wchar_t name, int col);

    void go(Move& move);
    void back(Move& move);
    Piece* move_go(int fseat, int tseat);
    void move_back(int fseat, int tseat, Piece* eatPiece);
    bool isKilled(PieceColor color); //判断是否将军
    bool isDied(PieceColor color); //判断是否被将死

    void setFEN(Info& info);
    void setFrom(Info& info);
    void setSeatPieces(vector<pair<int, Piece*>> seatPieces);
    const wstring toString();

    wstring test();

    PieceColor bottomColor; // 底端棋子颜色
private:
    vector<int> __getSeats(vector<Piece*> pies);
    void __setPiece(Piece* pie, int tseat);

    Pieces pieces; // 一副棋子类
    vector<Piece*> pieSeats; // 棋盘容器，顺序号即为位置seat
}; // Board class end.

#endif