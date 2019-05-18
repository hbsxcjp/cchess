#include "piece.h"
#include "board.h"
#include "seat.h"
#include <algorithm>
#include <cassert>
//#include <iomanip>
#include <sstream>
#include <string>

namespace PieceSpace {

Piece::Piece(const wchar_t ch)
    : ch_{ ch }
{
}

inline const wchar_t Piece::name() const { return BoardSpace::PieceCharManager::getName(ch_); }

inline const PieceColor Piece::color() const { return BoardSpace::PieceCharManager::getColor(ch_); }

const PieceKind Piece::kind() const { return BoardSpace::PieceCharManager::getKind(ch_); }

const std::wstring Piece::toString() const
{
    std::wstringstream wss{};
    wss << (color() == PieceColor::RED ? L'-' : L'+') << ch_ << name(); //<< std::boolalpha
    return wss.str();
}
}