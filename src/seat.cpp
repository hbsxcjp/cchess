#include "seat.h"
#include "piece.h"
//#include <iomanip>
#include <sstream>
#include <string>

namespace SeatSpace {

const bool Seat::isDiffColor(const std::shared_ptr<Seat>& fseat) const
{
    return !piece_ || piece_->color() != fseat->piece()->color();
}

const std::wstring Seat::toString() const
{
    std::wstringstream wss{};
    wss << row_ << col_ << (piece_ ? piece_->name() : L' '); //<< std::boolalpha << std::setw(2) <<
    return wss.str();
}

const std::shared_ptr<PieceSpace::Piece> Seat::to(std::shared_ptr<Seat>& tseat,
    const std::shared_ptr<PieceSpace::Piece>& fillPiece)
{
    const auto& eatPiece = tseat->piece();
    tseat->put(piece_);
    put(fillPiece);
    return eatPiece;
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Seat::__moveSeats(const BoardSpace::Board& board) const
{
    piece()->__moveSeats(board, *this);
}
}