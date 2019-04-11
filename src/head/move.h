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
    const std::shared_ptr<RootMove> read(std::ifstream& ifs, RecFormat fmt) const;
    void write(std::ofstream& ofs, RecFormat fmt) const;
    //void setMoves(RecFormat fmt);
private:
    std::shared_ptr<MoveRecord>& getMoveRecord(RecFormat fmt);

    std::shared_ptr<MoveRecord> moveRecord_{};
    const std::shared_ptr<RootMove> rootMove_{};
}

class MoveRecord {
public:
    MoveRecord() = default;
    virtual ~MoveRecord() = default;

    virtual bool is(RecFormat fmt) const = 0;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const = 0;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const = 0;

protected:
    const std::wstring __readInfo_getMoveStr(std::ifstream& ifs);
    void __readMove_ICCSZH(const std::wstring& moveStr, RecFormat fmt);
    void __readMove_CC(const std::wstring& moveStr);
    const std::wstring __getPGNInfo() const;
    const std::wstring __getPGNTxt_ICCSZH(RecFormat fmt) const;
    const std::wstring __getPGNTxt_CC() const;
};

class XQFMoveRecord : public MoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};

class PGNMoveRecord : public MoveRecord {
protected:
    using MoveRecord::MoveRecord;
    std::shared_ptr<RootMove> read_ICCSZH(std::ifstream& ifs, RecFormat fmt) const;
    void write_ICCSZH(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove, RecFormat fmt) const;
};

class PGN_ICCSMoveRecord : public PGNMoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};

class PGN_ZHMoveRecord : public PGNMoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};

class PGN_CCMoveRecord : public MoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};

class BINMoveRecord : public MoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};

class JSONMoveRecord : public MoveRecord {
public:
    using MoveRecord::MoveRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::shared_ptr<RootMove> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, const std::shared_ptr<RootMove>& rootMove) const;
};
}