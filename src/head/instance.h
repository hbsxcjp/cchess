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

namespace InfoSpace {
class Info;
}

namespace MoveSpace {
class Move;
class MoveOwner;
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

    void read(const std::string& infilename);
    void write(const std::string& outfilename);

    const bool isStart() const;
    const bool isLast() const;
    void go();
    void back();
    void goOther();
    void backFirst();
    void goLast();
    void goInc(const int inc);
    void changeSide(const ChangeType ct);
    const std::wstring toString() const;
    const std::wstring test();

    const int getMovCount() const;
    const int getRemCount() const;
    const int getRemLenMax() const;
    const int getMaxRow() const;
    const int getMaxCol() const;

private:
    std::shared_ptr<InfoSpace::Info> info_;
    std::shared_ptr<BoardSpace::Board> board_;
    std::shared_ptr<MoveSpace::MoveOwner> MoveOwner_;
    std::shared_ptr<MoveSpace::Move> currentMove_; // board对应该着已执行的状态
};

const std::string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
}

#endif