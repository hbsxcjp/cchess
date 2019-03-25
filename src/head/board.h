#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <memory>
#include <vector>

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {
class Seat;
}

namespace MoveSpace {
class Move;
}

enum class PieceColor;
enum class RecFormat;
enum class BoardSide {
    BOTTOM,
    TOP
};
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

namespace BoardSpace {

class Board {
public:
    Board();

    const bool isBottomSide(const PieceColor color) const { return bottomColor == color; }
    const BoardSide getSide(const PieceColor color) const { return isBottomSide(color) ? BoardSide::BOTTOM : BoardSide::TOP; }

    std::shared_ptr<SeatSpace::Seat>& getSeat(const int row, const int col);
    std::shared_ptr<SeatSpace::Seat>& getSeat(const int rowcol);
    std::shared_ptr<SeatSpace::Seat>& getOthSeat(const std::shared_ptr<SeatSpace::Seat>& seat, const ChangeType ct);
    std::vector<std::shared_ptr<SeatSpace::Seat>> getLiveSeats(const PieceColor color, const wchar_t name = L'\x00', const int col = -1) const;
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> getMoveSeats(const MoveSpace::Move& move, const RecFormat fmt);
    const std::wstring getIccs(const MoveSpace::Move& move) const;
    const std::wstring getZh(const MoveSpace::Move& move); // (fseat, tseat)->中文纵线着法, 着法未走状态

    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const std::wstring getFEN(const std::wstring& pieceChars) const;
    void putPieces(const std::wstring& fen);
    const std::wstring changeSide(const ChangeType ct);
    void setBottomSide();

    const std::wstring toString() const;
    const std::wstring test(); // const;

private:
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromICCS(const std::wstring& ICCS);
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromZh(const std::wstring& Zh); // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring __getChars(const std::wstring& fen) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __sortPawnSeats(const PieceColor color, const wchar_t name);

    PieceColor bottomColor; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个
};
}

#endif