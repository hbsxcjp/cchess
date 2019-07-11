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

    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();

    int frowcol() const;
    int trowcol() const;
    const std::wstring iccs() const;
    const std::shared_ptr<SeatSpace::Seat>& fseat() const { return fseat_; }
    const std::shared_ptr<SeatSpace::Seat>& tseat() const { return tseat_; }
    const std::wstring zh() const { return zh_; }
    const std::wstring& remark() const { return remark_; }
    const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
    const std::shared_ptr<Move>& next() const { return next_; }
    const std::shared_ptr<Move>& other() const { return other_; }
    const std::shared_ptr<Move> prev() const { return prev_.lock(); }

    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }

    void done();
    void undo() const;
    std::vector<std::shared_ptr<Move>> getPrevMoves();

    void setFTSeat(const std::shared_ptr<SeatSpace::Seat>& fseat,
        const std::shared_ptr<SeatSpace::Seat>& tseat)
    {
        fseat_ = fseat;
        tseat_ = tseat;
    }
    void setZh(std::wstring zhStr) { zh_ = zhStr; }
    void setRemark(std::wstring remark) { remark_ = remark; }
    const std::wstring toString() const;

    int nextNo() const { return nextNo_; }
    int otherNo() const { return otherNo_; }
    int CC_ColNo() const { return CC_ColNo_; }
    void setNextNo(int nextNo) { nextNo_ = nextNo; }
    void setOtherNo(int otherNo) { otherNo_ = otherNo; }
    void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

private:
    std::shared_ptr<SeatSpace::Seat> fseat_{};
    std::shared_ptr<SeatSpace::Seat> tseat_{};
    std::wstring zh_{};
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