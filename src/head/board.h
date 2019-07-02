#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <vector>

namespace PieceSpace {
class Piece;
class Pieces;
}

namespace SeatSpace {
class Seat;
class Seats;
}

enum class PieceColor;
enum class PieceKind;
enum class RecFormat;
enum class ChangeType;

namespace BoardSpace {

class Board {
public:
    Board();

    const bool isBottomSide(const PieceColor color) const { return bottomColor_ == color; }
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int row, const int col) const;
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int rowcol) const;
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const std::pair<int, int>& rowcol) const;

    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromZh(const std::wstring& Zh) const; // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const; // (fseat, tseat)->中文纵线着法, 着法未走状态

    const bool isKilled(const PieceColor color) const; //判断是否将军
    const bool isDied(const PieceColor color) const; //判断是否被将死

    void reset();
    void reset(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);
    const std::wstring getPieceChars() const;
    const std::wstring toString() const;
    const std::wstring test();

private:
    void __setBottomSide();

    PieceColor bottomColor_; // 底端棋子颜色
    const std::shared_ptr<PieceSpace::Pieces> pieces_;
    const std::shared_ptr<SeatSpace::Seats> seats_;
};
}

#endif