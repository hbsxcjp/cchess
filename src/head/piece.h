#ifndef PIECE_H
#define PIECE_H

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
    explicit Piece(const wchar_t _char);

    const int index() { return id; }
    const PieceColor color() { return clr; }
    const wchar_t wchar() { return ch; }
    const int seat() { return st; }
    void setSeat(const int seat) { st = seat; }

    virtual const wchar_t chName() { return L'\x0000'; }
    virtual const bool isBlank() { return false; }
    virtual const bool isKing() { return false; }
    virtual const bool isStronge() { return false; }

    // 棋子可置放的全部位置
    virtual const vector<int> getSeats(const PieceColor bottomColor);
    // 棋子可移动到的全部位置, 筛除本方棋子所占位置
    virtual const vector<int> getFilterMoveSeats(Board& board);
    // '获取棋子可走的位置, 不能被将军'
    const vector<int> getCanMoveSeats(Board& board);

    virtual const wstring toString();
    virtual ~Piece() = default;

    static int curIndex;
    static const wchar_t nullChar;

protected:
    const PieceColor clr;
    // 棋子可移动到的全部位置
    virtual const vector<int> __MoveSeats(Board& board);
    // 筛除棋子行棋规则不允许的位置
    virtual const vector<int> __filterMove_obstruct(Board& board,
        const vector<pair<int, int>>& move_obs);

private:
    wchar_t ch;
    int st; // 在棋盘中的位置序号
    int id; // 在一副棋子中的序号
};

class King : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return clr == PieceColor::RED ? L'帅' : L'将'; }
    const bool isKing() { return true; }
    const vector<int> getSeats(const PieceColor bottomColor);
private:
    const vector<int> __MoveSeats(Board& board);
};

class Advisor : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return clr == PieceColor::RED ? L'仕' : L'士'; }
    const vector<int> getSeats(const PieceColor bottomColor);
private:
    const vector<int> __MoveSeats(Board& board);
};

class Bishop : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return clr == PieceColor::RED ? L'相' : L'象'; }
    const vector<int> getSeats(const PieceColor bottomColor);
private:
    const vector<int> __MoveSeats(Board& board);
};

class Knight : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return L'马'; }
    const bool isStronge() { return true; };
private:
    const vector<int> __MoveSeats(Board& board);
};

class Rook : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return L'车'; }
    const bool isStronge() { return true; };
private:
    const vector<int> __MoveSeats(Board& board);
};

class Cannon : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return L'炮'; }
    const bool isStronge() { return true; };
private:
    const vector<int> __MoveSeats(Board& board);
};

class Pawn : public Piece {
public:
    using Piece::Piece;
    const wchar_t chName() { return clr == PieceColor::RED ? L'兵' : L'卒'; }
    const bool isStronge() { return true; }
    const vector<int> getSeats(const PieceColor bottomColor);
private:
    const vector<int> __MoveSeats(Board& board);
};

class NullPie : public Piece {
public:
    using Piece::Piece;
    const bool isBlank() { return true; }
    //void setSeat(int seat) {} // 加此函数，反而出现不明问题。
};

// 一副棋子类
class Pieces {
public:
    Pieces();

    const shared_ptr<Piece> getKingPie(const PieceColor color);
    const shared_ptr<Piece> getOthPie(const shared_ptr<Piece> pie);
    const shared_ptr<Piece> getFreePie(const wchar_t ch);
    //成员函数，类内声明，类外定义
    const vector<shared_ptr<Piece>> getPies() { return piePtrs; }
    const vector<shared_ptr<Piece>> getLivePies();
    const vector<shared_ptr<Piece>> getLivePies(const PieceColor color);
    const vector<shared_ptr<Piece>> getLiveStrongePies(const PieceColor color);
    const vector<shared_ptr<Piece>> getNamePies(const PieceColor color, const wchar_t name);
    const vector<shared_ptr<Piece>> getNameColPies(const PieceColor color, const wchar_t name, int col);
    const vector<shared_ptr<Piece>> getEatedPies();
    const vector<shared_ptr<Piece>> getEatedPies(const PieceColor color);

    void clearSeat();
    const wstring toString();
    const wstring test();

    // 相关特征棋子名字串
    static const wstring kingNames;
    static const wstring pawnNames;
    static const wstring advbisNames;
    static const wstring strongeNames;
    static const wstring lineNames;
    static const wstring allNames;
    static const shared_ptr<Piece> nullPiePtr;

private:
    /*
    vector<King> kings;
    vector<Advisor> advisors;
    vector<Bishop> bishops;
    vector<Knight> knights;
    vector<Rook> rooks;
    vector<Cannon> cannons;
    vector<Pawn> pawns;
    */
    vector<shared_ptr<Piece>> piePtrs;
};

#endif