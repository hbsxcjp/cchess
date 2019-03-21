#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>
using namespace std;
enum class PieceColor;
class Piece;

class Seat {
public:
    explicit Seat(const int row, const int col, shared_ptr<Piece>& piece)
        : row_{ row }
        , col_{ col }
        , piece_{ piece }
    {
    }

    const int row() const { return row_; }
    const int col() const { return col_; }
    const int rc() const { return row_ * 10 + col_; } // 十位为行，个位为列
    const shared_ptr<Piece>& piece() const { return piece_; }
    void put(const shared_ptr<Piece>& piece) { piece_ = piece; } // 置入棋子
    const wstring toString() const;

    static const PieceColor getColor(const wchar_t numZh);
    static const wchar_t getIndexChar(const int length, const int index) { return (length == 2 ? __preChars.substr(0, 2) : (length == 3 ? __preChars.substr(2, 3) : __preChars.substr(5, 5)))[index]; }
    static const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isForward) { return __movChars.at(isSameRow ? 1 : (isBottom == isForward ? 2 : 0)); }
    static const wchar_t getNumChar(const PieceColor color, const int num) { return __getChar(color, num - 1); };
    static const wchar_t getColChar(const PieceColor color, bool isBottom, const int col) { return __getChar(color, isBottom ? ColNum - col - 1 : col); };
    static const int getIndex(const wchar_t preChar);

    static const int getMovDir(const bool isBottom, const wchar_t movChar) { return static_cast<int>(__movChars.find(movChar) - 1) * (isBottom ? 1 : -1); }
    static const int getNum(const PieceColor color, const wchar_t numChar) { return static_cast<int>(__numChars.at(color).find(numChar)) + 1; }
    static const int getCol(bool isBottom, const int num) { return isBottom ? ColNum - num : num - 1; }
    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    //int movDir{ __movIndex.at(zhStr[2]) * (isBottom ? 1 : -1) }, num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    //int movDir{ __wchIndex(zhStr[2]) * (isBottom ? 1 : -1) },
    static const vector<shared_ptr<Seat>> __creatSeats();

    static const int RowNum{ 10 };
    static const int ColNum{ 9 };

private:
    int row_; //低四位
    int col_; //高四位
    shared_ptr<Piece> piece_;

    static const wchar_t __getChar(const PieceColor color, const int index) { return __numChars.at(color)[index]; };
    static const wstring __preChars;
    static const wstring __movChars;
    //static const map<wchar_t, int> __movIndex;
    static const map<PieceColor, wstring> __numChars;
};

/*
class Seats {
public:
    Seats();

    const Seat& getSeat(int col, int row) const { return seats[col][row]; }
    const vector<const Seat*> allSeats() const;
    const vector<const Seat*> kingSeats(BoardSide bs) const;
    const vector<const Seat*> advisorSeats(BoardSide bs) const;
    const vector<const Seat*> bishopSeats(BoardSide bs) const;
    const vector<const Seat*> pawnSeats(BoardSide bs) const;

    const bool isSameCol(const Seat& aseat, const Seat& bseat) const { return aseat.col_ == bseat.col_; }
    const Seat& rotateSeat(const Seat& seat) const { return seats[MaxCol - seat.col_][MaxRow - seat.row_]; }
    const Seat& symmetrySeat(const Seat& seat) const { return seats[MaxCol - seat.col_][seat.row_]; }
    const vector<const Seat*> getSameColSeats(const Seat& aseat, const Seat& bseat) const;

    // 位置行走函数
    const vector<const Seat*> getKingMoveSeats(const Seat& seat) const;
    const vector<const Seat*> getAdvisorMoveSeats(const Seat& seat) const;
    // 获取移动、象心行列值
    const vector<pair<const Seat*, const Seat*>> getBishopMove_CenSeats(const Seat& seat) const;
    // 获取移动、马腿行列值
    const vector<pair<const Seat*, const Seat*>> getKnightMove_LegSeats(const Seat& seat) const;
    // 车炮可走的四个方向位置
    const vector<vector<const Seat*>> getRookCannonMoveSeat_Lines(const Seat& seat) const;
    const vector<const Seat*> getPawnMoveSeats(const bool isBottomSide, const Seat& seat) const;
    // '多兵排序'
    const vector<const Seat*> sortPawnSeats(const bool isBottomSide, vector<Seat> pawnSeats) const;

private:
    vector<vector<Seat>> seats;
    // 棋盘数值常量
    static const int ColNum{ 0x09 };
    static const int RowNum{ 0x0A };
    static const int MinCol{ 0x00 };
    static const int MaxCol{ 0x08 };
    static const int MinRow{ 0x00 };
    static const int MaxRow{ 0x09 };
};
*/

/*
namespace Board_base {
// 空位置
const int nullSeat{ -1 };

// 棋盘数值常量
const int ColNum{ 9 };
const int RowNum{ 10 };
const int MinCol{ 0 };
const int MaxCol{ 8 };
const int MinRow{ 0 };
const int MaxRow{ 9 };

// 棋盘位置相关组合
const vector<int> allSeats{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
    72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89
};
const vector<int> bottomKingSeats{ 21, 22, 23, 12, 13, 14, 3, 4, 5 };
const vector<int> topKingSeats{ 84, 85, 86, 75, 76, 77, 66, 67, 68 };
const vector<int> bottomAdvisorSeats{ 21, 23, 13, 3, 5 };
const vector<int> topAdvisorSeats{ 84, 86, 76, 66, 68 };
const vector<int> bottomBishopSeats{ 2, 6, 18, 22, 26, 38, 42 };
const vector<int> topBishopSeats{ 47, 51, 63, 67, 71, 83, 87 };
const vector<int> bottomPawnSeats{
    27, 29, 31, 33, 35, 36, 38, 40, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89
};
const vector<int> topPawnSeats{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 47, 49, 51, 53, 54, 56, 58, 60, 62
};

// 函数
inline const int getRow(const int seat) { return seat / 9; }
inline const int getCol(const int seat) { return seat % 9; }
inline const int getSeat(const int row, const int col) { return row * 9 + col; }
inline const int rotateSeat(const int seat) { return 89 - seat; }
inline const int symmetrySeat(const int seat) { return (getRow(seat) + 1) * 9 - seat % 9 - 1; }
inline const bool isSameCol(const int seat, const int othseat) { return getCol(seat) == getCol(othseat); }
const vector<int> getSameColSeats(const int seat, const int othseat);

// 位置行走函数
const vector<int> getKingMoveSeats(const int seat);
const vector<int> getAdvisorMoveSeats(const int seat);
// 获取移动、象心行列值
const vector<pair<int, int>> getBishopMove_CenSeats(const int seat);
// 获取移动、马腿行列值
const vector<pair<int, int>> getKnightMove_LegSeats(const int seat);
// 车炮可走的四个方向位置
const vector<vector<int>> getRookCannonMoveSeat_Lines(const int seat);
const vector<int> getPawnMoveSeats(const bool isBottomSide, const int seat);
// '多兵排序'
const vector<int> sortPawnSeats(const bool isBottomSide, vector<int> pawnSeats);

// 测试函数
const wstring test();

} // namespace Board_base

*/

#endif