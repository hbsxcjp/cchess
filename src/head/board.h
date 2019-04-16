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
    //friend class PieceSpace::Piece;
    friend class PieceSpace::King;
    friend class PieceSpace::Advisor;
    friend class PieceSpace::Bishop;
    friend class PieceSpace::Knight;
    friend class PieceSpace::Rook;
    friend class PieceSpace::Cannon;
    friend class PieceSpace::Pawn;

public:
    Board();

    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int row, const int col) const { return seats_.at(row * ColNum + col); }
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int rowcol) const { return getSeat(rowcol / 10, rowcol % 10); }
    const std::shared_ptr<SeatSpace::Seat>& getRotateSeat(const std::shared_ptr<SeatSpace::Seat>& seat) const;
    const std::shared_ptr<SeatSpace::Seat>& getSymmetrySeat(const std::shared_ptr<SeatSpace::Seat>& seat) const;

    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromIccs(const std::wstring& ICCS) const;
    const std::wstring getIccs(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const;
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromZh(const std::wstring& Zh) const; // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const; // (fseat, tseat)->中文纵线着法, 着法未走状态

    const std::wstring toString() const;
    const std::wstring test();

    const bool isKilled(const PieceColor color) const; //判断是否将军
    const bool isDied(const PieceColor color) const; //判断是否被将死
    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::wstring getPieceChars() const;
    void putPieces(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);
    static const wchar_t nullChar{ L'_' };

private:
    const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces() const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats() const;

    const std::vector<std::shared_ptr<SeatSpace::Seat>>& getAllSeats() const { return seats_; }
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
    const std::vector<std::vector<std::shared_ptr<SeatSpace::Seat>>> __getRookCannonMoveSeat_Lines(
        const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> getPawnMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::wstring __getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats) const;

    void __setBottomSide();
    const PieceColor __getColor(const wchar_t numZh) const;
    const bool __isBottomSide(const PieceColor color) const { return bottomColor == color; }
    const std::shared_ptr<SeatSpace::Seat> __getKingSeat(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __sortPawnSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats() const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name, const int col) const;

    const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar) const
    {
        int index{ static_cast<int>(__getPreChars(seatsLen).find(preChar)) };
        return isBottom ? seatsLen - 1 - index : index;
    }
    const wchar_t getIndexChar(const int seatsLen, const bool isBottom, const int index) const
    {
        return __getPreChars(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
    }
    const std::wstring __getPreChars(const int length) const
    {
        switch (length) {
        case 2:
            return L"前后";
        case 3:
            return L"前中后";
        default:
            return L"一二三四五";
        }
    }
    const int getMovNum(const bool isBottom, const wchar_t movChar) const { return (static_cast<int>(movChars.find(movChar)) - 1) * (isBottom ? 1 : -1); }
    const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isLowToUp) const { return movChars.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0)); }
    const int getNum(const PieceColor color, const wchar_t numChar) const { return static_cast<int>(numChars.at(color).find(numChar)) + 1; }
    const wchar_t getNumChar(const PieceColor color, const int num) const { return numChars.at(color).at(num - 1); }
    const int getCol(bool isBottom, const int num) const { return isBottom ? ColNum - num : num - 1; }
    const wchar_t getColChar(const PieceColor color, bool isBottom, const int col) const { return numChars.at(color).at(isBottom ? ColNum - col - 1 : col); }

    const bool isKing(const wchar_t name) const { return nameChars.substr(0, 2).find(name) != std::wstring::npos; }
    const bool isAdvBish(const wchar_t name) const { return nameChars.substr(2, 4).find(name) != std::wstring::npos; }
    const bool isStronge(const wchar_t name) const { return nameChars.substr(6, 5).find(name) != std::wstring::npos; }
    const bool isLineMove(const wchar_t name) const { return isKing(name) || nameChars.substr(7, 4).find(name) != std::wstring::npos; }
    const bool isPawn(const wchar_t name) const { return nameChars.substr(nameChars.size() - 2, 2).find(name) != std::wstring::npos; }
    const bool isPieceName(const wchar_t name) const { return nameChars.find(name) != std::wstring::npos; };

    const int RowNum{ 10 };
    const int ColNum{ 9 };
    const int RowLowIndex{ 0 };
    const int RowLowMidIndex{ 2 };
    const int RowLowUpIndex{ 4 };
    const int RowUpLowIndex{ 5 };
    const int RowUpMidIndex{ 7 };
    const int RowUpIndex{ 9 };
    const int ColLowIndex{ 0 };
    const int ColMidLowIndex{ 3 };
    const int ColMidUpIndex{ 5 };
    const int ColUpIndex{ 8 };
    static const std::wstring nameChars;
    static const std::wstring movChars;
    static const std::map<PieceColor, std::wstring> numChars;

    PieceColor bottomColor; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    const std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个
};
}

#endif