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
public:
    Instance();

    const bool isStart() const { return !currentMove_; }
    const bool isLast() const;
    void go();
    void back();
    void goOther();
    void backFirst();
    void goLast();
    void goInc(int inc);
    void changeSide(ChangeType ct);
    const std::wstring toString() const;

    void read(const std::string& infilename);
    void write(const std::string& outfilename);

    const std::wstring& remark() const { return remark_; }
    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

private:
    void readXQF(std::istream& is);
    const std::wstring __getMoveStr(std::istream& is) const;
    void __readInfo_PGN(std::istream& is);
    void __writeInfo_PGN(std::ostream& os) const;
    void __readMove_PGN_ICCSZH(std::istream& is, RecFormat fmt);
    void __writeMove_PGN_ICCSZH(std::ostream& os, RecFormat fmt) const;
    void __readMove_PGN_CC(std::istream& is);
    void __writeMove_PGN_CC(std::ostream& os) const;
    void readBIN(std::istream& is);
    void writeBIN(std::ostream& os) const;
    void readJSON(std::istream& is);
    void writeJSON(std::ostream& os) const;

    void setFEN(const std::wstring& pieceChars, PieceColor color);
    void setFormat(RecFormat fmt);
    const std::wstring pieceChars() const;
    void setMoves(RecFormat fmt);
    const std::wstring moveInfo() const;

    std::map<std::wstring, std::wstring> info_{};
    std::shared_ptr<BoardSpace::Board> board_{};
    std::shared_ptr<MoveSpace::Move> rootMove_{};
    std::shared_ptr<MoveSpace::Move> currentMove_{}; // board对应该着已执行的状态

    std::wstring remark_{}; // 注释
    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);
const std::string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
const std::wstring test();
}

#endif