#ifndef BASE_H
#define BASE_H

#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

// 棋子相关常量
/*
  const BLACK = 'black';
  const RED = 'red';
*/
enum class side {
    red,
    black
};

// 全部棋子ch值与中文名称映射字典
/*
const CharNames = {
    'K': '帅', 'A': '仕', 'B': '相', 'N': '马',
    'R': '车', 'C': '炮', 'P': '兵',
    'k': '将', 'a': '士', 'b': '象', 'n': '马',
    'r': '车', 'c': '炮', 'p': '卒', '_': ''
};
*/
const map<char, wchar_t> CharNames{
    { 'K', L'帅' }, { 'A', L'仕' }, { 'B', L'相' }, { 'N', L'马' },
    { 'R', L'车' }, { 'C', L'炮' }, { 'P', L'兵' }, { 'k', L'将' },
    { 'a', L'士' }, { 'b', L'象' }, { 'n', L'马' }, { 'r', L'车' },
    { 'c', L'炮' }, { 'p', L'卒' }, { '_', L'\x0000' }
};
void print_CharNames()
{
    for (const auto m : CharNames) {
        cout << m.first << ": " << m.second << endl;
    }
}

/*
const KingNames = new Set('帅将');
const StrongeNames = new Set('马车炮兵卒');
const PieceNames = new Set('帅仕相马车炮兵将士象卒');
const LineMovePieceNames = new Set('帅车炮兵将卒');
const AdvisorBishopNames = new Set('仕相士象');
const PawnNames = new Set('兵卒');
*/
const map<string, string> Feature_PieceNames{ { "king", "帅将" },
    { "stronge", "马车炮兵卒" },
    { "allpiece",
        "帅仕相马车炮兵将士象卒" },
    { "line_move", "帅车炮兵将卒" },
    { "adv_bis", "仕相士象" },
    { "pawn", "兵卒" } };
void print_Feature_PieceNames()
{
    for (const auto m : Feature_PieceNames) {
        cout << m.first << ": " << m.second << endl;
    }
}

// 棋盘相关常量
/*
const NumCols = 9;
const NumRows = 10;
const MinColNo = 0;
const maxColNo = 8;
const FEN = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0
1'; const ColChars = 'abcdefghi'; const Num_Chinese = { 'red':
'一二三四五六七八九', 'black': '１２３４５６７８９'
};
const Chinese_Index = {
    '一':0, '二':1, '三':2, '四':3, '五':4,
    '前': 0, '中': 1, '后': 1
};
const Direction_Num = {
    '进': 1, '退': -1, '平': 0
};
const BlankBoard = `
┏━┯━┯━┯━┯━┯━┯━┯━┓
┃　│　│　│╲│╱│　│　│　┃
┠─┼─┼─┼─╳─┼─┼─┼─┨
┃　│　│　│╱│╲│　│　│　┃
┠─╬─┼─┼─┼─┼─┼─╬─┨
┃　│　│　│　│　│　│　│　┃
┠─┼─╬─┼─╬─┼─╬─┼─┨
┃　│　│　│　│　│　│　│　┃
┠─┴─┴─┴─┴─┴─┴─┴─┨
┃　　　　　　　　　　　　　　　┃
┠─┬─┬─┬─┬─┬─┬─┬─┨
┃　│　│　│　│　│　│　│　┃
┠─┼─╬─┼─╬─┼─╬─┼─┨
┃　│　│　│　│　│　│　│　┃
┠─╬─┼─┼─┼─┼─┼─╬─┨
┃　│　│　│╲│╱│　│　│　┃
┠─┼─┼─┼─╳─┼─┼─┼─┨
┃　│　│　│╱│╲│　│　│　┃
┗━┷━┷━┷━┷━┷━┷━┷━┛
`
// 边框粗线
*/

#endif