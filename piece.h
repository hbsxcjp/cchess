#ifndef PIECE_H
#define PIECE_H

class Board;
#include "board_base.h"

#include <string>
using std::wstring;

#include <vector>
using std::vector;

#include <sstream>
using std::wstringstream;

#include <iomanip>
using std::boolalpha;
using std::setw;

using namespace Board_base;

// 棋子类
class Piece {

  public:
    Piece(wchar_t _char)
        : clr{isalpha(_char)
                  ? (islower(_char) ? PieceColor::black : PieceColor::red)
                  : PieceColor::blank},
          ch{_char}, st{nullSeat}, id{curIndex++} {}

    int const index() { return id; }
    PieceColor const color() { return clr; }
    wchar_t const wchar() { return ch; }
    int seat() { return st; }
    void setSeat(int seat) { st = seat; }

    virtual wchar_t const chName() { return L'\x0000'; }
    virtual bool const isBlank() { return false; }
    virtual bool const isKing() { return false; }
    virtual bool const isStronge() { return false; }

    // 棋子可置放的全部位置
    virtual vector<int> getSeats(PieceColor bottomColor) { return allSeats; }
    // 棋子可移动到的全部位置, 筛除本方棋子所占位置
    virtual vector<int> getFilterMoveSeats(Board *board);
    // '获取棋子可走的位置, 不能被将军'
    vector<int> getCanMoveSeats(Board *board);

    virtual wstring toString();
    virtual ~Piece() = default;

    static int curIndex;
    static const wchar_t nullChar;

  protected:
    PieceColor clr;
    // 棋子可移动到的全部位置
    virtual vector<int> __MoveSeats(Board *board) { return allSeats; }
    // 筛除棋子行棋规则不允许的位置
    virtual vector<int> __filterMove_obstruct(Board *board,
                                            vector<pair<int, int>> move_obs);

  private:
    wchar_t ch;
    int st; // 在棋盘中的位置序号
    int id; // 在一副棋子中的序号
};

class King : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return clr == PieceColor::red ? L'帅' : L'将'; }
    bool const isKing() { return true; }
    vector<int> getSeats(PieceColor bottomColor) {
        return color() == bottomColor ? bottomKingSeats : topKingSeats;
    }
    vector<int> __MoveSeats(Board *board) { return getKingMoveSeats(seat()); }
};

class Advisor : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return clr == PieceColor::red ? L'仕' : L'士'; }
    vector<int> getSeats(PieceColor bottomColor) {
        return color() == bottomColor ? bottomAdvisorSeats : topAdvisorSeats;
    }
    vector<int> __MoveSeats(Board *board) {
        return getAdvisorMoveSeats(seat());
    }
};

class Bishop : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return clr == PieceColor::red ? L'相' : L'象'; }
    vector<int> getSeats(PieceColor bottomColor) {
        return color() == bottomColor ? bottomBishopSeats : topBishopSeats;
    }
    vector<int> __MoveSeats(Board *board) {
        return __filterMove_obstruct(board, getBishopMove_CenSeats(seat()));
    }
};

class Knight : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return L'马'; }
    bool const isStronge() { return true; }
    vector<int> __MoveSeats(Board *board) {
        return __filterMove_obstruct(board, getKnightMove_LegSeats((seat())));
    }
};

class Rook : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return L'车'; }
    bool const isStronge() { return true; }
    vector<int> __MoveSeats(Board *board);
};

class Cannon : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return L'炮'; }
    bool const isStronge() { return true; }
    vector<int> __MoveSeats(Board *board);
};

class Pawn : public Piece {
  public:
    using Piece::Piece;
    wchar_t const chName() { return clr == PieceColor::red ? L'兵' : L'卒'; }
    bool const isStronge() { return true; }
    vector<int> getSeats(PieceColor bottomColor) {
        return color() == bottomColor ? bottomPawnSeats : topPawnSeats;
    }
    vector<int> __MoveSeats(Board *board);
};

class NullPie : public Piece {
  public:
    using Piece::Piece;
    bool const isBlank() { return true; }
    //void setSeat(int seat) {} // 加此函数，反而出现不明问题。
};

// 一副棋子类
class Pieces {
  public:
    Pieces();

    Piece *getKingPie(PieceColor color);
    Piece *getOthPie(Piece *pie);
    //成员函数，类内声明，类外定义
    vector<Piece *> getPies() { return piePtrs; }
    vector<Piece *> getLivePies();
    vector<Piece *> getLivePies(PieceColor color);
    vector<Piece *> getLiveStrongePies(PieceColor color);
    vector<Piece *> getNamePies(PieceColor color, wchar_t name);
    vector<Piece *> getNameColPies(PieceColor color, wchar_t name, int col);
    vector<Piece *> getEatedPies();

    wstring toString();
    wstring test_piece();

    // 相关特征棋子名字串
    static const wstring kingNames;
    static const wstring pawnNames;
    static const wstring advbisNames;
    static const wstring strongeNames;
    static const wstring lineNames;
    static const wstring allNames;
    static Piece *nullPiePtr;

  private:
    static NullPie nullPiece; // 空棋子
    vector<King> kings;
    vector<Advisor> advisors;
    vector<Bishop> bishops;
    vector<Knight> knights;
    vector<Rook> rooks;
    vector<Cannon> cannons;
    vector<Pawn> pawns;
    vector<Piece *> piePtrs;
};

#endif