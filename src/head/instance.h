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

    void reset();
    void read(const std::string& infilename);
    void write(const std::string& outfilename);

    const std::wstring& remark() const { return remark_; }
    const int getMovCount() const { return movCount_; }
    const int getRemCount() const { return remCount_; }
    const int getRemLenMax() const { return remLenMax_; }
    const int getMaxRow() const { return maxRow_; }
    const int getMaxCol() const { return maxCol_; }

private:    
    void __readXQF(std::istream& is);
    const std::wstring __getMoveStr(std::istream& is) const;
    void __readInfo_PGN(std::istream& is);
    void __writeInfo_PGN(std::ostream& os) const;
    void __readMove_PGN_ICCSZH(std::istream& is, RecFormat fmt);
    void __writeMove_PGN_ICCSZH(std::ostream& os, RecFormat fmt) const;
    void __readMove_PGN_CC(std::istream& is);
    void __writeMove_PGN_CC(std::ostream& os) const;
    void __readBIN(std::istream& is);
    void __writeBIN(std::ostream& os) const;
    void __readJSON(std::istream& is);
    void __writeJSON(std::ostream& os) const;

    void __setFEN(const std::wstring& pieceChars, PieceColor color);
    void __setFormat(RecFormat fmt);
    const std::wstring __pieceChars() const;
    void __setMoves(RecFormat fmt);
    const std::wstring __moveInfo() const;

    std::wstring remark_{}; // 注释
    std::map<std::wstring, std::wstring> info_{};
    std::shared_ptr<MoveSpace::Move> rootMove_{};
    std::shared_ptr<BoardSpace::Board> board_{};
    std::shared_ptr<MoveSpace::Move> currentMove_{}; // board对应该着已执行的状态

    int movCount_{ 0 }; //着法数量
    int remCount_{ 0 }; //注解数量
    int remLenMax_{ 0 }; //注解最大长度
    int maxRow_{ 0 }; //# 存储最大着法深度
    int maxCol_{ 0 }; //# 存储视图最大列数
};

const std::wstring pieCharsToFEN(const std::wstring& pieceChars);  // 便利函数，下同
const std::wstring FENTopieChars(const std::wstring& fen);
const std::string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
const std::wstring test();
}

#endif