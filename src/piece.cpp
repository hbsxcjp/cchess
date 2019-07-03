#include "piece.h"
#include "board.h"
#include "seat.h"
#include <algorithm>
#include <cassert>
//#include <iomanip>
#include <sstream>
#include <string>

using namespace SeatSpace;
using namespace BoardSpace;
namespace PieceSpace {

Piece::Piece(const wchar_t ch)
    : ch_{ ch }
    , name_{ PieceManager::getName(ch_) }
    , color_{ PieceManager::getColor(ch_) }
{
}

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << (color() == PieceColor::RED ? L'+' : L'*') << ch() << PieceManager::getPrintName(*this); //<< std::boolalpha
    return wss.str();
}
const std::vector<std::shared_ptr<Seat>>
Piece::moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const
{
    return __moveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Piece::__putSeats(const Board& board) const
{
    return SeatManager::getAllSeats(board);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
King::__putSeats(const Board& board) const
{
    return SeatManager::getKingSeats(board, *this);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
King::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getKingMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Advisor::__putSeats(const Board& board) const
{
    return SeatManager::getAdvisorSeats(board, *this);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Advisor::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getAdvisorMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Bishop::__putSeats(const Board& board) const
{
    return SeatManager::getBishopSeats(board, *this);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Bishop::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getBishopMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Knight::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getKnightMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Rook::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getRookMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Cannon::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getCannonMoveSeats(board, fseat);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Pawn::__putSeats(const Board& board) const
{
    return SeatManager::getPawnSeats(board, *this);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>>
Pawn::__moveSeats(const Board& board, SeatSpace::Seat& fseat) const
{
    return SeatManager::getPawnMoveSeats(board, fseat);
}

Pieces::Pieces()
    : allPieces_{ PieceManager::createPieces() }
{
}

const std::shared_ptr<Piece>&
Pieces::getOtherPiece(const std::shared_ptr<Piece>& piece) const
{
    if (!piece)
        return piece;
    return allPieces_.at(
        (std::distance(allPieces_.begin(),
             std::find(allPieces_.begin(), allPieces_.end(), piece))
            + allPieces_.size() / 2)
        % allPieces_.size());
}

const std::vector<std::shared_ptr<Piece>>
Pieces::getBoardPieces(const std::wstring& pieceChars) const
{
    std::vector<std::shared_ptr<Piece>> pieces(pieceChars.size());
    std::vector<bool> used(allPieces_.size(), false);
    int pieceIndex{ 0 }, allPiecesSize = used.size();
    std::for_each(pieceChars.begin(), pieceChars.end(),
        [&](const wchar_t ch) {
            if (ch != PieceManager::nullChar())
                for (int index = 0; index < allPiecesSize; ++index)
                    if (allPieces_[index]->ch() == ch && !used[index]) {
                        pieces[pieceIndex] = allPieces_[index];
                        used[index] = true;
                        break;
                    }
            ++pieceIndex;
        });
    return pieces;
}

const std::wstring Pieces::toString() const
{
    std::wstringstream wss{};
    std::for_each(allPieces_.begin(), allPieces_.end(),
        [&](const std::shared_ptr<Piece>& piece) {
            wss << piece->toString() << L' ';
        });
    return wss.str();
}

const std::vector<std::shared_ptr<Piece>> PieceManager::createPieces()
{
    //L"KAABBNNRRCCPPPPPkaabbnnrrccppppp"
    return std::vector<std::shared_ptr<Piece>>{
        std::make_shared<King>(chStr_.at(0)),
        std::make_shared<Advisor>(chStr_.at(1)),
        std::make_shared<Advisor>(chStr_.at(1)),
        std::make_shared<Bishop>(chStr_.at(2)),
        std::make_shared<Bishop>(chStr_.at(2)),
        std::make_shared<Knight>(chStr_.at(3)),
        std::make_shared<Knight>(chStr_.at(3)),
        std::make_shared<Rook>(chStr_.at(4)),
        std::make_shared<Rook>(chStr_.at(4)),
        std::make_shared<Cannon>(chStr_.at(5)),
        std::make_shared<Cannon>(chStr_.at(5)),
        std::make_shared<Pawn>(chStr_.at(6)),
        std::make_shared<Pawn>(chStr_.at(6)),
        std::make_shared<Pawn>(chStr_.at(6)),
        std::make_shared<Pawn>(chStr_.at(6)),
        std::make_shared<Pawn>(chStr_.at(6)),
        std::make_shared<King>(chStr_.at(7)),
        std::make_shared<Advisor>(chStr_.at(8)),
        std::make_shared<Advisor>(chStr_.at(8)),
        std::make_shared<Bishop>(chStr_.at(9)),
        std::make_shared<Bishop>(chStr_.at(9)),
        std::make_shared<Knight>(chStr_.at(10)),
        std::make_shared<Knight>(chStr_.at(10)),
        std::make_shared<Rook>(chStr_.at(11)),
        std::make_shared<Rook>(chStr_.at(11)),
        std::make_shared<Cannon>(chStr_.at(12)),
        std::make_shared<Cannon>(chStr_.at(12)),
        std::make_shared<Pawn>(chStr_.at(13)),
        std::make_shared<Pawn>(chStr_.at(13)),
        std::make_shared<Pawn>(chStr_.at(13)),
        std::make_shared<Pawn>(chStr_.at(13)),
        std::make_shared<Pawn>(chStr_.at(13))
    };
}

const wchar_t PieceManager::getName(const wchar_t ch)
{
    const std::map<int, int> chIndex_nameIndex{
        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    };
    return nameChars_.at(chIndex_nameIndex.at(chStr_.find(ch)));
}

const wchar_t PieceManager::getPrintName(const Piece& piece)
{
    const std::map<wchar_t, wchar_t> rcpName{ { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    const wchar_t name{ piece.name() };
    return (piece.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end()) ? rcpName.at(name) : name;
}

const PieceColor PieceManager::getColor(const wchar_t ch)
{
    return islower(ch) ? PieceColor::BLACK : PieceColor::RED;
}

const PieceColor PieceManager::getColorFromZh(const wchar_t numZh)
{
    return numChars_.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const int PieceManager::getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index = __getPreChars(seatsLen).find(preChar);
    return isBottom ? seatsLen - 1 - index : index;
}

const wchar_t PieceManager::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return __getPreChars(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

const int PieceManager::getCol(bool isBottom, const int num)
{
    return isBottom ? SeatManager::ColNum() - num : num - 1;
}

const wchar_t PieceManager::getColChar(const PieceColor color, bool isBottom, const int col)
{
    return numChars_.at(color).at(isBottom ? SeatManager::ColNum() - col - 1 : col);
}

const std::wstring PieceManager::nameChars_{ L"帅将仕士相象马车炮兵卒" };
const std::wstring PieceManager::movChars_{ L"退平进" };
const std::map<PieceColor, std::wstring> PieceManager::numChars_{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
const std::wstring PieceManager::chStr_{ L"KABNRCPkabnrcp" };
const std::wstring PieceManager::ICCSChars_{ L"abcdefghi" };
}