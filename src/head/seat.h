#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>
enum class PieceColor;

namespace BoardSpace {
class Board;
}

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {

class Seat {

public:
    explicit Seat(int row, int col);

    const int row() const { return row_; }
    const int col() const { return col_; }
    const int rowcol() const { return row_ * 10 + col_; }
    const std::shared_ptr<PieceSpace::Piece> piece() const { return piece_; }
    const bool isDiffColor(const std::shared_ptr<Seat>& fseat) const;
    void put(const std::shared_ptr<PieceSpace::Piece>& piece = nullptr) { piece_ = piece; } // 置入棋子
    const std::shared_ptr<PieceSpace::Piece> to(std::shared_ptr<Seat>& tseat,
        const std::shared_ptr<PieceSpace::Piece>& fillPiece = nullptr);
    const std::wstring toString() const;

private:
    const int row_;
    const int col_;
    std::shared_ptr<PieceSpace::Piece> piece_{};
};
}

#endif