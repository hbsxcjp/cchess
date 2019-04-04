#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>
enum class PieceColor;

namespace PieceSpace {
class Piece;
class King;
class Advisor;
class Bishop;
class Knight;
class Rook;
class Cannon;
class Pawn;
}

namespace SeatSpace {

class Seat {
    friend class PieceSpace::King;
    friend class PieceSpace::Advisor;
    friend class PieceSpace::Bishop;
    friend class PieceSpace::Knight;
    friend class PieceSpace::Rook;
    friend class PieceSpace::Cannon;
    friend class PieceSpace::Pawn;

public:
    explicit Seat(int row, int col)
        : row_{ row }
        , col_{ col }
    {
    }

    const int row() const { return row_; }
    const int col() const { return col_; }
    const int rowcolValue() const { return row_ * 10 + col_; } // 十位为行，个位为列
    const std::shared_ptr<PieceSpace::Piece> piece() const { return piece_; }
    const bool isDiffColor(const std::shared_ptr<Seat>& fseat) const;
    const std::wstring toString() const;

    void put(const std::shared_ptr<PieceSpace::Piece>& piece = nullptr) { piece_ = piece; } // 置入棋子
    const std::shared_ptr<PieceSpace::Piece> to(std::shared_ptr<Seat>& tseat,
        const std::shared_ptr<PieceSpace::Piece>& fillPiece = nullptr);

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __moveSeats(const BoardSpace::Board& board) const;
    const int row_;
    const int col_;
    std::shared_ptr<PieceSpace::Piece> piece_{};
};
}

#endif