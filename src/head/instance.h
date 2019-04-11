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

    void read(const std::string& infilename);
    void write(const std::string& outfilename) const;
    void setFEN(const std::wstring& pieceChars);
    const std::map<std::wstring, std::wstring>& getInfo() const { return info_; }
    const std::shared_ptr<MoveSpace::Move>& getRootMove() const { return rootMove_; }
    void setInfo(const std::map<std::wstring, std::wstring>& info) { info_ = info; }
    void setBoard();
    //void setRootMove(const std::shared_ptr<MoveSpace::Move>& rootMove) { rootMove_ = rootMove; }
    void setMoves(const RecFormat fmt);

    const bool isStart() const;
    const bool isLast() const;
    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }
    const std::wstring moveInfo() const;
    const std::wstring toString() const;
    const std::wstring test() const;

    void go();
    void back();
    void goOther();
    void backFirst();
    void goLast();
    void goInc(const int inc);
    void changeSide(const ChangeType ct);

private:
    /*
    void writePGN(const std::string& filename, const RecFormat fmt = RecFormat::CC) const;
    const std::wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH) const;
    const std::wstring toString_CC() const;
    void writeBIN(const std::string& filename) const;
    void writeJSON(const std::string& filename) const;

    void readXQF(const std::string& filename);
    void readPGN(const std::string& filename, const RecFormat fmt);
    void readBIN(const std::string& filename);
    void readJSON(const std::string& filename);
    void __readICCSZH(const std::wstring& moveStr, const RecFormat fmt);
    void __readCC(const std::wstring& fullMoveStr);
    */

    std::map<std::wstring, std::wstring> info_;
    std::shared_ptr<BoardSpace::Board> board_;
    std::shared_ptr<MoveSpace::Move> rootMove_;
    std::shared_ptr<MoveSpace::Move> currentMove_; // board对应该着已执行的状态
    PieceColor firstColor_;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};
/*
class InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const = 0;
    virtual void read(const std::string& infilename, Instance& instance) = 0;

protected:
    const std::wstring __readInfo_getMoveStr(const std::string& infilename, Instance& instance);
    void __readMove_ICCSZH(const std::wstring& moveStr, Instance& instance, const RecFormat fmt);
    void __readMove_CC(const std::wstring& moveStr, Instance& instance);
    const std::wstring __getPGNInfo(const Instance& instance) const;
    const std::wstring __getPGNTxt_ICCSZH(const Instance& instance, const RecFormat fmt) const;
    const std::wstring __getPGNTxt_CC(const Instance& instance) const;
    */
};

/*
class XQFInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};

class PGN_ICCSInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};

class PGN_ZHInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};

class PGN_CCInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};

class BINInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};

class JSONInstanceRecord : public InstanceRecord {
public:
    virtual void write(const std::string& outfilename, const Instance& instance) const;
    virtual void read(const std::string& infilename, Instance& instance);
};
*/
//std::shared_ptr<InstanceRecord> getInstanceRecord(RecFormat fmt);

const std::string getExtName(const RecFormat fmt);
const RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
}

#endif