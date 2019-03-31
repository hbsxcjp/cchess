#ifndef BOARD_H
#define BOARD_H

#include <map>
#include <memory>
#include <vector>

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
class Seat;
}

enum class PieceColor;
enum class RecFormat;
enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

namespace BoardSpace {

class Board {
    friend class PieceSpace::King;
    friend class PieceSpace::Advisor;
    friend class PieceSpace::Bishop;
    friend class PieceSpace::Knight;
    friend class PieceSpace::Rook;
    friend class PieceSpace::Cannon;
    friend class PieceSpace::Pawn;

public:
    Board();

    const std::shared_ptr<SeatSpace::Seat> getSeat(const int row, const int col) const;
    const std::shared_ptr<SeatSpace::Seat> getSeat(const int rowcol) const;
    const std::shared_ptr<SeatSpace::Seat> getOthSeat(const std::shared_ptr<SeatSpace::Seat>& seat, const ChangeType ct) const;
    const std::wstring getPieceChars() const;
    void putPieces(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);
    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat);
    const bool isKilled(const PieceColor color); //判断是否将军
    const bool isDied(const PieceColor color); //判断是否被将死

    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeat(const std::wstring& ICCSZh, const RecFormat fmt) const;
    const std::wstring getIccs(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const;
    const std::wstring getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const; // (fseat, tseat)->中文纵线着法, 着法未走状态

    const std::wstring toString() const;
    const std::wstring test();

private:
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromIccs(const std::wstring& ICCS) const;
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> __getSeatFromZh(const std::wstring& Zh) const; // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __sortPawnSeats(const PieceColor color, const wchar_t name) const;

    void __setBottomSide();
    const bool __isBottomSide(const PieceColor color) const { return bottomColor == color; }
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats() const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name, const int col) const;
    const std::shared_ptr<SeatSpace::Seat> getKingSeat(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getAllSeats() const { return seats_; }
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getKingSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getAdvisorSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getBishopSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getPawnSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getKingMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getAdvsiorMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getRookMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getCannonMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> getRookCannonMoveSeat_Lines(
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getPawnMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;

    PieceColor bottomColor; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    const std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个
};

const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats();
const wchar_t getChar(const PieceColor color, const int index);
const std::wstring getPreChars(const int length);
const PieceColor getColor(const wchar_t numZh);
const wchar_t getIndexChar(const int length, const bool isBottom, const int index);
const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isLowToUp);
const wchar_t getNumChar(const PieceColor color, const int num);
const wchar_t getColChar(const PieceColor color, bool isBottom, const int col);
const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar);
const int getMovDir(const bool isBottom, const wchar_t movChar);
const int getNum(const PieceColor color, const wchar_t numChar);
const int getCol(bool isBottom, const int num);
const std::wstring __seatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats);

const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces();
const bool isKing(const wchar_t name);
const bool isAdvBish(const wchar_t name);
const bool isStronge(const wchar_t name);
const bool isLineMove(const wchar_t name);
const bool isPawn(const wchar_t name);
const bool isPieceName(const wchar_t name);

const int RowNum{ 10 };
const int RowLowIndex{ 0 };
const int RowLowMidIndex{ 2 };
const int RowLowUpIndex{ 4 };
const int RowUpLowIndex{ 5 };
const int RowUpMidIndex{ 7 };
const int RowUpIndex{ 9 };
const int ColNum{ 9 };
const int ColLowIndex{ 0 };
const int ColMidLowIndex{ 3 };
const int ColMidUpIndex{ 5 };
const int ColUpIndex{ 8 };
const wchar_t nullChar{ L'_' };
}

#endif