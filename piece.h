#ifndef PIECE_H
#define PIECE_H

#include <iostream>

#include <sstream>
using std::wstringstream;

#include <string>
using std::wstring;

#include <vector>
using std::vector;

// 棋子站队
enum class PieceColor {
    blank,
    red,
    black
};

// 棋子类
class Piece {

public:
    explicit Piece(wchar_t aWchar)
        : sd{ islower(aWchar) ? PieceColor::black : (isupper(aWchar) ? PieceColor::red : PieceColor::blank) }
        , ch{ aWchar }
        , id{ Piece::curIndex++ }
    {
    }

    wstring toString()
    {
        wstringstream wss{};
        wss << L"  " << index() << L"    " << static_cast<int>(side())
            << L"    " << wchar() << L"     " << chName()
            << L"     " << isKing() << L"        " << isStronge() << L'\n';
        return wss.str();
    }

    PieceColor const side()
    {
        return sd;
    }

    wchar_t const wchar()
    {
        return ch;
    }

    int const index()
    {
        return id;
    }

    wchar_t const chName()
    {
        switch (ch) {
        case L'K':
            return L'帅';
        case L'A':
            return L'仕';
        case L'B':
            return L'相';
        case L'N':
            return L'马';
        case L'R':
            return L'车';
        case L'C':
            return L'炮';
        case L'P':
            return L'兵';
        case L'k':
            return L'将';
        case L'a':
            return L'士';
        case L'b':
            return L'象';
        case L'n':
            return L'马';
        case L'r':
            return L'车';
        case L'c':
            return L'炮';
        case L'p':
            return L'卒';
        default:
            return L'\x0000';
        }
    }

    // 空棋子
    bool const isBlank()
    {
        return sd == PieceColor::black;
    }

    bool const isKing()
    {
        return ch == L'K' || ch == L'k';
    }

    bool const isStronge()
    {
        return ch == L'N' || ch == L'n' || ch == L'R' || ch == L'r' || ch == L'C' || ch == L'c' || ch == L'P' || ch == L'p';
    }

    // 棋子的全部活动位置(默认：车马炮的活动范围)
    vector<int> getCanSeats()
    {
        //return Board.allSeats();
    }

    // 筛除本方棋子占用的目标位置
    vector<int> filterColorSeats(vector<int>, wstring)
    { //moveSeats, board
        //return moveSeats.filter(s => board.getColor(s) != this->_color);
    }

    static int curIndex;
    static const Piece nullPie; // 空棋子

private:
    PieceColor sd;
    wchar_t ch;
    int id; // 在一副棋子中的序号
};

int Piece::curIndex{ -1 };
const Piece Piece::nullPie{ Piece(L'_') };

// 一副棋子类
class Pieces {
public:
    Pieces()
    {
        Piece::curIndex = 0;
        wstring pieceChars{ L"KAABBNNRRCCPPPPPkaabbnnrrccppppp" };
        for (auto wc : pieceChars)
            pies.push_back(Piece(wc));
    }

    wstring toString()
    {
        wstringstream wss{};
        wss << L"棋子信息：\nindex side wchar chName isKing isStronge\n";
        vector<Piece> mpies = pies;
        mpies.push_back(Piece::nullPie); // 关注空棋子的特性!
        for (auto pie : mpies) {
            wss << pie.toString();
        }
        return wss.str();
    }

    vector<Piece> getPies()
    {
        return pies;
    }

    Piece getKing(PieceColor side)
    {
        return pies[side == PieceColor::red ? 0 : 16];
    }

    Piece getOthSidePiece(Piece pie)
    {
        return pies[(pie.index() + 16) % 32];
    }

    //Pieces seatPieces(vector<int, wchar_t> seatChars)    {    }

    // 相关特征棋子名字串
    static const wstring kingNames;
    static const wstring pawnNames;
    static const wstring advbisNames;
    static const wstring strongeNames;
    static const wstring lineNames;
    static const wstring allNames;
    // static const pieceTypes;

private:
    vector<Piece> pies{};
};

// 相关特征棋子名字串: 类内声明，类外定义
const wstring Pieces::kingNames{ L"帅将" };
const wstring Pieces::pawnNames{ L"兵卒" };
const wstring Pieces::advbisNames{ L"仕相士象" };
const wstring Pieces::strongeNames{ L"马车炮兵卒" };
const wstring Pieces::lineNames{ L"帅车炮兵将卒" };
const wstring Pieces::allNames{ L"帅仕相马车炮兵将士象卒" };

#endif