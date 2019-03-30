#include "seat.h"
#include "piece.h"
#include <iomanip>
#include <sstream>
#include <string>

namespace SeatSpace {

const bool Seat::isDiffColor(const std::shared_ptr<Seat>& fseat) const
{
    return !piece_ || piece_->color() != fseat->piece()->color();
}

const std::shared_ptr<PieceSpace::Piece> Seat::to(std::shared_ptr<Seat>& tseat,
    const std::shared_ptr<PieceSpace::Piece>& fillPiece)
{
    auto eatPiece = tseat->piece();
    tseat->put(this->piece());
    this->put(fillPiece);
    return eatPiece;
}

const std::wstring Seat::toString() const
{
    std::wstringstream wss{};
    wss << row_ << col_ << (piece_ ? piece_->name() : L' '); //<< std::boolalpha << std::setw(2) <<
    return wss.str();
}
}