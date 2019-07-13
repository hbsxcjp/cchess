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

namespace MoveSpace {
class Move;
}

enum class PieceColor;
enum class ChangeType;
enum class RecFormat;

namespace InstanceSpace {

class Instance {
public:
    Instance();
    Instance(const std::string& infilename);

    void go();
    void back();
    void backTo(const std::shared_ptr<MoveSpace::Move>& move);
    void goOther();
    void goInc(int inc);
    void changeSide(ChangeType ct);

    void read(const std::string& infilename);
    void write(const std::string& outfilename);

    const std::wstring& remark() const;
    const std::wstring toString() const;
    const std::wstring test();
    
    const int getMovCount() const { return movCount_; }
    const int getRemCount() const { return remCount_; }
    const int getRemLenMax() const { return remLenMax_; }
    const int getMaxRow() const { return maxRow_; }
    const int getMaxCol() const { return maxCol_; }

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
    const std::wstring __getWString(std::wistream& wis) const;

    void __setMoveFromRowcol(const std::shared_ptr<MoveSpace::Move>& move,
        int frowcol, int trowcol, const std::wstring& remark = L"") const;
    void __setMoveFromStr(const std::shared_ptr<MoveSpace::Move>& move,
        const std::wstring& str, RecFormat fmt, const std::wstring& remark = L"") const;
    void __setMoveNums();
    void __setFEN(const std::wstring& pieceChars, PieceColor color);

    const std::wstring __pieceChars() const;
    const std::wstring __moveInfo() const;

    std::map<std::wstring, std::wstring> info_{};
    std::shared_ptr<MoveSpace::Move> root_{};
    std::shared_ptr<BoardSpace::Board> board_{};
    std::shared_ptr<MoveSpace::Move> currentMove_{ root_ };

    int movCount_{ 0 }; //着法数量
    int remCount_{ 0 }; //注解数量
    int remLenMax_{ 0 }; //注解最大长度
    int maxRow_{ 0 }; //# 存储最大着法深度
    int maxCol_{ 0 }; //# 存储视图最大列数
};

const std::wstring pieCharsToFEN(const std::wstring& pieceChars); // 便利函数，下同
const std::wstring FENTopieChars(const std::wstring& fen);
const std::string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
}

#endif