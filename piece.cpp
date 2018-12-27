#include "base.h"

Piece::Piece(char ch)
    : sd{ ch >= 'a' ? Side::black : Side::red }
    , c{ ch }
{
}

const Side Piece::side()
{
    return sd;
}

const char Piece::ch()
{
    return c;
}

const wchar_t Piece::chName()
{
    return (*Char_Names.find(c)).second;
}

const bool Piece::isKing()
{
    wstring fnames{ (*Feature_Names.find(L"king")).second };
    return fnames.find(chName()) != wstring::npos;
}

const bool Piece::isStronge()
{
    wstring fnames{ (*Feature_Names.find(L"stronge")).second };
    return fnames.find(chName()) != wstring::npos;
}

// 棋子的全部活动位置(默认：车马炮的活动范围)
vector<int> Piece::getCanSeats()
{
    //return Board.allSeats();
}

// 筛除本方棋子占用的目标位置
vector<int> Piece::filterColorSeats(vector<int> vi, wstring ws)
{ //moveSeats, board
    //return moveSeats.filter(s => board.getColor(s) != this->_color);
}
