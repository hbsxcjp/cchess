#include "piece.h"
#include <iomanip>
#include <sstream>
#include <string>
//#include <algorithm>
//#include <iomanip>
//#include <iterator>
//#include <sstream>

namespace SeatSpace {
class Seat;
}

namespace PieceSpace {

const std::vector<std::shared_ptr<SeatSpace::Seat>> Piece::getSeats(const PieceColor bottomColor) const
{
    return __getSeats(bottomColor);
}

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << std::boolalpha;
    wss << std::setw(2) << static_cast<int>(color())
        << std::setw(6) << ch() << std::setw(5) << name() << std::setw(8) << isKing(*this)
        << std::setw(8) << isPawn(*this) << std::setw(8) << isStronge(*this) << std::setw(8) << isLineMove(*this);
    return wss.str();
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

const wchar_t nullChar{ L'_' };
const std::wstring nameChars{ L"帅将仕士相象马车炮兵卒" };
const std::shared_ptr<Piece> nullPiece{ std::make_shared<NullPiece>(nullChar, L'　', PieceColor::BLANK) };
const std::vector<std::shared_ptr<Piece>> creatPieces();

//const PieceColor getOthColor(const PieceColor color) { return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED; }
const bool isKing(const Piece& piece) { return nameChars.substr(0, 2).find(piece.name()) != std::wstring::npos; }
const bool isAdvBish(const Piece& piece) { return nameChars.substr(2, 4).find(piece.name()) != std::wstring::npos; }
const bool isStronge(const Piece& piece) { return nameChars.substr(6, 5).find(piece.name()) != std::wstring::npos; }
const bool isLineMove(const Piece& piece) { return isKing(piece) || nameChars.substr(7, 4).find(piece.name()) != std::wstring::npos; }
const bool isPawn(const Piece& piece) { return nameChars.substr(nameChars.size() - 2, 2).find(piece.name()) != std::wstring::npos; }
const bool isPieceName(const Piece& piece) { return nameChars.find(piece.name()) != std::wstring::npos; }

/*
#include "board_base.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
using namespace std;
using namespace Board_base;

const wchar_t Piece::nullChar{ L'_' };

map<wchar_t, wchar_t> Piece::chNames{
    { L'K', L'帅' }, { L'k', L'将' }, { L'A', L'仕' }, { L'a', L'士' },
    { L'B', L'相' }, { L'b', L'象' }, { L'N', L'马' }, { L'n', L'马' },
    { L'R', L'车' }, { L'r', L'车' }, { L'C', L'炮' }, { L'c', L'炮' },
    { L'P', L'兵' }, { L'p', L'卒' }
};

Piece::Piece(const wchar_t _ch)
    : st{ Board_base::nullSeat }
    , ch{ _ch }
    , name{ Piece::chNames[_ch] }
    , clr{ isalpha(_ch)
            ? (islower(_ch) ? PieceColor::BLACK : PieceColor::RED)
            : PieceColor::BLANK }
{
}

inline const std::vector<std::shared_ptr<Seat>> Piece::getSeats(const PieceColor bottomColor) const { return allSeats; }

const std::vector<std::shared_ptr<Seat>> Piece::filterMoveSeats(const Board& board)
{
    std::vector<std::shared_ptr<Seat>> res{ __moveSeats(board) };
    auto p = remove_if(res.begin(), res.end(), [&](const int seat) {
        return board.getColor(seat) == color();
    });
    return (std::vector<std::shared_ptr<Seat>>{ res.begin(), p });
}

const std::vector<std::shared_ptr<Seat>> Piece::getCanMoveSeats(Board& board)
{
    std::vector<std::shared_ptr<Seat>> res{};
    auto fseat = seat();
    for (auto tseat : filterMoveSeats(board)) {
        auto eatPiece = board.go(fseat, tseat);
        // 移动棋子后，检测是否会被对方将军
        if (!board.isKilled(color()))
            res.push_back(tseat);
        board.back(fseat, tseat, eatPiece);
    }
    return res;
}

inline const std::vector<std::shared_ptr<Seat>> Piece::__moveSeats(const Board& board) const { return allSeats; }

const std::vector<std::shared_ptr<Seat>> Piece::__filterMove_obstruct(const Board& board,
    const std::vector<pair<int, int>>& seat_obss) const
{
    std::vector<std::shared_ptr<Seat>> res{};
    for (auto& seat_obs : seat_obss)
        if (board.isBlank(seat_obs.second))
            res.push_back(seat_obs.first);
    return res;
}

inline const std::vector<std::shared_ptr<Seat>> King::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomKingSeats : topKingSeats; }

inline const std::vector<std::shared_ptr<Seat>> King::__moveSeats(const Board& board) const { return getKingMoveSeats(seat()); }

inline const std::vector<std::shared_ptr<Seat>> Advisor::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomAdvisorSeats : topAdvisorSeats; }

inline const std::vector<std::shared_ptr<Seat>> Advisor::__moveSeats(const Board& board) const { return getAdvisorMoveSeats(seat()); }

inline const std::vector<std::shared_ptr<Seat>> Bishop::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomBishopSeats : topBishopSeats; }

inline const std::vector<std::shared_ptr<Seat>> Bishop::__moveSeats(const Board& board) const { return __filterMove_obstruct(board, getBishopMove_CenSeats(seat())); }

inline const std::vector<std::shared_ptr<Seat>> Knight::__moveSeats(const Board& board) const { return __filterMove_obstruct(board, getKnightMove_LegSeats((seat()))); }

const std::vector<std::shared_ptr<Seat>> Rook::__moveSeats(const Board& board) const
{
    std::vector<std::shared_ptr<Seat>> res{};
    for (auto seatLine : getRookCannonMoveSeat_Lines(seat()))
        for (auto seat : seatLine) {
            res.push_back(seat);
            if (!board.isBlank(seat))
                break;
        }
    return res;
}

const std::vector<std::shared_ptr<Seat>> Cannon::__moveSeats(const Board& board) const
{
    std::vector<std::shared_ptr<Seat>> res{};
    for (auto seatLine : getRookCannonMoveSeat_Lines(seat())) {
        bool skip = false;
        for (auto seat : seatLine)
            if (!skip) {
                if (board.isBlank(seat))
                    res.push_back(seat);
                else
                    skip = true;
            } else if (!board.isBlank(seat)) {
                res.push_back(seat);
                break;
            }
    }
    return res;
}

inline const std::vector<std::shared_ptr<Seat>> Pawn::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomPawnSeats : topPawnSeats; }

inline const std::vector<std::shared_ptr<Seat>> Pawn::__moveSeats(const Board& board) const { return getPawnMoveSeats(board.isBottomSide(color()), seat()); }

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << boolalpha;
    wss << std::setw(4) << std::setw(5) << static_cast<int>(color()) << std::setw(6)
        << wchar() << std::setw(6) << seat() << std::setw(5) << chName() << std::setw(9)
        << isBlank() << std::setw(8) << isKing() << std::setw(8) << isStronge() << L'\n';
    return wss.str();
}
*/
}