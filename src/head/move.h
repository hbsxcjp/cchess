#ifndef MOVE_H
#define MOVE_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace PieceSpace {
class Piece;
}

namespace SeatSpace {
class Seat;
}

namespace InfoSpace {
struct Key;
}

namespace BoardSpace {
class Board;
}

enum class PieceColor;
enum class ChangeType;
enum class RecFormat;

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
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
    void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }

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

    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();
    std::vector<std::shared_ptr<Move>> getPrevMoves();
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

class MoveOwner {
public:
    MoveOwner() = default;

    void read(std::istream& is, RecFormat fmt, const BoardSpace::Board& board, const InfoSpace::Key& key);
    void write(std::ostream& os, RecFormat fmt) const;
    void setMoves(RecFormat fmt, const BoardSpace::Board& board);
    void changeSide(const BoardSpace::Board* board, std::_Mem_fn<const int (BoardSpace::Board::*)(int rowcol) const> changeRowcol);

    const std::wstring& remark() const { return remark_; }
    const std::wstring moveInfo() const;
    const std::wstring toString() const;

    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

private:
    void readXQF(std::istream& is, const InfoSpace::Key& key);
    void writeXQF(std::ostream& os) const;
    const std::wstring __getMoveStr(std::istream& is) const;
    void readPGN_ICCSZH(std::istream& is, RecFormat fmt);
    void writePGN_ICCSZH(std::ostream& os, RecFormat fmt) const;
    void readPGN_CC(std::istream& is);
    void writePGN_CC(std::ostream& os) const;
    void readBIN(std::istream& is);
    void writeBIN(std::ostream& os) const;
    void readJSON(std::istream& is);
    void writeJSON(std::ostream& os) const;

    bool hasMove_{ false };
    std::shared_ptr<Move> rootMove_{};
    std::wstring remark_{}; // 注释
    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};
}

#endif