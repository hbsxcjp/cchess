#include "seat.h"
#include "board.h"
#include "piece.h"
#include <algorithm>
//#include <iomanip>
#include <cassert>
#include <sstream>
#include <string>

using namespace BoardSpace;
using namespace PieceSpace;
namespace SeatSpace {

Seat::Seat(int row, int col)
    : row_{ row }
    , col_{ col }
{
}

const std::vector<std::shared_ptr<Seat>>
Seat::getMoveSeats(const BoardSpace::Board& board)
{
    assert(piece());
    std::vector<std::shared_ptr<Seat>> moveSeats{
        __getNonOwnerSeats(board, piece()->moveSeats(board, *this))
    };
    PieceColor color{ piece()->color() };
    auto pos = std::remove_if(moveSeats.begin(), moveSeats.end(),
        [&](std::shared_ptr<Seat>& tseat) {
            auto& eatPiece = movTo(*tseat);
            // 移动棋子后，检测是否会被对方将军
            bool killed{ board.isKilled(color) };
            tseat->movTo(*this, eatPiece);
            return killed;
        });
    return std::vector<std::shared_ptr<Seat>>{ moveSeats.begin(), pos };
}

const std::shared_ptr<PieceSpace::Piece>
Seat::movTo(Seat& tseat, const std::shared_ptr<PieceSpace::Piece>& fillPiece)
{
    auto eatPiece = tseat.piece();
    tseat.put(piece_);
    put(fillPiece);
    return eatPiece;
}

const std::wstring Seat::toString() const
{
    std::wstringstream wss{};
    wss << row_ << col_ << (piece_ ? piece_->name() : L'_'); //<< std::boolalpha << std::setw(2) <<
    return wss.str();
}

const std::vector<std::shared_ptr<Seat>>
Seat::__getNonOwnerSeats(const BoardSpace::Board& board,
    const std::vector<std::shared_ptr<Seat>>& seats) const
{
    std::vector<std::shared_ptr<Seat>> nonOwnerSeats{};
    std::copy_if(seats.begin(), seats.end(), std::back_inserter(nonOwnerSeats),
        [&](const std::shared_ptr<Seat>& seat) {
            return !seat->piece() || seat->piece()->color() != piece_->color();
        });
    return nonOwnerSeats;
}

Seats::Seats()
    : allSeats_{ SeatManager::creatSeats() }
{
}

const std::shared_ptr<Seat>& Seats::getSeat(const int row, const int col) const
{
    return allSeats_.at(SeatManager::getIndex(row, col));
}

const std::shared_ptr<Seat>& Seats::getSeat(const int rowcol) const
{
    return allSeats_.at(SeatManager::getIndex(rowcol));
}

const std::vector<std::shared_ptr<Seat>> Seats::getAllSeats() const
{
    return allSeats_;
}

const std::shared_ptr<Seat>&
Seats::getKingSeat(const PieceColor color) const
{
    auto& liveSeats = getLiveSeats(color);
    auto pos = std::find_if(liveSeats.begin(), liveSeats.end(),
        [&](const std::shared_ptr<Seat>& seat) {
            return PieceManager::isKing(seat->piece()->name());
        });
    assert(pos != liveSeats.end());
    return *pos;
}

const std::vector<std::shared_ptr<Seat>>
Seats::getLiveSeats(const PieceColor color, const wchar_t name, const int col, bool getStronge) const
{
    std::vector<std::shared_ptr<Seat>> seats{};
    std::copy_if(allSeats_.begin(), allSeats_.end(), std::back_inserter(seats),
        [&](const std::shared_ptr<Seat>& seat) {
            auto& piece = seat->piece();
            return piece && color == piece->color()
                && (name == L'\x0' || name == piece->name())
                && (col == -1 || col == seat->col())
                && (!getStronge || PieceManager::isStronge(piece->name()));
        });
    return seats;
}

// '多兵排序'
const std::vector<std::shared_ptr<Seat>>
Seats::getSortPawnLiveSeats(bool isBottom, const PieceColor color, const wchar_t name) const
{
    // 最多5个兵
    std::vector<std::shared_ptr<Seat>>
        pawnSeats{ getLiveSeats(color, name) }, seats{};
    // 按列建立字典，按列排序
    std::map<int, std::vector<std::shared_ptr<Seat>>> colSeats{};
    std::for_each(pawnSeats.begin(), pawnSeats.end(),
        [&](const std::shared_ptr<Seat>& seat) {
            colSeats[isBottom ? -seat->col() : seat->col()].push_back(seat);
        }); // 底边则列倒序,每列位置倒序

    // 整合成一个数组
    std::for_each(colSeats.begin(), colSeats.end(),
        [&](const std::pair<int, std::vector<std::shared_ptr<Seat>>>& colSeat) {
            if (colSeat.second.size() > 1) // 筛除只有一个位置的列
                copy(colSeat.second.begin(), colSeat.second.end(), std::back_inserter(seats));
        }); //按列存入
    return seats;
}

void Seats::reset()
{
    std::for_each(allSeats_.begin(), allSeats_.end(),
        [&](const std::shared_ptr<Seat>& seat) { seat->reset(); });
}

void Seats::reset(const std::vector<std::shared_ptr<PieceSpace::Piece>>& boardPieces)
{
    assert(allSeats_.size() == boardPieces.size());
    int index{ 0 };
    std::for_each(allSeats_.begin(), allSeats_.end(),
        [&](const std::shared_ptr<Seat>& seat) { seat->put(boardPieces.at(index++)); });
}

const std::wstring Seats::getPieceChars() const
{
    std::wstringstream wss{};
    std::for_each(allSeats_.begin(), allSeats_.end(),
        [&](const std::shared_ptr<Seat>& seat) {
            wss << (seat->piece() ? seat->piece()->ch() : PieceManager::nullChar());
        });
    return wss.str();
}

const std::wstring Seats::toString() const
{
    std::wstringstream wss{};
    std::for_each(allSeats_.begin(), allSeats_.end(),
        [&](const std::shared_ptr<Seat>& seat) {
            wss << seat->toString() << L' ';
        });
    return wss.str();
}

const std::vector<std::shared_ptr<Seat>> SeatManager::creatSeats()
{
    std::vector<std::pair<int, int>> allRowcols = __getAllRowcols();
    std::vector<std::shared_ptr<Seat>> seats{};
    std::for_each(allRowcols.begin(), allRowcols.end(),
        [&](std::pair<int, int>& rowcol) {
            seats.push_back(std::make_shared<Seat>(rowcol.first, rowcol.second));
        });
    return seats;
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getAllSeats(const BoardSpace::Board& board)
{
    return __getSeats(board, __getAllRowcols());
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getKingSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece)
{
    return __getSeats(board, __getKingRowcols(board.isBottomSide(piece.color())));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getAdvisorSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece)
{
    return __getSeats(board, __getAdvisorRowcols(board.isBottomSide(piece.color())));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getBishopSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece)
{
    return __getSeats(board, __getBishopRowcols(board.isBottomSide(piece.color())));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getPawnSeats(const BoardSpace::Board& board, const PieceSpace::Piece& piece)
{
    return __getSeats(board, __getPawnRowcols(board.isBottomSide(piece.color())));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getKingMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    bool isBottom{ board.isBottomSide(fseat.piece()->color()) };
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::pair<int, int>> rowcols{
        { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = std::remove_if(rowcols.begin(), rowcols.end(),
        [&](const std::pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return __getSeats(board, std::vector<std::pair<int, int>>{ rowcols.begin(), pos });
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getAdvisorMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    bool isBottom{ board.isBottomSide(fseat.piece()->color()) };
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::pair<int, int>> rowcols{
        { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = std::remove_if(rowcols.begin(), rowcols.end(),
        [&](const std::pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return __getSeats(board, std::vector<std::pair<int, int>>{ rowcols.begin(), pos });
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getBishopMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    return __getNonObstacleSeats(board, __getBishopObs_MoveSeats(board, fseat));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getKnightMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    return __getNonObstacleSeats(board, __getKnightObs_MoveSeats(board, fseat));
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getRookMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    std::vector<std::shared_ptr<Seat>> moveSeats{};
    for (auto& seats : __getRookCannonMoveSeat_Lines(board, fseat))
        for (auto& seat : seats) {
            moveSeats.push_back(seat);
            if (seat->piece())
                break;
        }
    return moveSeats;
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getCannonMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    std::vector<std::shared_ptr<Seat>> moveSeats{};
    for (auto& seats : __getRookCannonMoveSeat_Lines(board, fseat)) {
        bool skip = false;
        for (auto& seat : seats) {
            if (!skip) {
                if (!seat->piece())
                    moveSeats.push_back(seat);
                else
                    skip = true;
            } else if (seat->piece()) {
                moveSeats.push_back(seat);
                break;
            }
        }
    }
    return moveSeats;
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::getPawnMoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    bool isBottom{ board.isBottomSide(fseat.piece()->color()) };
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::shared_ptr<Seat>> seats{};
    int row{}, col{};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        seats.push_back(board.getSeat(row, fcol));
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            seats.push_back(board.getSeat(frow, col));
        if ((col = fcol + 1) <= ColUpIndex_)
            seats.push_back(board.getSeat(frow, col));
    }
    return seats;
}

const std::wstring SeatManager::getSeatsStr(const std::vector<std::shared_ptr<Seat>>& seats)
{
    std::wstringstream wss{};
    wss << seats.size() << L"个: ";
    std::for_each(seats.begin(), seats.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            wss << seat->toString() << L' ';
        });
    return wss.str();
}

const std::vector<std::pair<int, int>> SeatManager::__getAllRowcols()
{
    std::vector<std::pair<int, int>> rowcols{};
    for (int row = 0; row < RowNum_; ++row)
        for (int col = 0; col < ColNum_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> SeatManager::__getKingRowcols(bool isBottom)
{
    std::vector<std::pair<int, int>> rowcols{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            rowcols.push_back({ row, col });
    return rowcols;
}

const std::vector<std::pair<int, int>> SeatManager::__getAdvisorRowcols(bool isBottom)
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

const std::vector<std::pair<int, int>> SeatManager::__getBishopRowcols(bool isBottom)
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

const std::vector<std::pair<int, int>> SeatManager::__getPawnRowcols(bool isBottom)
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

const std::vector<std::shared_ptr<Seat>>
SeatManager::__getSeats(const BoardSpace::Board& board, std::vector<std::pair<int, int>> rowcols)
{
    std::vector<std::shared_ptr<Seat>> seats{};
    std::for_each(rowcols.begin(), rowcols.end(), [&](std::pair<int, int>& rowcol) {
        seats.push_back(board.getSeat(rowcol));
    });
    return seats;
}

const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>
SeatManager::__getBishopObs_MoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    bool isBottom{ board.isBottomSide(fseat.piece()->color()) };
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>> obs_MoveSeats{};
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> obs_MoveRowcols{
        { { frow - 1, fcol - 1 }, { frow - 2, fcol - 2 } },
        { { frow - 1, fcol + 1 }, { frow - 2, fcol + 2 } },
        { { frow + 1, fcol - 1 }, { frow + 2, fcol - 2 } },
        { { frow + 1, fcol + 1 }, { frow + 2, fcol + 2 } }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    std::for_each(obs_MoveRowcols.begin(), obs_MoveRowcols.end(),
        [&](const std::pair<std::pair<int, int>, std::pair<int, int>>& obs_Moverowcol) {
            if (obs_Moverowcol.second.first >= rowLow
                && obs_Moverowcol.second.first <= rowUp
                && obs_Moverowcol.second.second >= ColLowIndex_
                && obs_Moverowcol.second.second <= ColUpIndex_)
                obs_MoveSeats.push_back(
                    { board.getSeat(obs_Moverowcol.first),
                        board.getSeat(obs_Moverowcol.second) });
        });
    return obs_MoveSeats;
}

const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>
SeatManager::__getKnightObs_MoveSeats(const BoardSpace::Board& board, const Seat& fseat)
{
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>> obs_MoveSeats{};
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
    std::for_each(obs_MoveRowcols.begin(), obs_MoveRowcols.end(),
        [&](const std::pair<std::pair<int, int>, std::pair<int, int>>& obs_Moverowcol) {
            if (obs_Moverowcol.second.first >= RowLowIndex_
                && obs_Moverowcol.second.first <= RowUpIndex_
                && obs_Moverowcol.second.second >= ColLowIndex_
                && obs_Moverowcol.second.second <= ColUpIndex_)
                obs_MoveSeats.push_back(
                    { board.getSeat(obs_Moverowcol.first),
                        board.getSeat(obs_Moverowcol.second) });
        });
    return obs_MoveSeats;
}

const std::vector<std::shared_ptr<Seat>>
SeatManager::__getNonObstacleSeats(const BoardSpace::Board& board,
    const std::vector<std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>>& obs_MoveSeats)
{
    std::vector<std::shared_ptr<Seat>> seats{};
    std::for_each(obs_MoveSeats.begin(), obs_MoveSeats.end(),
        [&](const std::pair<std::shared_ptr<Seat>, std::shared_ptr<Seat>>& obs_MoveSeat) {
            if (!obs_MoveSeat.first->piece())
                seats.push_back(obs_MoveSeat.second);
        });
    return seats;
}

const std::vector<std::vector<std::shared_ptr<Seat>>>
SeatManager::__getRookCannonMoveSeat_Lines(const BoardSpace::Board& board, const Seat& fseat)
{
    int frow{ fseat.row() }, fcol{ fseat.col() };
    std::vector<std::vector<std::shared_ptr<Seat>>> seat_Lines(4);
    for (int col = fcol - 1; col >= ColLowIndex_; --col)
        seat_Lines[0].push_back(board.getSeat(frow, col));
    for (int col = fcol + 1; col <= ColUpIndex_; ++col)
        seat_Lines[1].push_back(board.getSeat(frow, col));
    for (int row = frow - 1; row >= RowLowIndex_; --row)
        seat_Lines[2].push_back(board.getSeat(row, fcol));
    for (int row = frow + 1; row <= RowUpIndex_; ++row)
        seat_Lines[3].push_back(board.getSeat(row, fcol));
    return seat_Lines;
}

/*
const std::vector<std::pair<int, int>> SeatManager::getKingMoveRowcols(bool isBottom, const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
    std::vector<std::pair<int, int>> rowcols{
        { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = std::remove_if(rowcols.begin(), rowcols.end(),
        [&](const std::pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return std::vector<std::pair<int, int>>{ rowcols.begin(), pos };
}

const std::vector<std::pair<int, int>> SeatManager::getAdvisorMoveRowcols(bool isBottom, const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
    std::vector<std::pair<int, int>> rowcols{
        { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = std::remove_if(rowcols.begin(), rowcols.end(),
        [&](const std::pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return std::vector<std::pair<int, int>>{ rowcols.begin(), pos };
}

const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> SeatManager::getBishopObs_MoveRowcols(bool isBottom, const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
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

const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> SeatManager::getKnightObs_MoveRowcols(const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
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

const std::vector<std::vector<std::pair<int, int>>> SeatManager::getRookCannonMoveRowcol_Lines(const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
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

const std::vector<std::pair<int, int>> SeatManager::getPawnMoveRowcols(bool isBottom, const Seat& seat)
{
    int frow{ seat.row() }, fcol{ seat.col() };
    std::vector<std::pair<int, int>> rowcols{};
    int row{}, col{};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        rowcols.push_back({ row, fcol });
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            rowcols.push_back({ frow, col });
        if ((col = fcol + 1) <= ColUpIndex_)
            rowcols.push_back({ frow, col });
    }
    return rowcols;
}
*/
}