#include "piece.h"
#include "board.h"
#include "seat.h"
#include <iomanip>
#include <sstream>
#include <string>

namespace PieceSpace {

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << std::boolalpha << static_cast<int>(color()) << ch() << name();
    return wss.str();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::getSeats(const BoardSpace::Board& board) const
{
    return board.getKingSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getKingMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::getSeats(const BoardSpace::Board& board) const
{
    return board.getAdvisorSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getAdvsiorMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::getSeats(const BoardSpace::Board& board) const
{
    return board.getBishopSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getBishopMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::getSeats(const BoardSpace::Board& board) const
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getKnightMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::getSeats(const BoardSpace::Board& board) const
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getRookMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::getSeats(const BoardSpace::Board& board) const
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getCannonMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::getSeats(const BoardSpace::Board& board) const
{
    return board.getPawnSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::__moveSeats(const BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat) const
{
    return board.getPawnMoveSeats(fseat);
}
}