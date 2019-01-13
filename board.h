#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

#include <string>
using std::wstring;

// 棋盘端部
enum class BoardSide { bottom, top };
enum class ChangeType { exchange, rotate, symmetry };
class Piece;
class Pieces;
class Move;

class Board {
  public:
    Board() : bottomColor{PieceColor::red}, pieces{Pieces()} {
        pieSeats.resize(90);
        std::fill(pieSeats.begin(), pieSeats.end(), Pieces::nullPiePtr);
    }
    // Board(wstring pgn) {}

    Piece *getPiece(int seat) { return pieSeats[seat]; }
    bool isBlank(int seat) { return getPiece(seat)->isBlank(); }
    PieceColor getColor(int seat) { return getPiece(seat)->color(); }
    bool isBottomSide(PieceColor color) { return bottomColor == color; }
    BoardSide getSide(PieceColor color) {
        return isBottomSide(color) ? BoardSide::bottom : BoardSide::top;
    }   

    vector<int> getSideNameSeats(PieceColor color, wchar_t name);
    vector<int> getSideNameColSeats(PieceColor color, wchar_t name, int col);

    Piece *go(Move* move);
    void back(Move* move);
    Piece *move_go(int fseat, int tseat);
    void move_back(int fseat, int tseat, Piece *eatPiece);

    bool isKilled(PieceColor color); //判断是否将军
    bool isDied(PieceColor color);   //判断是否被将死

    wstring getFEN();
    void setFEN(wstring FEN);
    void changeSide(ChangeType ct = ChangeType::exchange);
    const wstring toString();

    wstring test_board();

    PieceColor bottomColor; // 底端棋子颜色
  private:
    wstring __FEN();
    vector<int> __getSeats(vector<Piece *> pies);
    void __setPieces(wstring chars);
    void __setPiece(Piece *pie, int tseat);
    void __clearPieces();

    Pieces pieces;            // 一副棋子类
    vector<Piece *> pieSeats; // 棋盘容器，顺序号即为位置seat
};                            // Board class end.

#endif