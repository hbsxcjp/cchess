#ifndef SEAT_H
#define SEAT_H

#include <map>
#include <memory>
#include <vector>

enum class PieceColor;
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

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

    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<Seat>>
    getMoveSeats(const BoardSpace::Board& board);

    // 置入棋子
    void put(const std::shared_ptr<PieceSpace::Piece>& piece = nullptr) { piece_ = piece; }
    const std::shared_ptr<PieceSpace::Piece>
    movTo(Seat& tseat, const std::shared_ptr<PieceSpace::Piece>& fillPiece = nullptr);
    void reset() { put(); }

    const std::wstring toString() const;

private:
    const std::vector<std::shared_ptr<Seat>>
    __getNonOwnerSeats(const BoardSpace::Board& board, const std::vector<std::shared_ptr<Seat>>& seats) const;

    const int row_;
    const int col_;
    std::shared_ptr<PieceSpace::Piece> piece_{};
};

class Seats {
public:
    Seats();

    const std::shared_ptr<Seat>& getSeat(const int row, const int col) const;
    const std::shared_ptr<Seat>& getSeat(const int rowcol) const;
    const std::shared_ptr<Seat>& getSeat(const std::pair<int, int>& rowcol) const { return getSeat(rowcol.first, rowcol.second); }
    const std::vector<std::shared_ptr<Seat>> getAllSeats() const;

    const std::shared_ptr<Seat>&
    getKingSeat(const PieceColor color) const;
    const std::vector<std::shared_ptr<Seat>>
    getLiveSeats(const PieceColor color, const wchar_t name = L'\x00', const int col = -1, bool getStronge = false) const;
    const std::vector<std::shared_ptr<Seat>>
    getSortPawnLiveSeats(bool isBottom, const PieceColor color, const wchar_t name) const;

    void reset();
    void reset(const std::vector<std::shared_ptr<PieceSpace::Piece>>& boardPieces);

    const std::wstring getPieceChars() const;
    const std::wstring toString() const;

private:
    // 一块棋盘位置容器，固定的90个
    const std::vector<std::shared_ptr<Seat>> allSeats_;
};

class SeatManager {
public:
    static const std::vector<std::shared_ptr<Seat>> creatSeats();

    static const int ColNum() { return ColNum_; };
    static const bool isBottom(const int row) { return row < RowLowUpIndex_; };
    static const int getIndex(const int row, const int col) { return row * ColNum_ + col; }
    static const int getIndex(const int rowcol) { return rowcol / 10 * ColNum_ + rowcol % 10; }
    static const int getRotate(int rowcol) { return (RowNum_ - rowcol / 10 - 1) * 10 + (ColNum_ - rowcol % 10 - 1); }
    static const int getSymmetry(int rowcol) { return rowcol + ColNum_ - rowcol % 10 * 2 - 1; }

    static const std::vector<std::shared_ptr<Seat>>
    getAllSeats(const BoardSpace::Board& board);
    static const std::vector<std::shared_ptr<Seat>>
    getKingSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece);
    static const std::vector<std::shared_ptr<Seat>>
    getAdvisorSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece);
    static const std::vector<std::shared_ptr<Seat>>
    getBishopSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece);
    static const std::vector<std::shared_ptr<Seat>>
    getPawnSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece);

    static const std::vector<std::shared_ptr<Seat>>
    getKingMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getAdvisorMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getBishopMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getKnightMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getRookMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getCannonMoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    getPawnMoveSeats(const BoardSpace::Board& board, const Seat& fseat);

    static const std::wstring getSeatsStr(const std::vector<std::shared_ptr<Seat>>& seats);

private:
    static const std::vector<std::pair<int, int>> __getAllRowcols();
    static const std::vector<std::pair<int, int>> __getKingRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> __getAdvisorRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> __getBishopRowcols(bool isBottom);
    static const std::vector<std::pair<int, int>> __getPawnRowcols(bool isBottom);

    static const std::vector<std::shared_ptr<Seat>>
    __getSeats(const BoardSpace::Board& board, std::vector<std::pair<int, int>> rowcols);
    static const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>
    __getBishopObs_MoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>
    __getKnightObs_MoveSeats(const BoardSpace::Board& board, const Seat& fseat);
    static const std::vector<std::shared_ptr<Seat>>
    __getNonObstacleSeats(const BoardSpace::Board& board,
        const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>& obs_MoveSeats);
    static const std::vector<std::vector<std::shared_ptr<Seat>>>
    __getRookCannonMoveSeat_Lines(const BoardSpace::Board& board, const Seat& fseat);

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
}

#endif