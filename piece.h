#ifndef PIECE_H
#define PIECE_H

#include <string>
using std::wstring;

#include <vector>
using std::vector;

#include <sstream>
using std::wstringstream;

#include <iomanip>
using std::boolalpha;
using std::setw;

// 棋子站队
enum class PieceColor { blank, red, black };

// 棋子类
class Piece {

  public:
    explicit Piece(wchar_t aWchar)
        : seat(nullSeat), id{curIndex++}, ch{aWchar},
          clr{islower(aWchar)
                  ? PieceColor::black
                  : (isupper(aWchar) ? PieceColor::red : PieceColor::blank)} {}

    int const index() { return id; }
    PieceColor const color() { return clr; }
    wchar_t const wchar() { return ch; }
    wchar_t const chName() {
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
    bool const isBlank() { return id == -1; }
    bool const isKing() { return ch == L'K' || ch == L'k'; }
    bool const isStronge() {
        return ch == L'N' || ch == L'n' || ch == L'R' || ch == L'r' ||
               ch == L'C' || ch == L'c' || ch == L'P' || ch == L'p';
    }

    const wstring toString();
    // 棋子可置放的全部位置
    vector<int> getSeats(PieceColor bottomColor);
    // 棋子可移动到的全部位置, 筛除本方棋子所占位置
    vector<int> getCanMoveSeats();

    static const int nullSeat;  // 空位置
    static const Piece nullPie; // 空棋子
    int seat;                   // 在棋盘中的位置序号

  private:
    static int curIndex;
    int id; // 在一副棋子中的序号
    wchar_t ch;
    PieceColor clr;
};

// 一副棋子类
class Pieces {
  public:
    Pieces();

    Piece getKingPie(PieceColor color) {
        return pies[color == PieceColor::red ? 0 : 16];
    }
    int getKingSeat(PieceColor color) { return getKingPie(color).seat; }
    Piece getOthPie(Piece pie) { return pies[(pie.index() + 16) % 32]; }

    vector<Piece *> getPiePtrs();

    //成员函数，类内声明，类外定义
    vector<Piece *> getLivePieces();
    vector<Piece *> getLivePieces(PieceColor color);
    vector<int> getNameSeats(PieceColor color, wchar_t name);
    vector<int> getNameColSeats(PieceColor color, wchar_t name, int col);
    vector<Piece *> getEatedPieces();

    wstring toString();
    // Pieces seatPieces(vector<int, wchar_t> seatChars)    {    }
    // static const pieceTypes;

    // 相关特征棋子名字串
    static const wstring kingNames;
    static const wstring pawnNames;
    static const wstring advbisNames;
    static const wstring strongeNames;
    static const wstring lineNames;
    static const wstring allNames;

    vector<Piece> pies{};
};

wstring test_piece();

#endif