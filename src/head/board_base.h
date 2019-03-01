#ifndef BOARD_BASE_H
#define BOARD_BASE_H

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
using namespace std;


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
inline int getRow(const int seat) { return seat / 9; }
inline int getCol(const int seat) { return seat % 9; }
inline int getSeat(const int row, const int col) { return row * 9 + col; }
inline int rotateSeat(const int seat) { return 89 - seat; }
inline int symmetrySeat(const int seat) { return (getRow(seat) + 1) * 9 - seat % 9 - 1; }
inline bool isSameCol(const int seat, const int othseat) { return getCol(seat) == getCol(othseat); }
vector<int> getSameColSeats(const int seat, const int othseat);

// 位置行走函数
vector<int> getKingMoveSeats(const int seat);
vector<int> getAdvisorMoveSeats(const int seat);
// 获取移动、象心行列值
vector<pair<int, int>> getBishopMove_CenSeats(const int seat);
// 获取移动、马腿行列值
vector<pair<int, int>> getKnightMove_LegSeats(const int seat);
// 车炮可走的四个方向位置
vector<vector<int>> getRookCannonMoveSeat_Lines(const int seat);
vector<int> getPawnMoveSeats(const bool isBottomSide, const int seat);
// '多兵排序'
vector<int> sortPawnSeats(const bool isBottomSide, vector<int> pawnSeats);

// 测试函数
const wstring test();

} // namespace Board_base

#endif