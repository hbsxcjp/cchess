#ifndef PIECE_H
#define PIECE_H

#include <map>
#include <memory>
#include <string>
using namespace std;

class Board;
enum class PieceColor { RED,
    BLACK,
    BLANK };

// 棋子类
class Piece {

public:
    explicit Piece(const wchar_t ch);

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const { return name_; }
    const PieceColor color() const { return color_; }

    static shared_ptr<Piece> nullPiece;
    const wstring toString() const;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
};

// 棋子辅助类
class PieceAide {
public:
    static const wchar_t getNullChar() { return __nullChar; }
    static const wchar_t getName(const wchar_t ch) { return __charNames.at(ch); }
    static const PieceColor getColor(const wchar_t ch) { return ch == L'_' ? PieceColor::BLANK : (islower(ch) ? PieceColor::BLACK : PieceColor::RED); }
    static const PieceColor getOthColor(const PieceColor color) { return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED; }

    static const bool isKing(const wchar_t name) { return static_cast<wstring>(L"将帅").find(name) != wstring::npos; }
    static const bool isAdvBish(const wchar_t name) { return static_cast<wstring>(L"仕士相象").find(name) != wstring::npos; }
    static const bool isStronge(const wchar_t name) { return static_cast<wstring>(L"马车炮兵卒").find(name) != wstring::npos; }
    static const bool isLineMove(const wchar_t name) { return static_cast<wstring>(L"将帅车炮兵卒").find(name) != wstring::npos; }
    static const bool isPawn(const wchar_t name) { return static_cast<wstring>(L"兵卒").find(name) != wstring::npos; }
    static const bool isPieceName(const wchar_t name) { return static_cast<wstring>(L"帅将仕士相象马车炮兵卒").find(name) != wstring::npos; }

    static const wchar_t getIndexChar(const int length, const int index) { return wstring(length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五"))[index]; }
    static const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isForward) { return isSameRow ? L'平' : (isBottom == isForward ? L'进' : L'退'); }
    static const wchar_t getColChar(const PieceColor color, bool isBottom, const int col) { return __getChar(color, isBottom ? ColNum - col - 1 : col); };
    static const wchar_t getNumChar(const PieceColor color, const int num) { return __getChar(color, num - 1); };

    static const PieceColor getColor_wch(const wchar_t wch) { return __numChars.at(PieceColor::RED).find(wch) != wstring::npos ? PieceColor::RED : PieceColor::BLACK; };

    static const wchar_t getNum(const PieceColor color, const wchar_t numChar) { return static_cast<int>(__numChars.at(color).find(numChar)) + 1; }
    static const wchar_t getCol(bool isBottom, const int num) { return isBottom ? ColNum - num : num - 1; }
    static const int getMovDir(const bool isBottom, const wchar_t movChar) { return __movIndex.at(movChar) * (isBottom ? 1 : -1); }

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    //int movDir{ __movIndex.at(zhStr[2]) * (isBottom ? 1 : -1) }, num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    //int movDir{ __wchIndex(zhStr[2]) * (isBottom ? 1 : -1) },

    static const int RowNum{ 10 };
    static const int ColNum{ 9 };

private:
    //auto __getNum = [&](const wchar_t wch) { return static_cast<int>(__numChars.at(color).find(wch)) + 1; };
    //auto __getCol = [&](const int num) { return isBottom ? ColNum - num : num - 1; };

    static const wchar_t __getChar(const PieceColor color, const int index) { return __numChars.at(color)[index]; };
    static const wchar_t __nullChar{ L'_' };
    static const map<wchar_t, wchar_t> __charNames;
    static const map<PieceColor, wstring> __numChars;
    static const map<wchar_t, int> __movIndex;
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