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

namespace BoardSpace {
class Board;
}

namespace MoveSpace {

// 着法节点类
class Move : public std::enable_shared_from_this<Move> {
public:
    Move() = default;

    const std::shared_ptr<SeatSpace::Seat>& fseat() const { return fseat_; }
    const std::shared_ptr<SeatSpace::Seat>& tseat() const { return tseat_; }
    const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
    const std::shared_ptr<Move>& next() const { return next_; }
    const std::shared_ptr<Move>& other() const { return other_; }
    const std::shared_ptr<Move> prev() const { return prev_.lock(); }
    //int frowcol() const;
    int frowcol() const { return frowcol_; }
    //int trowcol() const;
    int trowcol() const { return trowcol_; }
    const std::wstring& iccs() const { return iccs_; }
    const std::wstring& zh() const { return zh_; }
    const std::wstring& remark() const { return remark_; }
    int nextNo() const { return nextNo_; }
    int otherNo() const { return otherNo_; }
    int CC_ColNo() const { return CC_ColNo_; }

    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
    std::vector<std::shared_ptr<Move>> getPrevMoves();
    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();

    void setIccs(std::wstring iccs) { iccs_ = iccs; }
    void setZh(std::wstring zh) { zh_ = zh; }
    void setFrowcol(int frowcol) { frowcol_ = frowcol; }
    void setTrowcol(int trowcol) { trowcol_ = trowcol; }
    void setRemark(std::wstring remark) { remark_ = remark; }
    void setNextNo(int nextNo) { nextNo_ = nextNo; }
    void setOtherNo(int otherNo) { otherNo_ = otherNo; }
    void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

    void setFromRowcols(const std::shared_ptr<BoardSpace::Board>& board);
    void setFromIccs(const std::shared_ptr<BoardSpace::Board>& board);
    void setFromZh(const std::shared_ptr<BoardSpace::Board>& board);

    const std::wstring toString() const;

private:
    void __setRowCols();
    void __setIccs();
    void __setZh(const std::shared_ptr<BoardSpace::Board>& board);

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