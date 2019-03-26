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

    std::vector<std::shared_ptr<SeatSpace::Seat>> getAllSeats();
    std::vector<std::shared_ptr<SeatSpace::Seat>> getKingSeats(const PieceSpace::Piece& piece);
    std::vector<std::shared_ptr<SeatSpace::Seat>> getAdvisorSeats(const PieceSpace::Piece& piece);
    std::vector<std::shared_ptr<SeatSpace::Seat>> getBishopSeats(const PieceSpace::Piece& piece);
    std::vector<std::shared_ptr<SeatSpace::Seat>> getPawnSeats(const PieceSpace::Piece& piece);

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
    const std::wstring test();

private:
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromICCS(const std::wstring& ICCS);
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromZh(const std::wstring& Zh); // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring __getChars(const std::wstring& fen) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __sortPawnSeats(const PieceColor color, const wchar_t name);

    PieceColor bottomColor; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个

    class BoardAide {
    public:
        static const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats();
        static const wchar_t getChar(const PieceColor color, const int index);
        static const std::wstring getPreChars(const int length);
        static const PieceColor getColor(const wchar_t numZh);
        static const wchar_t getIndexChar(const int length, const bool isBottom, const int index);
        static const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isForward);
        static const wchar_t getNumChar(const PieceColor color, const int num);
        static const wchar_t getColChar(const PieceColor color, bool isBottom, const int col);
        static const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar);
        static const int getMovDir(const bool isBottom, const wchar_t movChar);
        static const int getNum(const PieceColor color, const wchar_t numChar);
        static const int getCol(bool isBottom, const int num);

        static const int RowNum;
        static const int RowLowIndex;
        static const int RowLowMidIndex;
        static const int RowLowUpIndex;
        static const int RowUpLowIndex;
        static const int RowUpMidIndex;
        static const int RowUpIndex;
        static const int ColNum;
        static const int ColLowIndex;
        static const int ColMidLowIndex;
        static const int ColMidUpIndex;
        static const int ColUpIndex;
        static const std::wstring movChars;
        static const std::map<PieceColor, std::wstring> numChars;
    };
};
}

#endif