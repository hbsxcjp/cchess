#ifndef PIECE_H
#define PIECE_H

//#include <memory>
#include <map>
#include <string>
//#include <vector>
using namespace std;

class Board;
enum class PieceColor {
    RED,
    BLACK,
    BLANK
};

static map<wchar_t, wchar_t> charNames{
    { L'K', L'帅' }, { L'k', L'将' }, { L'A', L'仕' }, { L'a', L'士' },
    { L'B', L'相' }, { L'b', L'象' }, { L'N', L'马' }, { L'n', L'马' },
    { L'R', L'车' }, { L'r', L'车' }, { L'C', L'炮' }, { L'c', L'炮' },
    { L'P', L'兵' }, { L'p', L'卒' }, { L'_', L'　' }
};

// 棋子类
class Piece {

public:
    explicit Piece(const wchar_t ch)
        : ch_{ ch }
        , name_{ charNames[ch] }
        , color_{ ch == L'_' ? PieceColor::BLANK : (islower(ch) ? PieceColor::BLACK : PieceColor::RED) }
    {
    }

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const { return name_; }
    const PieceColor color() const { return color_; }

    virtual const bool isKing() const { return tolower(ch_) == L'k'; }
    virtual const bool isPawn() const { return tolower(ch_) == L'p'; }
    virtual const bool isStronge() const { return static_cast<wstring>(L"nrcp").find(tolower(ch_)) != wstring::npos; }
    virtual const bool isLineMove() const { return static_cast<wstring>(L"krcp").find(tolower(ch_)) != wstring::npos; }

    static PieceColor getOthColor(const PieceColor color);
    const wstring toString() const;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
};

/*
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class Board;
enum class PieceColor {
    BLANK,
    RED,
    BLACK
};

// 棋子类
class Piece {

public:
    Piece(const wchar_t _ch);

    const int seat() const { return st; }
    const wchar_t ch_() const { return ch; }
    const PieceColor color() const { return clr; }
    const wchar_t chName() const { return name; }
    virtual const bool isBlank() const { return false; }
    virtual const bool isKing() const { return false; }
    virtual const bool isStronge() const { return false; }

    void setSeat(const int seat) { st = seat; }
    // 棋子可置放的全部位置
    virtual const vector<int> getSeats(const PieceColor bottomColor) const;
    // 棋子可移动到的全部位置, 筛除本方棋子所占位置
    virtual const vector<int> filterMoveSeats(const Board& board);
    // '获取棋子可走的位置, 不能被将军'
    const vector<int> getCanMoveSeats(Board& board);

    virtual const wstring toString() const;
    virtual ~Piece() = default;
    static const wchar_t nullChar;

protected:
    // 棋子可移动到的全部位置
    virtual const vector<int> __moveSeats(const Board& board) const;
    // 筛除棋子行棋规则不允许的位置
    virtual const vector<int> __filterMove_obstruct(const Board& board,
        const vector<pair<int, int>>& seat_obss) const;

private:
    int st; // 在棋盘中的位置序号
    const wchar_t ch;
    const wchar_t name;
    const PieceColor clr;
    static map<wchar_t, wchar_t> chNames;
};

class King : public Piece {
public:
    using Piece::Piece;
    const bool isKing() const { return true; }
    const vector<int> getSeats(const PieceColor bottomColor) const;

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Advisor : public Piece {
public:
    using Piece::Piece;
    const vector<int> getSeats(const PieceColor bottomColor) const;

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Bishop : public Piece {
public:
    using Piece::Piece;
    const vector<int> getSeats(const PieceColor bottomColor) const;

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Knight : public Piece {
public:
    using Piece::Piece;
    const bool isStronge() const { return true; };

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Rook : public Piece {
public:
    using Piece::Piece;
    const bool isStronge() const { return true; };

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Cannon : public Piece {
public:
    using Piece::Piece;
    const bool isStronge() const { return true; };

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class Pawn : public Piece {
public:
    using Piece::Piece;
    const bool isStronge() const { return true; }
    const vector<int> getSeats(const PieceColor bottomColor) const;

private:
    const vector<int> __moveSeats(const Board& board) const;
};

class NullPie : public Piece {
public:
    using Piece::Piece;
    const bool isBlank() const { return true; }
    //void setSeat(int seat) {} // 加此函数，反而出现不明问题。
};
*/

#endif