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

namespace MoveSpace {
class Move;
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

    const bool isKilled(const PieceColor color) const;
    const bool isDied(const PieceColor color) const;

    void reset(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);
    const std::pair<std::shared_ptr<SeatSpace::Seat>, std::shared_ptr<SeatSpace::Seat>>
    getMoveSeat(const std::wstring& zhStr) const;
    const std::wstring getZhStr(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const;

    const std::wstring getPieceChars() const;
    const std::wstring toString() const;
    const std::wstring test();

private:
    void __setBottomSide();

    PieceColor bottomColor_;
    const std::shared_ptr<PieceSpace::Pieces> pieces_;
    const std::shared_ptr<SeatSpace::Seats> seats_;
};
}

#endif