#include "piece.h"

#include <iostream>
using std::boolalpha;

#include <iomanip>
using std::setw;

//#include <sstream>
using std::wstringstream;

#include <vector>
using std::vector;


const wstring Piece::toString() {
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(5) << index() << setw(4) << static_cast<int>(color())
        << setw(5) << wchar() << setw(6) << chName() << setw(9)
        << isBlank() << setw(8) << isKing() << setw(8) << isStronge() << L'\n';
    return wss.str();
}

 // 棋子可置放的全部位置
vector<int> Piece::getSeats() {
    // return Board.allSeats();
}

 // 棋子可移动到的全部位置 // 筛除本方棋子占用的目标位置
vector<int> Piece::getCanMoveSeats() 
{ // moveSeats, board
                            // return moveSeats.filter(s =>
                            // board.getColor(s) != this->_color);
}

// 相关特征棋子名字串: 类内声明，类外定义
const wstring Piece::kingNames{L"帅将"};
const wstring Piece::pawnNames{L"兵卒"};
const wstring Piece::advbisNames{L"仕相士象"};
const wstring Piece::strongeNames{L"马车炮兵卒"};
const wstring Piece::lineNames{L"帅车炮兵将卒"};
const wstring Piece::allNames{L"帅仕相马车炮兵将士象卒"};

// 类外初始化类内静态const成员
int Piece::curIndex{-1};
const int Piece::nullSeat{-1};
const Piece Piece::nullPie{Piece(L'_')};


// 一副棋子类
Pieces::Pieces() {
    wstring pieceChars{L"KAABBNNRRCCPPPPPkaabbnnrrccppppp"};
    for (auto wc : pieceChars) 
        pies.push_back(Piece(wc));
}

vector<Piece *> Pieces::getPiePtrs() {
    vector<Piece *> res{};
    for (int i = 0; i != 32; ++i)
        res.push_back(&pies[i]);
    return res;
}

wstring Pieces::toString() {
    wstring ws{L"index color wchar chName isBlank isKing isStronge\n"};
    for (auto pie : pies) 
        ws += pie.toString();
    return ws + (Piece {Piece::nullPie}).toString();  // 关注空棋子的特性! 将const限定符取消  
}

// Pieces seatPieces(vector<int, wchar_t> seatChars)    {    }

extern wstring test_piece() {
    Pieces pieces = Pieces();
    wstringstream wss{};
    wss << L"test piece.h\n-----------------------------------------------------\nPieces::toString:\n";
    wss << pieces.toString();

    wss << L"Piece::getKingPie\\Piece::chName：\n红棋->" << pieces.getKingPie(PieceColor::red).chName()
        << L' ' << L"黑棋->" << pieces.getKingPie(PieceColor::black).chName();

    wss << L"\nPieces::getOthSidePiece：\n";
    for (auto piePtr : pieces.getPiePtrs())
        wss << piePtr->chName() << L"->" << pieces.getOthPie(*piePtr).chName()
            << L' ';

    wss << L"\n特征棋子串：\n"
        << Piece::kingNames << L' ' << Piece::pawnNames << L' '
        << Piece::advbisNames << L' ' << Piece::strongeNames << L' '
        << Piece::lineNames << L' ' << Piece::allNames << L"\n\n";

    return wss.str();
}

