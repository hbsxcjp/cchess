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
    ICCS,
    ZH,
    CC,
    BIN,
    JSON
};

namespace InstanceSpace {

class Instance {
public:
    Instance();

    void write(const std::string& fname, const RecFormat fmt = RecFormat::CC) const;

    const bool isStart() const;
    const bool isLast() const;
    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }
    const std::wstring toString() const;
    const std::wstring test() const;

    void read(const std::string& filename);
    void go();
    void back();
    void goOther();
    void backFirst();
    void goLast();
    void goInc(const int inc);
    void changeSide(const ChangeType ct);

private:
    const std::wstring __moveInfo() const;
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
    void setFEN(const std::wstring& pieceChars);
    void setBoard();
    void setMoves(const RecFormat fmt);

    const std::wstring getFEN(const std::wstring& pieceChars) const;
    const std::wstring getPieceChars(const std::wstring& fen) const;

    // 着法节点类
    class Move : public std::enable_shared_from_this<Move> {
    public:
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

        std::shared_ptr<SeatSpace::Seat> fseat_{};
        std::shared_ptr<SeatSpace::Seat> tseat_{};
        std::shared_ptr<PieceSpace::Piece> eatPie_{};
        std::shared_ptr<Move> next_{};
        std::shared_ptr<Move> other_{};
        std::weak_ptr<Move> prev_{};

        std::wstring remark_{}; // 注释
        std::wstring iccs_{}; // 着法数字字母描述
        std::wstring zh_{}; // 着法中文描述
        int n_{ 0 }; // 着法深度
        int o_{ 0 }; // 变着广度
        int CC_Col_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
    };

    std::map<std::wstring, std::wstring> info_;
    std::shared_ptr<BoardSpace::Board> board_;
    std::shared_ptr<Move> rootMove_;
    std::shared_ptr<Move> currentMove_; // board对应该着已执行的状态
    PieceColor firstColor_;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

const std::string getExtName(const RecFormat fmt);
const RecFormat getRecFormat(const std::string& ext);
void transDir(const std::string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);
}

#endif