#ifndef CHESSINSTANCEIO_H
#define CHESSINSTANCEIO_H
// 中国象棋棋盘布局类型 by-cjp

#include <memory>
#include <string>
#include <vector>
using namespace std;

class ChessInstance;
enum class PieceColor;
enum class RecFormat {
    XQF,
    ICCS,
    ZH,
    CC,
    BIN,
    JSON
};

class ChessInstanceIO {
public:
    static void read(const string& filename, ChessInstance& ci);
    static void write(const string& filename, ChessInstance& ci, const RecFormat fmt = RecFormat::ZH);

    static void transDir(const string& dirfrom, const RecFormat fmt = RecFormat::XQF);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

private:
    static void readXQF(const string& filename, ChessInstance& ci);
    static void readPGN(const string& filename, ChessInstance& ci, const RecFormat fmt);
    static void readBIN(const string& filename, ChessInstance& ci);
    static void readJSON(const string& filename, ChessInstance& ci);
    static void __fromICCSZH(const wstring& moveStr, ChessInstance& ci, const RecFormat fmt);
    static void __fromCC(const wstring& fullMoveStr, ChessInstance& ci);

    static void writePGN(const string& filename, ChessInstance& ci, const RecFormat fmt = RecFormat::ZH);
    static const wstring toString_ICCSZH(ChessInstance& ci, const RecFormat fmt = RecFormat::ZH);
    static const wstring toString_CC(ChessInstance& ci);
    static void writeBIN(const string& filenameconst, ChessInstance& ci);
    static void writeJSON(const string& filenameconst, ChessInstance& ci);

    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);
};

#endif