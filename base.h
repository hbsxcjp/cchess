#ifndef BASE_H
#define BASE_H

#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

// 棋子站队
enum class Side {
    red,
    black
};
// 全部棋子ch值与中文名称映射
const map<char, wchar_t> Char_Names{
    { 'K', L'帅' }, { 'A', L'仕' }, { 'B', L'相' }, { 'N', L'马' },
    { 'R', L'车' }, { 'C', L'炮' }, { 'P', L'兵' }, { 'k', L'将' },
    { 'a', L'士' }, { 'b', L'象' }, { 'n', L'马' }, { 'r', L'车' },
    { 'c', L'炮' }, { 'p', L'卒' }, { '_', L'\x0000' }
};
// 相关特征棋子名字串
const map<wstring, wstring> Feature_Names{ { L"king", L"帅将" },
    { L"stronge", L"马车炮兵卒" },
    { L"allpiece", L"帅仕相马车炮兵将士象卒" },
    { L"line_move", L"帅车炮兵将卒" },
    { L"adv_bis", L"仕相士象" },
    { L"pawn", L"兵卒" } };

// 棋盘相关常量
const int ColNum{ 9 };
const int RowNum{ 10 };
const int MinCol{ 0 };
const int MaxCol{ 8 };
const wstring FEN{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1" };
const wstring ColChars{ L"abcdefghi" };
const map<Side, wstring> Side_ChNums{
    { Side::red, L"一二三四五六七八九" },
    { Side::black, L"１２３４５６７８９" }
};
const map<wchar_t, int> ChNum_Indexs{
    { L'一', 0 }, { L'二', 1 }, { L'三', 2 }, { L'四', 3 }, { L'五', 4 },
    { L'前', 0 }, { L'中', 1 }, { L'后', 1 }
};
const map<wchar_t, int> Direction_Nums{
    { L'进', 1 }, { L'退', -1 }, { L'平', 0 }
};

// 文本空棋盘
const wstring TextBlankBoard{
    L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
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
    "┗━┷━┷━┷━┷━┷━┷━┷━┛\n"
}; // 边框粗线

// 棋局信息类
class Info {
public:
    Info(string pgn);
    Info();

    wstring toString();
    Info* setFromPgn(wstring);

private:
    map<wstring, wstring> info;
};

// 棋子类
class Piece {
public:
    explicit Piece(char);

    const Side side();
    const char ch();
    const wchar_t chName();
    const bool isKing();
    const bool isStronge();

    vector<int> getCanSeats();
    // 筛除本方棋子占用的目标位置
    vector<int> filterColorSeats(vector<int>, wstring);

private:
    Side sd;
    char c;
};

#endif