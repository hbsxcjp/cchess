#ifndef PIECE_H
#define PIECE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SeatSpace {
class Seat;
}

namespace BoardSpace {
class Board;
}

enum class PieceColor { RED,
    BLACK,
    BLANK };

namespace PieceSpace {
// 棋子类
class Piece {
    friend class BoardSpace::Board;

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
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board) = 0;
    // 棋子可移动到的全部位置
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat)
        = 0;

    const std::wstring toString() const;
    virtual ~Piece() = default;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
};

class King : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Advisor : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Bishop : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Knight : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Rook : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Cannon : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class Pawn : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

class NullPiece : public Piece {
public:
    using Piece::Piece;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(BoardSpace::Board& board);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat);
};

extern const std::vector<std::shared_ptr<Piece>> creatPieces();
extern const bool isKing(const wchar_t name);
extern const bool isAdvBish(const wchar_t name);
extern const bool isStronge(const wchar_t name);
extern const bool isLineMove(const wchar_t name);
extern const bool isPawn(const wchar_t name);
extern const bool isPieceName(const wchar_t name);

extern const wchar_t nullChar;
extern const std::wstring nameChars;
extern const std::shared_ptr<Piece> nullPiece;
}

#endif