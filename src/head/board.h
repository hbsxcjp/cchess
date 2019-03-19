#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include <map>
#include <memory>
#include <vector>

using namespace std;
class Seat;
class Piece;
class Move;
enum class RecFormat;
enum class BoardSide {
    BOTTOM,
    TOP
};
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

class Board {
public:
    Board();

    const bool isBottomSide(const PieceColor color) const { return bottomColor == color; }
    const BoardSide getSide(const PieceColor color) const { return isBottomSide(color) ? BoardSide::BOTTOM : BoardSide::TOP; }

    shared_ptr<Seat>& getSeat(const int row, const int col) { return seats_[row * ColNum + col]; }
    shared_ptr<Seat>& getSeat(const int rowcol) { return seats_[rowcol / 10 * ColNum + rowcol % 10]; }
    shared_ptr<Seat>& getOthSeat(const shared_ptr<Seat>& seat, const ChangeType ct);
    vector<shared_ptr<Seat>> getLiveSeats(const PieceColor color = PieceColor::BLANK, const wchar_t name = L'\x00', const int col = -1) const;
    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> getMoveSeats(const Move& move, const RecFormat fmt);
    const wstring getIccs(const Move& move) const;
    const wstring getZh(const Move& move); // (fseat, tseat)->中文纵线着法, 着法未走状态

    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const wstring getFEN(const wstring& pieceChars) const;
    void putPieces(const wstring& fen);
    const wstring changeSide(const ChangeType ct);
    void setBottomSide();

    const wstring toString() const;
    const wstring test();// const;
    static shared_ptr<Piece> nullPiece;

private:
    const wstring __getChars(const wstring& fen) const;

    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> __getSeatFromICCS(const wstring& ICCS);
    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> __getSeatFromZh(const wstring& Zh); // 中文纵线着法->(fseat, tseat), 着法未走状态
    const vector<shared_ptr<Seat>> __sortPawnSeats(const PieceColor color, const wchar_t name);
    const vector<shared_ptr<Piece>> __creatPieces();
    const vector<shared_ptr<Seat>> __creatSeats();
    static wchar_t __nullChar;
    static map<PieceColor, wstring> __numChars;
    // 棋盘数值常量
    const int RowNum{ 10 };
    const int ColNum{ 9 };

    PieceColor bottomColor; // 底端棋子颜色
    const vector<shared_ptr<Piece>> pieces_; // 一副棋子，固定的32个
    vector<shared_ptr<Seat>> seats_; // 一块棋盘位置容器，固定的90个
};

#endif