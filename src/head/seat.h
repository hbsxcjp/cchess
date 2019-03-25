#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>
enum class PieceColor;

namespace PieceSpace {
    class Piece;
}

namespace SeatSpace {

class Seat {
public:
    explicit Seat(int row, int col, const std::shared_ptr<PieceSpace::Piece>& piece)
        : row_{ row }
        , col_{ col }
        , piece_{ piece }
    {
    }

    const int row() const { return row_; }
    const int col() const { return col_; }
    const int rowcolValue() const { return row_ * 10 + col_; } // 十位为行，个位为列
    const std::shared_ptr<PieceSpace::Piece>& piece() const { return piece_; }
    void put(const std::shared_ptr<PieceSpace::Piece>& piece) { piece_ = piece; } // 置入棋子
    const std::wstring toString() const;

private:
    int row_; //低四位
    int col_; //高四位
    std::shared_ptr<PieceSpace::Piece> piece_;
};

extern const std::vector<std::shared_ptr<Seat>> creatSeats();

extern const wchar_t getChar(const PieceColor color, const int index);
extern const std::wstring getPreChars(const int length);
extern const PieceColor getColor(const wchar_t numZh);
extern const wchar_t getIndexChar(const int length, const bool isBottom, const int index);
extern const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isForward);
extern const wchar_t getNumChar(const PieceColor color, const int num);
extern const wchar_t getColChar(const PieceColor color, bool isBottom, const int col);
extern const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar);
extern const int getMovDir(const bool isBottom, const wchar_t movChar);
extern const int getNum(const PieceColor color, const wchar_t numChar);
extern const int getCol(bool isBottom, const int num);

extern const std::vector<std::pair<int,int>>& getKingRowCols(bool isBottom);
extern const std::vector<std::pair<int,int>>& getAdviSorRowCols(bool isBottom);
extern const std::vector<std::pair<int,int>>& getBishopRowCols(bool isBottom);
extern const std::vector<std::pair<int,int>>& getPawnRowCols(bool isBottom);


extern const int RowNum;
extern const int RowLowIndex;
extern const int RowLowMidIndex;
extern const int RowLowUpIndex;
extern const int RowUpLowIndex;
extern const int RowUpMidIndex;
extern const int RowUpIndex;
extern const int ColNum;
extern const int ColLowIndex;
extern const int ColMidLowIndex;
extern const int ColMidUpIndex;
extern const int ColUpIndex;
extern const std::wstring movChars;
extern const std::map<PieceColor, std::wstring> numChars;

}

/*
class Seats {
public:
    Seats();

    const Seat& getSeat(int col, int row) const { return seats[col][row]; }
    const std::vector<const Seat*> allSeats() const;
    const std::vector<const Seat*> kingSeats(BoardSide bs) const;
    const std::vector<const Seat*> advisorSeats(BoardSide bs) const;
    const std::vector<const Seat*> bishopSeats(BoardSide bs) const;
    const std::vector<const Seat*> pawnSeats(BoardSide bs) const;

    const bool isSameCol(const Seat& aseat, const Seat& bseat) const { return aseat.col_ == bseat.col_; }
    const Seat& rotateSeat(const Seat& seat) const { return seats[MaxCol - seat.col_][MaxRow - seat.row_]; }
    const Seat& symmetrySeat(const Seat& seat) const { return seats[MaxCol - seat.col_][seat.row_]; }
    const std::vector<const Seat*> getSameColSeats(const Seat& aseat, const Seat& bseat) const;

    // 位置行走函数
    const std::vector<const Seat*> getKingMoveSeats(const Seat& seat) const;
    const std::vector<const Seat*> getAdvisorMoveSeats(const Seat& seat) const;
    // 获取移动、象心行列值
    const std::vector<pair<const Seat*, const Seat*>> getBishopMove_CenSeats(const Seat& seat) const;
    // 获取移动、马腿行列值
    const std::vector<pair<const Seat*, const Seat*>> getKnightMove_LegSeats(const Seat& seat) const;
    // 车炮可走的四个方向位置
    const std::vector<std::vector<const Seat*>> getRookCannonMoveSeat_Lines(const Seat& seat) const;
    const std::vector<const Seat*> getPawnMoveSeats(const bool isBottomSide, const Seat& seat) const;
    // '多兵排序'
    const std::vector<const Seat*> sortPawnSeats(const bool isBottomSide, std::vector<Seat> pawnSeats) const;

private:
    std::vector<std::vector<Seat>> seats;
    // 棋盘数值常量
    const int ColNum{ 0x09 };
    const int RowNum{ 0x0A };
    const int MinCol{ 0x00 };
    const int MaxCol{ 0x08 };
    const int MinRow{ 0x00 };
    const int MaxRow{ 0x09 };
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
const std::vector<int> allSeats{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
    72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89
};
const std::vector<int> bottomKingSeats{ 21, 22, 23, 12, 13, 14, 3, 4, 5 };
const std::vector<int> topKingSeats{ 84, 85, 86, 75, 76, 77, 66, 67, 68 };
const std::vector<int> bottomAdvisorSeats{ 21, 23, 13, 3, 5 };
const std::vector<int> topAdvisorSeats{ 84, 86, 76, 66, 68 };
const std::vector<int> bottomBishopSeats{ 2, 6, 18, 22, 26, 38, 42 };
const std::vector<int> topBishopSeats{ 47, 51, 63, 67, 71, 83, 87 };
const std::vector<int> bottomPawnSeats{
    27, 29, 31, 33, 35, 36, 38, 40, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89
};
const std::vector<int> topPawnSeats{
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
const std::vector<int> getSameColSeats(const int seat, const int othseat);

// 位置行走函数
const std::vector<int> getKingMoveSeats(const int seat);
const std::vector<int> getAdvisorMoveSeats(const int seat);
// 获取移动、象心行列值
const std::vector<pair<int, int>> getBishopMove_CenSeats(const int seat);
// 获取移动、马腿行列值
const std::vector<pair<int, int>> getKnightMove_LegSeats(const int seat);
// 车炮可走的四个方向位置
const std::vector<std::vector<int>> getRookCannonMoveSeat_Lines(const int seat);
const std::vector<int> getPawnMoveSeats(const bool isBottomSide, const int seat);
// '多兵排序'
const std::vector<int> sortPawnSeats(const bool isBottomSide, std::vector<int> pawnSeats);

// 测试函数
const std::wstring test();

} // namespace Board_base

*/

#endif