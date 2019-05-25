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
    const std::shared_ptr<PieceSpace::Piece>& piece() const { return piece_; }
    const bool isDifColor(PieceColor color) const;
    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getMoveSeats(const BoardSpace::Board& board);

    void put(const std::shared_ptr<PieceSpace::Piece>& piece = nullptr) { piece_ = piece; } // 置入棋子
    const std::shared_ptr<PieceSpace::Piece> movTo(Seat& tseat, const std::shared_ptr<PieceSpace::Piece>& fillPiece = nullptr);

    const std::wstring toString() const;

private:
    const int row_;
    const int col_;
    std::shared_ptr<PieceSpace::Piece> piece_{};
};

class RowcolManager {
public:
    static const int ColNum() { return ColNum_; };
    static const bool isBottom(const int row) { return row < RowLowUpIndex_; };
    static const int getIndex(const int row, const int col) { return row * ColNum_ + col; }
    static const int getIndex(const int rowcol) { return rowcol / 10 * ColNum_ + rowcol % 10; }

    static const std::vector<std::pair<int, int>> getAllRowcols();
    static const std::vector<std::pair<int, int>> getKingRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> getAdvisorRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> getBishopRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> getPawnRowcols(bool isBottom);

    static const std::vector<std::pair<int, int>> getKingMoveRowcols(bool isBottom, int frow, int fcol);
    static const std::vector<std::pair<int, int>> getAdvisorMoveRowcols(bool isBottom, int frow, int fcol);
    static const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> getBishopObs_MoveRowcols(bool isBottom, int frow, int fcol);
    static const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> getKnightObs_MoveRowcols(int frow, int fcol);
    static const std::vector<std::vector<std::pair<int, int>>> getRookCannonMoveRowcol_Lines(int frow, int fcol);
    static const std::vector<std::pair<int, int>> getPawnMoveRowcols(bool isBottom, int frow, int fcol);

    static const int getRotate(int rowcol) { return (RowNum_ - rowcol / 10 - 1) * 10 + (ColNum_ - rowcol % 10 - 1); }
    static const int getSymmetry(int rowcol) { return rowcol + ColNum_ - rowcol % 10 * 2 - 1; }

private:
    static const int RowNum_{ 10 };
    static const int ColNum_{ 9 };
    static const int RowLowIndex_{ 0 };
    static const int RowLowMidIndex_{ 2 };
    static const int RowLowUpIndex_{ 4 };
    static const int RowUpLowIndex_{ 5 };
    static const int RowUpMidIndex_{ 7 };
    static const int RowUpIndex_{ 9 };
    static const int ColLowIndex_{ 0 };
    static const int ColMidLowIndex_{ 3 };
    static const int ColMidUpIndex_{ 5 };
    static const int ColUpIndex_{ 8 };
};

const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats();
const std::wstring getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats);
}

#endif