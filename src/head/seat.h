#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>
enum class PieceColor;

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {

class Seat {

public:
    explicit Seat(int row, int col, const std::shared_ptr<PieceSpace::Piece> piece = nullptr)
        : row_{ row }
        , col_{ col }
        , piece_{ piece }
    {
    }

    const int row() const { return row_; }
    const int col() const { return col_; }
    const int rowcolValue() const { return row_ * 10 + col_; } // 十位为行，个位为列
    const std::shared_ptr<PieceSpace::Piece> piece() const { return piece_; }
    const bool isDiffColor(const std::shared_ptr<Seat>& fseat);

    void put(const std::shared_ptr<PieceSpace::Piece> piece = nullptr) { piece_ = piece; } // 置入棋子
    const std::shared_ptr<PieceSpace::Piece>& to(std::shared_ptr<Seat>& tseat, const std::shared_ptr<PieceSpace::Piece> fillPiece = nullptr);
    const std::wstring toString() const;

private:
    const int row_; 
    const int col_; 
    std::shared_ptr<PieceSpace::Piece> piece_;
};
}

#endif