#ifndef BOARD_BASE_H
#define BOARD_BASE_H

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// 棋子站队
enum class PieceColor {
    blank,
    red,
    black
};

enum class RecFormat {
    XQF,
    ICCS,
    ZH,
    CC,
    BIN,
    JSON
};

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

const wstring FEN{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
const wstring FENPro{ FEN + L" r - - 0 1" };
const wstring ColChars{ L"abcdefghi" };
// 文本空棋盘
const wstring TextBlankBoard{ L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
                              "┃　│　│　│╲│╱│　│　│　┃\n"
                              "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                              "┃　│　│　│╱│╲│　│　│　┃\n"
                              "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                              "┃　│　│　│　│　│　│　│　┃\n"
                              "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                              "┃　│　│　│　│　│　│　│　┃\n"
                              "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                              "┃　　　　　　　　　　　　　　　┃\n"
                              "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                              "┃　│　│　│　│　│　│　│　┃\n"
                              "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                              "┃　│　│　│　│　│　│　│　┃\n"
                              "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                              "┃　│　│　│╲│╱│　│　│　┃\n"
                              "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                              "┃　│　│　│╱│╲│　│　│　┃\n"
                              "┗━┷━┷━┷━┷━┷━┷━┷━┛\n" }; // 边框粗线

// 字符获取函数
inline wstring getNumChars(PieceColor color)
{
    return color == PieceColor::red ? L"一二三四五六七八九" : L"１２３４５６７８９";
}
inline wchar_t getNumChar(PieceColor color, int col) { return getNumChars(color)[col]; }

inline int getIndex(wchar_t ch)
{
    static map<wchar_t, int> ChNum_Indexs{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
        { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
        { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
    return ChNum_Indexs[ch];
}
/*
inline int getNum(wchar_t ch)
{
    static map<wchar_t, int>  ChNum_Indexs{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
        { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
        { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
    return ChNum_Indexs[ch];
}*/

// 函数
inline int getRow(int seat) { return seat / 9; }
inline int getCol(int seat) { return seat % 9; }
inline int getSeat(int row, int col) { return row * 9 + col; }
inline int rotateSeat(int seat) { return 89 - seat; }
inline int symmetrySeat(int seat) { return (getRow(seat) + 1) * 9 - seat % 9 - 1; }
inline bool isSameCol(int seat, int othseat) { return getCol(seat) == getCol(othseat); }
inline bool find_char(wstring ws, wchar_t ch) { return ws.find(ch) != wstring::npos; }
inline int find_index(vector<int> seats, int seat)
{
    int size = seats.size();
    for (int i = 0; i != size; ++i)
        if (seat == seats[i])
            return i;
    return 0;
}
vector<int> getSameColSeats(int seat, int othseat);

// 位置行走函数
vector<int> getKingMoveSeats(int seat);
vector<int> getAdvisorMoveSeats(int seat);
// 获取移动、象心行列值
vector<pair<int, int>> getBishopMove_CenSeats(int seat);
// 获取移动、马腿行列值
vector<pair<int, int>> getKnightMove_LegSeats(int seat);
// 车炮可走的四个方向位置
vector<vector<int>> getRookCannonMoveSeat_Lines(int seat);
vector<int> getPawnMoveSeats(bool isBottomSide, int seat);
// '多兵排序'
vector<int> sortPawnSeats(bool isBottomSide, vector<int> pawnSeats);

inline int __subbyte(int a, int b) { return (256 + a - b) % 256; }
inline string trim(string& str);
inline wstring wtrim(wstring& str);
wstring print_vector_int(vector<int> vi);
std::string ws2s(const std::wstring& ws);
std::wstring s2ws(const std::string& s);
wstring readTxt(string fileName);
void writeTxt(string fileName, wstring ws);
void getFiles(string path, vector<string>& files);
int copyFile(const char* SourceFile, const char* NewFile);
string getExt(string filename);

// 测试函数
wstring test();
} // namespace Board_base

#endif