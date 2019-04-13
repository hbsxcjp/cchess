#ifndef MOVE_H
#define MOVE_H

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

    const std::shared_ptr<Move>& setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat, const std::shared_ptr<SeatSpace::Seat>& tseat);
    const std::shared_ptr<Move>& setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>& seats);

    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
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

    int frowcol_{};
    int trowcol_{};
    std::wstring iccs_{}; // 着法数字字母描述
    std::wstring zh_{}; // 着法中文描述
    std::wstring remark_{}; // 注释
    int n_{ 0 }; // 着法深度
    int o_{ 0 }; // 变着广度
    int CC_Col_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
};

class RootMove : public Move {
public:
    using Move::Move;
    void read(std::ifstream& ifs, RecFormat fmt, const BoardSpace::Board& board);
    void write(std::ofstream& ofs, RecFormat fmt) const;
    void setMoves(RecFormat fmt, const BoardSpace::Board& board);
    const std::wstring moveInfo() const;
    
    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

private:
    void readXQF(std::ifstream& ifs);
    void writeXQF(std::ofstream& ofs) const;
    const std::wstring getMoveStr(std::ifstream& ifs) const;
    void readPGN_ICCSZH(std::ifstream& ifs, RecFormat fmt);
    void writePGN_ICCSZH(std::ofstream& ofs, RecFormat fmt) const;
    void readPGN_CC(std::ifstream& ifs);
    void writePGN_CC(std::ofstream& ofs) const;
    void readBIN(std::ifstream& ifs);
    void writeBIN(std::ofstream& ofs) const;
    void readJSON(std::ifstream& ifs);
    void writeJSON(std::ofstream& ofs) const;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};
}