#ifndef PIECE_H
#define PIECE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

class Board;
enum class PieceColor { RED,
    BLACK,
    BLANK };

// 棋子类
class Piece {

public:
    explicit Piece(const wchar_t ch, const wchar_t name, const PieceColor color)
        : ch_{ ch }
        , name_{ name }
        , color_{ color }
    {
    }

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const { return name_; }
    const PieceColor color() const { return color_; }
    const wstring toString() const;

    static const PieceColor getOthColor(const PieceColor color) { return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED; }
    static const bool isKing(const wchar_t name) { return __nameChars.substr(0, 2).find(name) != wstring::npos; }
    static const bool isAdvBish(const wchar_t name) { return __nameChars.substr(2, 4).find(name) != wstring::npos; }
    static const bool isStronge(const wchar_t name) { return __nameChars.substr(6, 5).find(name) != wstring::npos; }
    static const bool isLineMove(const wchar_t name) { return isKing(name) || __nameChars.substr(7, 4).find(name) != wstring::npos; }
    static const bool isPawn(const wchar_t name) { return __nameChars.substr(__nameChars.size() - 2, 2).find(name) != wstring::npos; }
    static const bool isPieceName(const wchar_t name) { return __nameChars.find(name) != wstring::npos; }
    static const vector<shared_ptr<Piece>> __creatPieces();

    static const wchar_t nullChar;
    static shared_ptr<Piece> nullPiece;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;

    static const wstring __nameChars;
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