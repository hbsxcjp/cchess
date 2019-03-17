#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <memory>
#include <vector>

using namespace std;
class Seat;
class Piece;
class Move;
enum class PieceColor;
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

    //const bool isBlank(const shared_ptr<Seat>& seat) const { return getPiece(seat) == nullptr; }
    //const shared_ptr<Piece>& getPiece(const shared_ptr<Seat>& seat) const;
    //const PieceColor getColor(const shared_ptr<Seat>& seat) const;

    shared_ptr<Seat>& getSeat(const int row, const int col) { return seats_[row * ColNum + col]; }
    vector<shared_ptr<Seat>> getLiveSeats(const PieceColor color, const wchar_t name = L'\x00', const int col = -1) const;
    //vector<shared_ptr<Seat>> getSideLiveSeats(const PieceColor color) const;
    //vector<shared_ptr<Seat>> getSideNameSeats(const PieceColor color, const wchar_t name) const;
    //vector<shared_ptr<Seat>> getSideNameColSeats(const PieceColor color, const wchar_t name, const int col) const;

    //void go(Move& move);
    //shared_ptr<Piece> go(Seat& fseat, Seat& tseat);
    //void back(Move& move);
    //void back(Seat& fseat, Seat& tseat, shared_ptr<Piece> eatPiece);
    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const wstring getChars() const;
    void putPieces(const wstring& chars);
    shared_ptr<Seat>& getOthSeat(const shared_ptr<Seat>& seat, const ChangeType ct);
    void changeSide(const ChangeType ct);
    void setBottomSide();

    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> getMoveSeats(const int frowcol, const int trowcol);
    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> getMoveSeats(const Move& move, const RecFormat fmt);
    const wstring getIccs(const Move& move) const;
    // (fseat, tseat)->中文纵线着法, 着法未走状态
    const wstring getZh(const Move& move);
    //const wstring test();

    const wstring toString() const;

private:
    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> __getSeatFromICCS(const wstring& ICCS);
    // 中文纵线着法->(fseat, tseat), 着法未走状态
    const pair<const shared_ptr<Seat>, const shared_ptr<Seat>> __getSeatFromZh(const wstring& Zh);
    //const vector<shared_ptr<Seat>> __getSeats(const vector<shared_ptr<Piece>>& pies) const;

    static map<PieceColor, wstring> __numChars;
    //const shared_ptr<Piece> __getFreePie(wchar_t ch) const;

    const vector<shared_ptr<Piece>> __creatPieces();
    vector<shared_ptr<Seat>> __creatSeats();
    // 棋盘数值常量
    const int RowNum{ 10 };
    const int ColNum{ 9 };

    PieceColor bottomColor; // 底端棋子颜色
    const vector<shared_ptr<Piece>> pieces_; // 一副棋子，固定的32个
    vector<shared_ptr<Seat>> seats_; // 一块棋盘位置容器，固定的90个
};

#endif