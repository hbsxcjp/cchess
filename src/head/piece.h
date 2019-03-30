#ifndef PIECE_H
#define PIECE_H

#include <memory>
#include <vector>

namespace SeatSpace {
class Seat;
}

namespace BoardSpace {
class Board;
}

enum class PieceColor { RED,
    BLACK,
    BLANK
};

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
    virtual ~Piece() = default;

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const { return name_; }
    const PieceColor color() const { return color_; }
    // 棋子可置放的全部位置
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const = 0;
    const std::wstring toString() const;

private:
    // 棋子可移动到的全部位置
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const = 0;
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
};

class King : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Advisor : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Bishop : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Knight : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Rook : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Cannon : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};

class Pawn : public Piece {
public:
    using Piece::Piece;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(const BoardSpace::Board& board) const;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board,
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
};
}

#endif