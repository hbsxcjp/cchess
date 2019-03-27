#include "piece.h"
#include "board.h"
#include "seat.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
//#include <iomanip>
//#include <iterator>
//#include <sstream>

namespace PieceSpace {

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << std::boolalpha;
    wss << std::setw(2) << static_cast<int>(color())
        << std::setw(6) << ch() << std::setw(5) << name() << std::setw(8) << isKing(name_)
        << std::setw(8) << isPawn(name_) << std::setw(8) << isStronge(name_) << std::setw(8) << isLineMove(name_);
    return wss.str();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::getSeats(BoardSpace::Board& board)
{
    return board.getKingSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getKingMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::getSeats(BoardSpace::Board& board)
{
    return board.getAdvisorSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getAdvsiorMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::getSeats(BoardSpace::Board& board)
{
    return board.getBishopSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getBishopMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getKnightMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getRookMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::getSeats(BoardSpace::Board& board)
{
    return board.getAllSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getCannonMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::getSeats(BoardSpace::Board& board)
{
    return board.getPawnSeats(color());
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return board.getPawnMoveSeats(fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> NullPiece::getSeats(BoardSpace::Board& board)
{
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{};
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> NullPiece::moveSeats(BoardSpace::Board& board,
    const std::shared_ptr<SeatSpace::Seat>& fseat)
{
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{};
}


const std::vector<std::shared_ptr<Piece>> creatPieces()
{
    //const map<wchar_t, int> charIndexs{
    //    { L'K', 0 }, { L'k', 1 }, { L'A', 2 }, { L'a', 3 }, { L'B', 4 }, { L'b', 5 },
    //    { L'N', 6 }, { L'n', 6 }, { L'R', 7 }, { L'r', 7 }, { L'C', 8 }, { L'c', 8 }, { L'P', 9 }, { L'p', 10 }
    //};
    //for (auto& ch : std::wstring{ L"KAABBNNRRCCPPPPPkaabbnnrrccppppp" })
    //    pieces.push_back(std::make_shared<Piece>(ch, nameChars[charIndexs.at(ch)],
    //        islower(ch) ? PieceColor::BLACK : PieceColor::RED));
    std::vector<std::shared_ptr<Piece>> pieces{
        std::make_shared<King>(L'K', nameChars.at(0), PieceColor::RED),
        std::make_shared<Advisor>(L'A', nameChars.at(2), PieceColor::RED),
        std::make_shared<Advisor>(L'A', nameChars.at(2), PieceColor::RED),
        std::make_shared<Bishop>(L'B', nameChars.at(4), PieceColor::RED),
        std::make_shared<Bishop>(L'B', nameChars.at(4), PieceColor::RED),
        std::make_shared<Knight>(L'N', nameChars.at(6), PieceColor::RED),
        std::make_shared<Knight>(L'N', nameChars.at(6), PieceColor::RED),
        std::make_shared<Rook>(L'R', nameChars.at(7), PieceColor::RED),
        std::make_shared<Rook>(L'R', nameChars.at(7), PieceColor::RED),
        std::make_shared<Cannon>(L'C', nameChars.at(8), PieceColor::RED),
        std::make_shared<Cannon>(L'C', nameChars.at(8), PieceColor::RED),
        std::make_shared<Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<Pawn>(L'P', nameChars.at(9), PieceColor::RED),
        std::make_shared<King>(L'k', nameChars.at(1), PieceColor::BLACK),
        std::make_shared<Advisor>(L'a', nameChars.at(3), PieceColor::BLACK),
        std::make_shared<Advisor>(L'a', nameChars.at(3), PieceColor::BLACK),
        std::make_shared<Bishop>(L'b', nameChars.at(5), PieceColor::BLACK),
        std::make_shared<Bishop>(L'b', nameChars.at(5), PieceColor::BLACK),
        std::make_shared<Knight>(L'n', nameChars.at(6), PieceColor::BLACK),
        std::make_shared<Knight>(L'n', nameChars.at(6), PieceColor::BLACK),
        std::make_shared<Rook>(L'r', nameChars.at(7), PieceColor::BLACK),
        std::make_shared<Rook>(L'r', nameChars.at(7), PieceColor::BLACK),
        std::make_shared<Cannon>(L'c', nameChars.at(8), PieceColor::BLACK),
        std::make_shared<Cannon>(L'c', nameChars.at(8), PieceColor::BLACK),
        std::make_shared<Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<Pawn>(L'p', nameChars.at(10), PieceColor::BLACK),
        std::make_shared<Pawn>(L'p', nameChars.at(10), PieceColor::BLACK)
    };
    return pieces;
}

//const PieceColor getOthColor(const PieceColor color) { return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED; }
const bool isKing(const wchar_t name) { return nameChars.substr(0, 2).find(name) != std::wstring::npos; }
const bool isAdvBish(const wchar_t name) { return nameChars.substr(2, 4).find(name) != std::wstring::npos; }
const bool isStronge(const wchar_t name) { return nameChars.substr(6, 5).find(name) != std::wstring::npos; }
const bool isLineMove(const wchar_t name) { return isKing(name) || nameChars.substr(7, 4).find(name) != std::wstring::npos; }
const bool isPawn(const wchar_t name) { return nameChars.substr(nameChars.size() - 2, 2).find(name) != std::wstring::npos; }
const bool isPieceName(const wchar_t name) { return nameChars.find(name) != std::wstring::npos; }

const wchar_t nullChar{ L'_' };
const std::wstring nameChars{ L"帅将仕士相象马车炮兵卒" };
const std::shared_ptr<Piece> nullPiece{ std::make_shared<NullPiece>(nullChar, L'　', PieceColor::BLANK) };

/*
map<wchar_t, wchar_t> Piece::chNames{
    { L'K', L'帅' }, { L'k', L'将' }, { L'A', L'仕' }, { L'a', L'士' },
    { L'B', L'相' }, { L'b', L'象' }, { L'N', L'马' }, { L'n', L'马' },
    { L'R', L'车' }, { L'r', L'车' }, { L'C', L'炮' }, { L'c', L'炮' },
    { L'P', L'兵' }, { L'p', L'卒' }
};{
}

*/
}