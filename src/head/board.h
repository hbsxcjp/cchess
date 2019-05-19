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

    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int row, const int col) const;
    const std::shared_ptr<SeatSpace::Seat>& getSeat(const int rowcol) const;

    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromIccs(const std::wstring& ICCS) const;
    const std::wstring getIccs(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const;
    const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
    getMoveSeatFromZh(const std::wstring& Zh) const; // 中文纵线着法->(fseat, tseat), 着法未走状态
    const std::wstring getZh(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat) const; // (fseat, tseat)->中文纵线着法, 着法未走状态

    // 棋子可置放的全部位置
    const std::vector<std::shared_ptr<SeatSpace::Seat>> putSeats(const std::shared_ptr<PieceSpace::Piece>& piece) const;
    // '获取棋子可走的位置, 不能被将军'
    const std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats(std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const bool isKilled(const PieceColor color) const; //判断是否将军
    const bool isDied(const PieceColor color) const; //判断是否被将死

    void reset(const std::wstring& pieceChars);
    void changeSide(const ChangeType ct);

    const std::wstring getPieceChars() const;
    const std::wstring toString() const;
    const std::wstring test();

private:
    void __setBottomSide();
    const bool __isBottomSide(const PieceColor color) const { return bottomColor_ == color; }
    const std::shared_ptr<SeatSpace::Seat>& __getKingSeat(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getSortPawnLiveSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats() const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getLiveSeats(const PieceColor color, const wchar_t name, const int col) const;

    // 棋子可移动到的全部位置
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getKAPMoveSeats(PieceKind kind, const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getBishopMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getKnightMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getRookMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>> __getCannonMoveSeats(const std::shared_ptr<SeatSpace::Seat>& fseat) const;

    PieceColor bottomColor_; // 底端棋子颜色
    const std::vector<std::shared_ptr<PieceSpace::Piece>> pieces_; // 一副棋子，固定的32个
    const std::vector<std::shared_ptr<SeatSpace::Seat>> seats_; // 一块棋盘位置容器，固定的90个
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
    static const std::vector<std::pair<int, int>> getBishopMoveRowcols(bool isBottom, int frow, int fcol);
    static const std::vector<std::pair<int, int>> getKnightMoveRowcols(int frow, int fcol);
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

class PieceCharManager {
public:
    static const int getRowFromICCSChar(const wchar_t ch) { return ch - 48; } // 0:48
    static const int getColFromICCSChar(const wchar_t ch) { return ch - 97; } // a:97
    static const wchar_t getColICCSChar(const int col) { return std::wstring(L"abcdefghi").at(col); } // a:97

    static const std::vector<wchar_t> getAllChs();
    static const wchar_t getName(const wchar_t ch);
    static const wchar_t getPrintName(const PieceSpace::Piece& piece);
    static const PieceColor getColor(const wchar_t ch);
    static const PieceKind getKind(const wchar_t ch);
    static const PieceColor getColorFromZh(const wchar_t numZh);
    static const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar);
    static const wchar_t getIndexChar(const int seatsLen, const bool isBottom, const int index);
    static const wchar_t nullChar() { return nullChar_; };
    static const int getMovNum(const bool isBottom, const wchar_t movChar) { return (static_cast<int>(movChars_.find(movChar)) - 1) * (isBottom ? 1 : -1); }
    static const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isLowToUp) { return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0)); }
    static const int getNum(const PieceColor color, const wchar_t numChar) { return static_cast<int>(numChars_.at(color).find(numChar)) + 1; }
    static const wchar_t getNumChar(const PieceColor color, const int num) { return numChars_.at(color).at(num - 1); }
    static const int getCol(bool isBottom, const int num) { return isBottom ? RowcolManager::ColNum() - num : num - 1; }
    static const wchar_t getColChar(const PieceColor color, bool isBottom, const int col) { return numChars_.at(color).at(isBottom ? RowcolManager::ColNum() - col - 1 : col); }

    static const bool isKing(const wchar_t name) { return nameChars_.substr(0, 2).find(name) != std::wstring::npos; }
    static const bool isAdvBish(const wchar_t name) { return nameChars_.substr(2, 4).find(name) != std::wstring::npos; }
    static const bool isStronge(const wchar_t name) { return nameChars_.substr(6, 5).find(name) != std::wstring::npos; }
    static const bool isLineMove(const wchar_t name) { return isKing(name) || nameChars_.substr(7, 4).find(name) != std::wstring::npos; }
    static const bool isPawn(const wchar_t name) { return nameChars_.substr(nameChars_.size() - 2, 2).find(name) != std::wstring::npos; }
    static const bool isPiece(const wchar_t name) { return nameChars_.find(name) != std::wstring::npos; };

private:
    static const std::wstring __getPreChars(const int length) { return length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五"); }
    static const std::wstring nameChars_;
    static const std::wstring movChars_;
    static const std::map<PieceColor, std::wstring> numChars_;
    static const std::wstring chStr_;
    static const wchar_t nullChar_{ L'_' };
};

const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces();
const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats();
const std::wstring getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats);
}

#endif