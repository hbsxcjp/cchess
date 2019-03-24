#ifndef PIECE_H
#define PIECE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class Seat;
class Board;
enum class PieceColor { RED,
    BLACK,
    BLANK };

/* 棋子类
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
    const std::wstring toString() const;

    static const PieceColor getOthColor(const PieceColor color) { return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED; }
    static const bool isKing(const wchar_t name) { return __nameChars.substr(0, 2).find(name) != std::wstring::npos; }
    static const bool isAdvBish(const wchar_t name) { return __nameChars.substr(2, 4).find(name) != std::wstring::npos; }
    static const bool isStronge(const wchar_t name) { return __nameChars.substr(6, 5).find(name) != std::wstring::npos; }
    static const bool isLineMove(const wchar_t name) { return isKing(name) || __nameChars.substr(7, 4).find(name) != std::wstring::npos; }
    static const bool isPawn(const wchar_t name) { return __nameChars.substr(__nameChars.size() - 2, 2).find(name) != std::wstring::npos; }
    static const bool isPieceName(const wchar_t name) { return __nameChars.find(name) != std::wstring::npos; }
    static const std::vector<std::shared_ptr<Piece>> __creatPieces();

    static const wchar_t nullChar;
    static std::shared_ptr<Piece> nullPiece;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;

    static const std::wstring __nameChars;
};
//*/

//*
namespace PieceSpace {
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

    // 棋子可置放的全部位置
    const std::vector<std::shared_ptr<Seat>> getSeats(const PieceColor bottomColor) const;
    // 棋子可移动到的全部位置, 筛除本方棋子所占位置
    //virtual const std::vector<std::shared_ptr<Seat>> filterMoveSeats(const Board& board);
    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<Seat>> getMoveSeats(Board& board);

    //virtual const std::wstring toString() const;
    const std::wstring toString() const;
    virtual ~Piece() = default;
    //static const wchar_t nullChar;

protected:
    // 棋子可置放的全部位置
    virtual const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const = 0;
    // 棋子可移动到的全部位置
    virtual const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
    // 筛除棋子行棋规则不允许的位置
    virtual const std::vector<std::shared_ptr<Seat>> __filterMove_obstruct(const Board& board,
        const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>& seat_obss) const;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
};

class King : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Advisor : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Bishop : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Knight : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Rook : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Cannon : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class Pawn : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

class NullPiece : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<Seat>> __getSeats(const PieceColor bottomColor) const;
    const std::vector<std::shared_ptr<Seat>> __moveSeats(const Board& board) const;
};

extern const wchar_t nullChar;
extern const std::wstring nameChars;
extern const std::shared_ptr<Piece> nullPiece;
extern const std::vector<std::shared_ptr<Piece>> creatPieces();

extern const PieceColor getOthColor(const PieceColor color);
extern const bool isKing(const wchar_t name);
extern const bool isAdvBish(const wchar_t name);
extern const bool isStronge(const wchar_t name);
extern const bool isLineMove(const wchar_t name);
extern const bool isPawn(const wchar_t name);
extern const bool isPieceName(const wchar_t name);
}
//*/

#endif