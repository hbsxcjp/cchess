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

    const std::shared_ptr<MoveSpace::Move> read(const std::string& infilename, RecFormat fmt);
    void write(const std::string& outfilename, RecFormat fmt) const;
    //void setMoves(const RecFormat fmt);

    const std::shared_ptr<Move>& setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat, const std::shared_ptr<SeatSpace::Seat>& tseat);
    const std::shared_ptr<Move>& setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>& seats);

    void cutNext() { next_ = nullptr; }
    void cutOther() { other_ && (other_ = other_->other_); }
    const std::shared_ptr<Move>& addNext();
    const std::shared_ptr<Move>& addOther();
    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();
    std::vector<std::shared_ptr<Move>> getPrevMoves();

    const std::wstring moveInfo() const;
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

class MoveRecord {
public:
    MoveRecord() = default;
    virtual ~MoveRecord() = default;

    virtual void read(const std::string& infilename) = 0;
    virtual void write(const std::string& outfilename) const = 0;

protected:
    const std::wstring __readInfo_getMoveStr(const std::string& infilename);
    void __readMove_ICCSZH(const std::wstring& moveStr, const RecFormat fmt);
    void __readMove_CC(const std::wstring& moveStr);
    const std::wstring __getPGNInfo() const;
    const std::wstring __getPGNTxt_ICCSZH(const RecFormat fmt) const;
    const std::wstring __getPGNTxt_CC() const;
};

class XQFMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

class PGN_ICCSMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

class PGN_ZHMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

class PGN_CCMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

class BINMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

class JSONMoveRecord : public MoveRecord {
public:
    virtual void read(const std::string& infilename);
    virtual void write(const std::string& outfilename) const;
};

std::shared_ptr<MoveRecord> getMoveRecord(RecFormat fmt);
}