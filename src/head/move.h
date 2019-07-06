#ifndef MOVE_H
#define MOVE_H
// 中国象棋棋盘布局类型 by-cjp

#include <memory>
#include <string>
#include <vector>

enum class RecFormat {
    XQF,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC,
    BIN,
    JSON
};

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {
class Seat;
}

namespace BoardSpace {
class Board;
}

namespace MoveSpace {

// 着法节点类
class Move : public std::enable_shared_from_this<Move> {
public:
    Move() = default;

    const std::shared_ptr<SeatSpace::Seat>& fseat() const { return ftseat_.first; }
    const std::shared_ptr<SeatSpace::Seat>& tseat() const { return ftseat_.second; }
    const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
    const std::shared_ptr<Move>& next() const { return next_; }
    const std::shared_ptr<Move>& other() const { return other_; }
    const std::shared_ptr<Move> prev() const { return prev_.lock(); }

    int frowcol() const;
    int trowcol() const;
    const std::wstring iccs() const;
    const std::wstring zh(const std::shared_ptr<BoardSpace::Board>& board) const;

    const std::wstring& remark() const { return remark_; }
    int nextNo() const { return nextNo_; }
    int otherNo() const { return otherNo_; }
    int CC_ColNo() const { return CC_ColNo_; }

    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
    void reset(std::shared_ptr<SeatSpace::Seat>& fseat,
        std::shared_ptr<SeatSpace::Seat>& tseat, std::wstring remark = L"");
    void reset(const std::shared_ptr<BoardSpace::Board>& board,
        int frowcol, int trowcol, std::wstring remark = L"");
    void reset(const std::shared_ptr<BoardSpace::Board>& board, const std::wstring& str,
        RecFormat fmt, std::wstring remark = L"");
    std::vector<std::shared_ptr<Move>> getPrevMoves();
    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();

    void setFTSeat(std::pair<std::shared_ptr<SeatSpace::Seat>,
        std::shared_ptr<SeatSpace::Seat>>
            ftseat) { ftseat_ = ftseat; }
    void setRemark(std::wstring remark) { remark_ = remark; }
    void setNextNo(int nextNo) { nextNo_ = nextNo; }
    void setOtherNo(int otherNo) { otherNo_ = otherNo; }
    void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

    const std::wstring toString(const std::shared_ptr<BoardSpace::Board>& board) const;

private:
    std::pair<std::shared_ptr<SeatSpace::Seat>, std::shared_ptr<SeatSpace::Seat>> ftseat_{};
    std::wstring remark_{}; // 注释

    std::shared_ptr<PieceSpace::Piece> eatPie_{};
    std::shared_ptr<Move> next_{};
    std::shared_ptr<Move> other_{};
    std::weak_ptr<Move> prev_{};

    int nextNo_{ 0 }; // 着法深度
    int otherNo_{ 0 }; // 变着广度
    int CC_ColNo_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
};
}

#endif