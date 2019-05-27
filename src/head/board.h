#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <vector>

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {
class Seat;
}

enum class PieceColor;
enum class PieceKind;
enum class RecFormat;
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

namespace BoardSpace {

class Board {
public:
    Board();

    const bool isBottomSide(const PieceColor color) const { return bottomColor_ == color; }
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int row, const int col) const;
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int rowcol) const;
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const std::pair<int, int>& rowcol) const { return getSeat(rowcol.first, rowcol.second); }
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getSeats(std::vector<std::pair<int, int>> rowcols = std::vector<std::pair<int, int>>{}) const;

    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromZh(const std::wstring& Zh) const; // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const; // (fseat, tseat)->中文纵线着法, 着法未走状态

    const bool isKilled(const PieceColor color) const; //判断是否将军
    const bool isDied(const PieceColor color) const; //判断是否被将死

    void reset(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);
    const std::wstring getPieceChars() const;
    const std::wstring toString() const;
    const std::wstring test();

private:
    void __setBottomSide();
    const std::shared_ptr<SeatSpace::Seat>& __getKingSeat(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats() const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name, const int col) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveStrongeSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getSortPawnLiveSeats(const PieceColor color, const wchar_t name) const;

    PieceColor bottomColor_; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    const std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个
};
}

#endif