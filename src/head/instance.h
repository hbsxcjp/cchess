#ifndef INSTANCE_H
#define INSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include <map>
#include <memory>
#include <string>


class Board;
class Move;
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

class Instance {
public:
    Instance();
    Instance(const std::string& filename);
    void write(const std::string& fname, const RecFormat fmt = RecFormat::CC);

    const PieceColor currentColor() const;
    const bool isStart() const;
    const bool isLast() const;
    void forward();
    void backward();
    void forwardOther();
    void backwardTo(std::shared_ptr<Move> move);
    void to(std::shared_ptr<Move> move);
    void backFirst();
    void forLast();
    void go(const int inc);
    void cutNext();
    void cutOther();
    void changeSide(const ChangeType ct);

    const int getMovCount() const { return movCount; }
    const int getRemCount() const { return remCount; }
    const int getRemLenMax() const { return remLenMax; }
    const int getMaxRow() const { return maxRow; }
    const int getMaxCol() const { return maxCol; }

    static const std::string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const std::string& ext);

    const std::wstring toString() const;
    const std::wstring test() const;

private:
    void readXQF(const std::string& filename);
    void readPGN(const std::string& filename, const RecFormat fmt);
    void readBIN(const std::string& filename);
    void readJSON(const std::string& filename);
    void __readICCSZH(const std::wstring& moveStr, const RecFormat fmt);
    void __readCC(const std::wstring& fullMoveStr);

    const std::wstring __moveInfo() const;
    void writePGN(const std::string& filename, const RecFormat fmt = RecFormat::CC) const;
    const std::wstring toString_ICCSZH(const RecFormat fmt = RecFormat::ZH) const;
    const std::wstring toString_CC() const;
    void writeBIN(const std::string& filenameconst) const;
    void writeJSON(const std::string& filenameconst) const;

    void setFEN(const std::wstring& pieceChars);
    void setBoard();
    void setMoves(const RecFormat fmt);

    std::map<std::wstring, std::wstring> info_;
    std::shared_ptr<Board> board_;
    std::shared_ptr<Move> rootMove_;
    std::shared_ptr<Move> currentMove_; // board对应该着已执行的状态
    PieceColor firstColor_;

    int movCount{ 0 }; //着法数量
    int remCount{ 0 }; //注解数量
    int remLenMax{ 0 }; //注解最大长度
    int maxRow{ 0 }; //# 存储最大着法深度
    int maxCol{ 0 }; //# 存储视图最大列数
};

#endif