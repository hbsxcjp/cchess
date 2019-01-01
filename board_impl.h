#ifndef BOARD_IMPL_H
#define BOARD_IMPL_H

#include "board.h"


// 棋盘位置相关组合: 类内声明，类外定义
const vector<int> Board::allSeats{
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
    72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89};
const vector<int> Board::bottomKingSeats{21, 22, 23, 12, 13, 14, 3, 4, 5};
const vector<int> Board::topKingSeats{84, 85, 86, 75, 76, 77, 66, 67, 68};
const vector<int> Board::bottomAdvisorSeats{21, 23, 13, 3, 5};
const vector<int> Board::topAdvisorSeats{84, 86, 76, 66, 68};
const vector<int> Board::bottomBishopSeats{2, 6, 18, 22, 26, 38, 42};
const vector<int> Board::topBishopSeats{47, 51, 63, 67, 71, 83, 87};
const vector<int> Board::bottomPawnSeats{
    27, 29, 31, 33, 35, 36, 38, 40, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89};
const vector<int> Board::topPawnSeats{
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 47, 49, 51, 53, 54, 56, 58, 60, 62};

// 棋盘相关字符串: 类内声明，类外定义
const map<PieceColor, wstring> Board::Side_ChNums{
    {PieceColor::red, L"一二三四五六七八九"},
    {PieceColor::black, L"１２３４５６７８９"}};
const map<wchar_t, int> Board::ChNum_Indexs{{L'一', 0}, {L'二', 1}, {L'三', 2},
                                            {L'四', 3}, {L'五', 4}, {L'前', 0},
                                            {L'中', 1}, {L'后', 1}};
const map<wchar_t, int> Board::Direction_Nums{
    {L'进', 1}, {L'退', -1}, {L'平', 0}};

const wstring Board::FEN{
    L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1"};
const wstring Board::ColChars{L"abcdefghi"};
// 文本空棋盘
const wstring Board::TextBlankBoard{L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
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
                                    "┗━┷━┷━┷━┷━┷━┷━┷━┛\n"}; // 边框粗线


#endif