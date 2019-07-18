#ifndef PIECE_H
#define PIECE_H

#include <map>
#include <memory>
#include <vector>

namespace SeatSpace {
class Seat;
}

namespace BoardSpace {
class Board;
}

enum class PieceColor {
    RED,
    BLACK
};

namespace PieceSpace {
// 棋子类
class Piece {
public:
    explicit Piece(const wchar_t ch);

    const wchar_t ch() const { return ch_; }
    const wchar_t name() const { return name_; }
    const PieceColor color() const { return color_; }
    const std::wstring toString() const;

    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    putSeats(const BoardSpace::Board& board) const { return __putSeats(board); }
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;

    virtual ~Piece() = default;

private:
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __putSeats(const BoardSpace::Board& board) const;
    virtual const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const = 0;
    const wchar_t ch_, name_;
    const PieceColor color_;
};

class King : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __putSeats(const BoardSpace::Board& board) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Advisor : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __putSeats(const BoardSpace::Board& board) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Bishop : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __putSeats(const BoardSpace::Board& board) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Knight : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Rook : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Cannon : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Pawn : public Piece {
public:
    using Piece::Piece;

private:
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __putSeats(const BoardSpace::Board& board) const;
    const std::vector<std::shared_ptr<SeatSpace::Seat>>
    __moveSeats(const BoardSpace::Board& board, SeatSpace::Seat& fseat) const;
};

class Pieces {
public:
    Pieces();

    const std::shared_ptr<Piece>&
    getOtherPiece(const std::shared_ptr<Piece>& piece) const;
    const std::vector<std::shared_ptr<Piece>>
    getBoardPieces(const std::wstring& pieceChars) const;

    const std::wstring toString() const;

private:
    const std::vector<std::shared_ptr<Piece>> allPieces_;
};

class PieceManager {
public:
    static const std::vector<std::shared_ptr<Piece>> createPieces();

    static const std::wstring getZhChars()
    {
        return (preChars_ + nameChars_ + movChars_
            + numChars_.at(PieceColor::RED) + numChars_.at(PieceColor::BLACK));
    }

    static const std::wstring getICCSChars()
    {
        return std::to_wstring(1234567890) + ICCSChars_;
    }

    static const std::wstring getFENStr() { return FENStr_; }

    static const int getRowFromICCSChar(const wchar_t ch) { return ch - '0'; } // 0:48
    static const int getColFromICCSChar(const wchar_t ch) { return ICCSChars_.find(ch); }
    static const wchar_t getColICCSChar(const int col) { return ICCSChars_.at(col); }

    static const wchar_t getName(const wchar_t ch);
    static const wchar_t getPrintName(const Piece& piece);
    static const PieceColor getColor(const wchar_t ch);
    static const PieceColor getColorFromZh(const wchar_t numZh);
    static const int getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar);
    static const wchar_t getIndexChar(const int seatsLen, const bool isBottom, const int index);

    static const wchar_t nullChar() { return nullChar_; };

    static const int getMovNum(const bool isBottom, const wchar_t movChar)
    {
        return (static_cast<int>(movChars_.find(movChar)) - 1) * (isBottom ? 1 : -1);
    }

    static const wchar_t getMovChar(const bool isSameRow, bool isBottom, bool isLowToUp)
    {
        return movChars_.at(isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0));
    }

    static const int getNum(const PieceColor color, const wchar_t numChar)
    {
        return static_cast<int>(numChars_.at(color).find(numChar)) + 1;
    }

    static const wchar_t getNumChar(const PieceColor color, const int num)
    {
        return numChars_.at(color).at(num - 1);
    }

    static const int getCol(bool isBottom, const int num);
    static const wchar_t getColChar(const PieceColor color, bool isBottom, const int col);

    static const bool isKing(const wchar_t name)
    {
        return nameChars_.substr(0, 2).find(name) != std::wstring::npos;
    }

    static const bool isAdvBish(const wchar_t name)
    {
        return nameChars_.substr(2, 4).find(name) != std::wstring::npos;
    }

    static const bool isStronge(const wchar_t name)
    {
        return nameChars_.substr(6, 5).find(name) != std::wstring::npos;
    }

    static const bool isLineMove(const wchar_t name)
    {
        return isKing(name) || nameChars_.substr(7, 4).find(name) != std::wstring::npos;
    }

    static const bool isPawn(const wchar_t name)
    {
        return nameChars_.substr(nameChars_.size() - 2, 2).find(name) != std::wstring::npos;
    }

    static const bool isPiece(const wchar_t name)
    {
        return nameChars_.find(name) != std::wstring::npos;
    };

private:
    static const std::wstring __getPreChars(const int length)
    {
        return (length == 2 ? (std::wstring{ preChars_ }).erase(1, 1) //L"前后"
                            : (length == 3 ? preChars_
                                           //L"一二三四五");
                                           : numChars_.at(PieceColor::RED).substr(0, 5)));
    }

    static const std::wstring chChars_;
    static const std::wstring preChars_;
    static const std::wstring nameChars_;
    static const std::wstring movChars_;
    static const std::map<PieceColor, std::wstring> numChars_;
    static const wchar_t nullChar_{ L'_' };
    static const std::wstring ICCSChars_;
    static const std::wstring FENStr_;
};
}

#endif