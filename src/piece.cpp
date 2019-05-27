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
    , name_{ CharManager::getName(ch_) }
    , color_{ CharManager::getColor(ch_) }
{
}

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << (color() == PieceColor::RED ? L'-' : L'+') << ch_ << name(); //<< std::boolalpha
    return wss.str();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Piece::__difColorSeats(const BoardSpace::Board& board,
    const std::vector<std::pair<int, int>>& rowcols) const
{
    auto seats = board.getSeats(rowcols);
    auto pos = std::remove_if(seats.begin(), seats.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        return !seat->isDifColor(color());
    });
    return std::vector<std::shared_ptr<SeatSpace::Seat>>{ seats.begin(), pos };
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Piece::__overDifColorSeats(const BoardSpace::Board& board,
    const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& obs_MoveRowcols) const
{
    std::vector<std::pair<int, int>> rowcols{};
    std::for_each(obs_MoveRowcols.begin(), obs_MoveRowcols.end(),
        [&](const std::pair<std::pair<int, int>, std::pair<int, int>>& obs_MoveRowcol) {
            if (!board.getSeat(obs_MoveRowcol.first)->piece())
                rowcols.push_back(obs_MoveRowcol.second);
        });
    return __difColorSeats(board, rowcols);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::__putSeats(const Board& board) const
{
    return board.getSeats(RowcolManager::getKingRowcols(board.isBottomSide(color())));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> King::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    return __difColorSeats(board, RowcolManager::getKingMoveRowcols(board.isBottomSide(color()), fseat.row(), fseat.col()));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::__putSeats(const Board& board) const
{
    return board.getSeats(RowcolManager::getAdvisorRowcols(board.isBottomSide(color())));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Advisor::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    return __difColorSeats(board, RowcolManager::getAdvisorMoveRowcols(board.isBottomSide(color()), fseat.row(), fseat.col()));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::__putSeats(const Board& board) const
{
    return board.getSeats(RowcolManager::getBishopRowcols(board.isBottomSide(color())));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Bishop::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    return __overDifColorSeats(board, RowcolManager::getBishopObs_MoveRowcols(board.isBottomSide(color()), fseat.row(), fseat.col()));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::__putSeats(const Board& board) const
{
    return board.getSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Knight::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    return __overDifColorSeats(board, RowcolManager::getKnightObs_MoveRowcols(fseat.row(), fseat.col()));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::__putSeats(const Board& board) const
{
    return board.getSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Rook::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    std::vector<std::pair<int, int>> moveRowcols{};
    for (auto& rowcols : RowcolManager::getRookCannonMoveRowcol_Lines(fseat.row(), fseat.col()))
        for (auto& rowcol : rowcols) {
            moveRowcols.push_back(rowcol);
            if (board.getSeat(rowcol)->piece())
                break;
        }
    return __difColorSeats(board, moveRowcols);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::__putSeats(const Board& board) const
{
    return board.getSeats();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Cannon::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    std::vector<std::pair<int, int>> moveRowcols{};
    for (auto& rowcols : RowcolManager::getRookCannonMoveRowcol_Lines(fseat.row(), fseat.col())) {
        bool skip = false;
        for (auto& rowcol : rowcols) {
            bool isPiece{ bool(board.getSeat(rowcol)->piece()) };
            if (!skip) {
                if (!isPiece)
                    moveRowcols.push_back(rowcol);
                else
                    skip = true;
            } else if (isPiece) {
                moveRowcols.push_back(rowcol);
                break;
            }
        }
    }
    return __difColorSeats(board, moveRowcols);
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::__putSeats(const Board& board) const
{
    return board.getSeats(RowcolManager::getPawnRowcols(board.isBottomSide(color())));
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Pawn::__moveSeats(const Board& board,
    const SeatSpace::Seat& fseat) const
{
    return __difColorSeats(board, RowcolManager::getPawnMoveRowcols(board.isBottomSide(color()), fseat.row(), fseat.col()));
}

const wchar_t CharManager::getName(const wchar_t ch)
{
    const std::map<int, int> chIndex_nameIndex{
        { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
        { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
    };
    return nameChars_.at(chIndex_nameIndex.at(chStr_.find(ch)));
}

const wchar_t CharManager::getPrintName(const PieceSpace::Piece& piece)
{
    const std::map<wchar_t, wchar_t> rcpName{ { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    const wchar_t name{ piece.name() };
    return (piece.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end()) ? rcpName.at(name) : name;
}

const PieceColor CharManager::getColor(const wchar_t ch) { return islower(ch) ? PieceColor::BLACK : PieceColor::RED; }

const PieceColor CharManager::getColorFromZh(const wchar_t numZh)
{
    return numChars_.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK;
}

const int CharManager::getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index{ static_cast<int>(__getPreChars(seatsLen).find(preChar)) };
    return isBottom ? seatsLen - 1 - index : index;
}

const wchar_t CharManager::getIndexChar(const int seatsLen, const bool isBottom, const int index)
{
    return __getPreChars(seatsLen).at(isBottom ? seatsLen - 1 - index : index);
}

const int CharManager::getCol(bool isBottom, const int num)
{
    return isBottom ? RowcolManager::ColNum() - num : num - 1;
}

const wchar_t CharManager::getColChar(const PieceColor color, bool isBottom, const int col)
{
    return numChars_.at(color).at(isBottom ? RowcolManager::ColNum() - col - 1 : col);
}

const std::wstring CharManager::nameChars_{ L"帅将仕士相象马车炮兵卒" };
const std::wstring CharManager::movChars_{ L"退平进" };
const std::map<PieceColor, std::wstring> CharManager::numChars_{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
const std::wstring CharManager::chStr_{ L"KABNRCPkabnrcp" };

const std::vector<std::shared_ptr<PieceSpace::Piece>> creatPieces()
{
    //L"KAABBNNRRCCPPPPPkaabbnnrrccppppp"
    return std::vector<std::shared_ptr<PieceSpace::Piece>>{
        std::make_shared<PieceSpace::King>(L'K'),
        std::make_shared<PieceSpace::Advisor>(L'A'),
        std::make_shared<PieceSpace::Advisor>(L'A'),
        std::make_shared<PieceSpace::Bishop>(L'B'),
        std::make_shared<PieceSpace::Bishop>(L'B'),
        std::make_shared<PieceSpace::Knight>(L'N'),
        std::make_shared<PieceSpace::Knight>(L'N'),
        std::make_shared<PieceSpace::Rook>(L'R'),
        std::make_shared<PieceSpace::Rook>(L'R'),
        std::make_shared<PieceSpace::Cannon>(L'C'),
        std::make_shared<PieceSpace::Cannon>(L'C'),
        std::make_shared<PieceSpace::Pawn>(L'P'),
        std::make_shared<PieceSpace::Pawn>(L'P'),
        std::make_shared<PieceSpace::Pawn>(L'P'),
        std::make_shared<PieceSpace::Pawn>(L'P'),
        std::make_shared<PieceSpace::Pawn>(L'P'),
        std::make_shared<PieceSpace::King>(L'k'),
        std::make_shared<PieceSpace::Advisor>(L'a'),
        std::make_shared<PieceSpace::Advisor>(L'a'),
        std::make_shared<PieceSpace::Bishop>(L'b'),
        std::make_shared<PieceSpace::Bishop>(L'b'),
        std::make_shared<PieceSpace::Knight>(L'n'),
        std::make_shared<PieceSpace::Knight>(L'n'),
        std::make_shared<PieceSpace::Rook>(L'r'),
        std::make_shared<PieceSpace::Rook>(L'r'),
        std::make_shared<PieceSpace::Cannon>(L'c'),
        std::make_shared<PieceSpace::Cannon>(L'c'),
        std::make_shared<PieceSpace::Pawn>(L'p'),
        std::make_shared<PieceSpace::Pawn>(L'p'),
        std::make_shared<PieceSpace::Pawn>(L'p'),
        std::make_shared<PieceSpace::Pawn>(L'p'),
        std::make_shared<PieceSpace::Pawn>(L'p')
    };
}
}