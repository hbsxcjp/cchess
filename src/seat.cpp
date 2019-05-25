#include "seat.h"
#include "board.h"
#include "piece.h"
#include <algorithm>
//#include <iomanip>
#include <cassert>
#include <sstream>
#include <string>

using namespace BoardSpace;
namespace SeatSpace {

Seat::Seat(int row, int col)
    : row_{ row }
    , col_{ col }
{
}

const bool Seat::isDifColor(PieceColor color) const
{
    return !piece_ || piece_->color() != color;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Seat::getMoveSeats(const BoardSpace::Board& board)
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> moveSeats{ piece()->moveSeats(board, *this) };
    PieceColor color{ piece()->color() };
    auto pos = std::remove_if(moveSeats.begin(), moveSeats.end(),
        [&](std::shared_ptr<SeatSpace::Seat>& tseat) {
            auto& eatPiece = movTo(*tseat);
            bool killed{ board.isKilled(color) }; // 移动棋子后，检测是否会被对方将军
            tseat->movTo(*this, eatPiece);
            return killed;
        });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ moveSeats.begin(), pos };
}

const std::wstring Seat::toString() const
{
    std::wstringstream wss{};
    wss << row_ << col_ << (piece_ ? piece_->name() : L'_'); //<< std::boolalpha << std::setw(2) <<
    return wss.str();
}

const std::shared_ptr<PieceSpace::Piece> Seat::movTo(Seat& tseat, const std::shared_ptr<PieceSpace::Piece>& fillPiece)
{
    auto eatPiece = tseat.piece();
    tseat.put(piece_);
    put(fillPiece);
    return eatPiece;
}

const std::vector<std::pair<int, int>> RowcolManager::getAllRowcols()
{
    std::vector<std::pair<int, int>> rowcols{};
    for (int row = 0; row < RowNum_; ++row)
        for (int col = 0; col < ColNum_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getKingRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getAdvisorRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            if ((col + row) % 2 == rmd)
                rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getBishopRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2) {
            int gap{ row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                rowcols.push_back({ row, col });
        }
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getPawnRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int lfrow{ isBottom ? RowLowUpIndex_ - 1 : RowUpLowIndex_ },
        ufrow{ isBottom ? RowLowUpIndex_ : RowUpLowIndex_ + 1 },
        ltrow{ isBottom ? RowUpLowIndex_ : RowLowIndex_ },
        utrow{ isBottom ? RowUpIndex_ : RowLowUpIndex_ };
    for (int row = lfrow; row <= ufrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2)
            rowcols.push_back({ row, col });
    for (int row = ltrow; row <= utrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getKingMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol } },
        moveRowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::pair<int, int>> RowcolManager::getAdvisorMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> rowcols{ { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 } },
        moveRowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    std::copy_if(rowcols.begin(), rowcols.end(), std::back_inserter(moveRowcols),
        [&](const std::pair<int, int>& rowcol) {
            return (rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return moveRowcols;
}

const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> RowcolManager::getBishopObs_MoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> obs_MoveRowcols{
        { { frow - 1, fcol - 1 }, { frow - 2, fcol - 2 } },
        { { frow - 1, fcol + 1 }, { frow - 2, fcol + 2 } },
        { { frow + 1, fcol - 1 }, { frow + 2, fcol - 2 } },
        { { frow + 1, fcol + 1 }, { frow + 2, fcol + 2 } }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    auto pos = std::remove_if(obs_MoveRowcols.begin(), obs_MoveRowcols.end(),
        [&](const std::pair<std::pair<int, int>, std::pair<int, int>>& obs_Moverowcol) {
            return !(obs_Moverowcol.second.first >= rowLow && obs_Moverowcol.second.first <= rowUp
                && obs_Moverowcol.second.second >= ColLowIndex_ && obs_Moverowcol.second.second <= ColUpIndex_);
        });
    return std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>{ obs_MoveRowcols.begin(), pos };
}

const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> RowcolManager::getKnightObs_MoveRowcols(int frow, int fcol)
{
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> obs_MoveRowcols{
        { { frow - 1, fcol }, { frow - 2, fcol - 1 } },
        { { frow - 1, fcol }, { frow - 2, fcol + 1 } },
        { { frow, fcol - 1 }, { frow - 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow - 1, fcol + 2 } },
        { { frow, fcol - 1 }, { frow + 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow + 1, fcol + 2 } },
        { { frow + 1, fcol }, { frow + 2, fcol - 1 } },
        { { frow + 1, fcol }, { frow + 2, fcol + 1 } }
    };
    auto pos = std::remove_if(obs_MoveRowcols.begin(), obs_MoveRowcols.end(),
        [&](const std::pair<std::pair<int, int>, std::pair<int, int>>& obs_Moverowcol) {
            return !(obs_Moverowcol.second.first >= RowLowIndex_ && obs_Moverowcol.second.first <= RowUpIndex_
                && obs_Moverowcol.second.second >= ColLowIndex_ && obs_Moverowcol.second.second <= ColUpIndex_);
        });
    return std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>{ obs_MoveRowcols.begin(), pos };
}

const std::vector<std::vector<std::pair<int, int>>> RowcolManager::getRookCannonMoveRowcol_Lines(int frow, int fcol)
{
    std::vector<std::vector<std::pair<int, int>>> rowcol_Lines(4);
    for (int col = fcol - 1; col >= ColLowIndex_; --col)
        rowcol_Lines[0].push_back({ frow, col });
    for (int col = fcol + 1; col <= ColUpIndex_; ++col)
        rowcol_Lines[1].push_back({ frow, col });
    for (int row = frow - 1; row >= RowLowIndex_; --row)
        rowcol_Lines[2].push_back({ row, fcol });
    for (int row = frow + 1; row <= RowUpIndex_; ++row)
        rowcol_Lines[3].push_back({ row, fcol });
    return rowcol_Lines;
}

const std::vector<std::pair<int, int>> RowcolManager::getPawnMoveRowcols(bool isBottom, int frow, int fcol)
{
    std::vector<std::pair<int, int>> moveRowcols{};
    int row{}, col{};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        moveRowcols.push_back({ row, fcol });
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            moveRowcols.push_back({ frow, col });
        if ((col = fcol + 1) <= ColUpIndex_)
            moveRowcols.push_back({ frow, col });
    }
    return moveRowcols;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> creatSeats()
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (auto& rowcol : RowcolManager::getAllRowcols())
        seats.push_back(std::make_shared<SeatSpace::Seat>(rowcol.first, rowcol.second));
    return seats;
}

const std::wstring getSeatsStr(const std::vector<std::shared_ptr<SeatSpace::Seat>>& seats)
{
    std::wstringstream wss{};
    wss << seats.size() << L"个: ";
    std::for_each(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << seat->toString() << L' '; });
    return wss.str();
}
}