#include "piece.h"
#include <iomanip>
#include <sstream>
#include <string>
using namespace std;

Piece::Piece(const wchar_t ch)
    : ch_{ ch }
    , name_{ PieceAide::getName(ch) }
    , color_{ PieceAide::getColor(ch) }
{
}

shared_ptr<Piece> Piece::nullPiece{ make_shared<Piece>(PieceAide::getNullChar()) };

const wstring Piece::toString() const
{
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(2) << static_cast<int>(color())
        << setw(6) << ch() << setw(5) << name() << setw(8) << PieceAide::isKing(name())
        << setw(8) << PieceAide::isPawn(name()) << setw(8) << PieceAide::isStronge(name()) << setw(8) << PieceAide::isLineMove(name());
    return wss.str();
}

const map<wchar_t, wchar_t> PieceAide::__charNames{
    { L'K', L'帅' }, { L'k', L'将' }, { L'A', L'仕' }, { L'a', L'士' },
    { L'B', L'相' }, { L'b', L'象' }, { L'N', L'马' }, { L'n', L'马' },
    { L'R', L'车' }, { L'r', L'车' }, { L'C', L'炮' }, { L'c', L'炮' },
    { L'P', L'兵' }, { L'p', L'卒' }, { L'_', L'　' } // L'　'
};

const map<PieceColor, wstring> PieceAide::__numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};

const map<wchar_t, int> PieceAide::__movIndex{ { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
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

inline const vector<int> Piece::getSeats(const PieceColor bottomColor) const { return allSeats; }

const vector<int> Piece::filterMoveSeats(const Board& board)
{
    vector<int> res{ __moveSeats(board) };
    auto p = remove_if(res.begin(), res.end(), [&](const int seat) {
        return board.getColor(seat) == color();
    });
    return (vector<int>{ res.begin(), p });
}

const vector<int> Piece::getCanMoveSeats(Board& board)
{
    vector<int> res{};
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

inline const vector<int> Piece::__moveSeats(const Board& board) const { return allSeats; }

const vector<int> Piece::__filterMove_obstruct(const Board& board,
    const vector<pair<int, int>>& seat_obss) const
{
    vector<int> res{};
    for (auto& seat_obs : seat_obss)
        if (board.isBlank(seat_obs.second))
            res.push_back(seat_obs.first);
    return res;
}

inline const vector<int> King::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomKingSeats : topKingSeats; }

inline const vector<int> King::__moveSeats(const Board& board) const { return getKingMoveSeats(seat()); }

inline const vector<int> Advisor::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomAdvisorSeats : topAdvisorSeats; }

inline const vector<int> Advisor::__moveSeats(const Board& board) const { return getAdvisorMoveSeats(seat()); }

inline const vector<int> Bishop::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomBishopSeats : topBishopSeats; }

inline const vector<int> Bishop::__moveSeats(const Board& board) const { return __filterMove_obstruct(board, getBishopMove_CenSeats(seat())); }

inline const vector<int> Knight::__moveSeats(const Board& board) const { return __filterMove_obstruct(board, getKnightMove_LegSeats((seat()))); }

const vector<int> Rook::__moveSeats(const Board& board) const
{
    vector<int> res{};
    for (auto seatLine : getRookCannonMoveSeat_Lines(seat()))
        for (auto seat : seatLine) {
            res.push_back(seat);
            if (!board.isBlank(seat))
                break;
        }
    return res;
}

const vector<int> Cannon::__moveSeats(const Board& board) const
{
    vector<int> res{};
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

inline const vector<int> Pawn::getSeats(const PieceColor bottomColor) const { return color() == bottomColor ? bottomPawnSeats : topPawnSeats; }

inline const vector<int> Pawn::__moveSeats(const Board& board) const { return getPawnMoveSeats(board.isBottomSide(color()), seat()); }

const wstring Piece::toString() const
{
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(4) << setw(5) << static_cast<int>(color()) << setw(6)
        << wchar() << setw(6) << seat() << setw(5) << chName() << setw(9)
        << isBlank() << setw(8) << isKing() << setw(8) << isStronge() << L'\n';
    return wss.str();
}
*/