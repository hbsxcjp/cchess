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

enum class PieceColor {
    RED,
    BLACK
};

enum class PieceKind {
    KING,
    ADVSIOR,
    BISHOP,
    KNIGHT,
    ROOK,
    CANNON,
    PAWN
};

namespace PieceSpace {
// 棋子类
class Piece {
public:
    explicit Piece(const wchar_t ch);

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const;
    const PieceColor color() const;
    const PieceKind kind() const;
    const std::wstring toString() const;

private:
    const wchar_t ch_;
};
}

#endif