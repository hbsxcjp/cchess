#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

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
enum class RecFormat {
    XQF,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC,
    BIN,
    JSON
};

namespace InstanceSpace {

class Instance {
    class Move;

public:
    Instance();
    Instance(const std::string& infilename);
    void read(const std::string& infilename);
    void write(const std::string& outfilename);

    void go();
    void back();
    void backTo(const std::shared_ptr<Move>& move);
    void goOther();
    void goInc(int inc);
    void changeSide(ChangeType ct);

    const int getMovCount() const { return movCount_; }
    const int getRemCount() const { return remCount_; }
    const int getRemLenMax() const { return remLenMax_; }
    const int getMaxRow() const { return maxRow_; }
    const int getMaxCol() const { return maxCol_; }

    const std::wstring& remark() const;
    const std::wstring toString();
    const std::wstring test();

private:
    void __reset();
    void __readXQF(std::istream& is);

    void __readBIN(std::istream& is);
    void __writeBIN(std::ostream& os) const;
    void __readJSON(std::istream& is);
    void __writeJSON(std::ostream& os) const;
    void __readInfo_PGN(std::wistream& wis);
    void __writeInfo_PGN(std::wostream& wos) const;
    void __readMove_PGN_ICCSZH(std::wistream& wis, RecFormat fmt);
    void __writeMove_PGN_ICCSZH(std::wostream& wos, RecFormat fmt) const;
    void __readMove_PGN_CC(std::wistream& wis);
    void __writeMove_PGN_CC(std::wostream& wos) const;

    void __setMoveFromRowcol(const std::shared_ptr<Move>& move,
        int frowcol, int trowcol, const std::wstring& remark = L"") const;
    void __setMoveFromStr(const std::shared_ptr<Move>& move,
        const std::wstring& str, RecFormat fmt, const std::wstring& remark = L"") const;
    void __setMoveZhStrAndNums();
    void __setFEN(const std::wstring& pieceChars, PieceColor color);

    const std::wstring __pieceChars() const;
    const std::wstring __moveInfo() const;

    std::map<std::wstring, std::wstring> info_{};
    std::shared_ptr<BoardSpace::Board> board_{};
    std::shared_ptr<Move> rootMove_{}, currentMove_{ rootMove_ };
    int movCount_{ 0 }, remCount_{ 0 }, remLenMax_{ 0 }, maxRow_{ 0 }, maxCol_{ 0 };

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
        const std::wstring zh() const { return zhStr_; }
        const std::wstring& remark() const { return remark_; }
        const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
        const std::shared_ptr<Move>& next() const { return next_; }
        const std::shared_ptr<Move>& other() const { return other_; }
        const std::shared_ptr<Move> prev() const { return prev_.lock(); }

        void done();
        void undo() const;
        std::vector<std::shared_ptr<Move>> getPrevMoves();
        void cutNext() { next_ = nullptr; }
        void cutOther() { other_ && (other_ = other_->other_); }
        void setPrev(std::weak_ptr<Move> prev) { prev_ = prev; }
        void setFTSeat(const std::shared_ptr<SeatSpace::Seat>& fseat,
            const std::shared_ptr<SeatSpace::Seat>& tseat)
        {
            fseat_ = fseat;
            tseat_ = tseat;
        }
        void setZhStr(const std::wstring& zhStr) { zhStr_ = zhStr; }
        void setRemark(const std::wstring& remark) { remark_ = remark; }
        const std::wstring toString() const;

        int nextNo() const { return nextNo_; }
        int otherNo() const { return otherNo_; }
        int CC_ColNo() const { return CC_ColNo_; }
        void setNextNo(int nextNo) { nextNo_ = nextNo; }
        void setOtherNo(int otherNo) { otherNo_ = otherNo; }
        void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

    private:
        std::shared_ptr<SeatSpace::Seat> fseat_{}, tseat_{};
        std::wstring zhStr_{}, remark_{}; // 注释

        std::shared_ptr<PieceSpace::Piece> eatPie_{};
        std::shared_ptr<Move> next_{}, other_{};
        std::weak_ptr<Move> prev_{};

        int nextNo_{ 0 }, otherNo_{ 0 }, CC_ColNo_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
    };
};

const std::wstring getWString(std::wistream& wis);
const std::wstring pieCharsToFEN(const std::wstring& pieceChars); // 便利函数，下同
const std::wstring FENTopieChars(const std::wstring& fen);
const std::string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
}

#endif