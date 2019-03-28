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
    wss << std::boolalpha << std::setw(3) << static_cast<int>(color()) << std::setw(5) << ch() << std::setw(5) << name();
    return wss.str();
}

std::vector<std::shared_ptr<SeatSpace::Seat>> King::getSeats(BoardSpace::Board& board)
{
    return board.getKingSeats(color());
}

std::vector<std::shared_ptr<SeatSpace::Seat>> King::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getKingMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::getSeats(BoardSpace::Board& board)
{
    return board.getAdvisorSeats(color());
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getAdvsiorMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::getSeats(BoardSpace::Board& board)
{
    return board.getBishopSeats(color());
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getBishopMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}


std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getKnightMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getRookMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getCannonMoveSeats(fseat);
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::getSeats(BoardSpace::Board& board)
{
    return board.getPawnSeats(color());
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getPawnMoveSeats(fseat);
}
}