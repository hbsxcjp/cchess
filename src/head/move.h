#ifndef MOVE_H
#define MOVE_H
// 中国象棋棋盘布局类型 by-cjp

#include <memory>
#include <string>
#include <vector>

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {
class Seat;
}

namespace MoveSpace {

// 着法节点类
class Move : public std::enable_shared_from_this<Move> {
public:
    Move() = default;

    const std::shared_ptr<SeatSpace::Seat>& fseat() const { return fseat_; }
    const std::shared_ptr<SeatSpace::Seat>& tseat() const { return tseat_; }
    const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
    const std::shared_ptr<Move>& setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat, const std::shared_ptr<SeatSpace::Seat>& tseat);
    const std::shared_ptr<Move>& setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>& seats);

    const std::shared_ptr<Move>& next() const { return next_; }
    const std::shared_ptr<Move>& other() const { return other_; }
    const std::shared_ptr<Move> prev() const { return prev_.lock(); }
    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
    std::vector<std::shared_ptr<Move>> getPrevMoves();
    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();

    int frowcol() const { return frowcol_; }
    int trowcol() const { return trowcol_; }
    void setFrowcol(int frowcol) { frowcol_ = frowcol; }
    void setTrowcol(int trowcol) { trowcol_ = trowcol; }
    const std::wstring& iccs() const { return iccs_; }
    const std::wstring& zh() const { return zh_; }
    const std::wstring& remark() const { return remark_; }
    void setIccs(std::wstring iccs) { iccs_ = iccs; }
    void setZh(std::wstring zh) { zh_ = zh; }
    void setRemark(std::wstring remark) { remark_ = remark; }
    int nextNo() const { return nextNo_; }
    int otherNo() const { return otherNo_; }
    int CC_ColNo() const { return CC_ColNo_; }
    void setNextNo(int nextNo) { nextNo_ = nextNo; }
    void setOtherNo(int otherNo) { otherNo_ = otherNo; }
    void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

    const std::wstring toString() const;

private:
    std::shared_ptr<SeatSpace::Seat> fseat_{};
    std::shared_ptr<SeatSpace::Seat> tseat_{};
    std::shared_ptr<PieceSpace::Piece> eatPie_{};
    std::shared_ptr<Move> next_{};
    std::shared_ptr<Move> other_{};
    std::weak_ptr<Move> prev_{};

    int frowcol_{ -1 };
    int trowcol_{ -1 };
    std::wstring iccs_{}; // 着法数字字母描述
    std::wstring zh_{}; // 着法中文描述
    std::wstring remark_{}; // 注释
    int nextNo_{ 1 }; // 着法深度
    int otherNo_{ 0 }; // 变着广度
    int CC_ColNo_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
};
}

#endif