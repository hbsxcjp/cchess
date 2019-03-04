#ifndef CHESSINSTANCEIO_H
#define CHESSINSTANCEIO_H
// 中国象棋棋盘布局类型 by-cjp

#include <map>
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

    static void __initSet(ChessInstance& ci, const RecFormat fmt);

private:
    static void readXQF(const string& filename, ChessInstance& ci);
    static void readPGN(const string& filename, ChessInstance& ci, const RecFormat fmt);
    static void readBIN(const string& filename, ChessInstance& ci);
    static void readJSON(const string& filename, ChessInstance& ci);

    static void __fromICCSZH(const wstring& moveStr, ChessInstance& ci, const RecFormat fmt);
    static void __fromCC(const wstring& fullMoveStr, ChessInstance& ci);
    static const pair<int, int> getSeat__ICCS(const wstring& ICCS);
    static const wstring getICCS(const int fseat, const int tseat);
    // 中文纵线着法->(fseat, tseat), 着法未走状态
    static const pair<int, int> getSeat__Zh(const wstring& Zh, ChessInstance& ci);
    // (fseat, tseat)->中文纵线着法, 着法未走状态
    static const wstring getZh(const int fseat, const int tseat, ChessInstance& ci);

    static void writePGN(const string& filename, ChessInstance& ci, const RecFormat fmt = RecFormat::ZH);
    static const wstring toString_ICCSZH(ChessInstance& ci, const RecFormat fmt = RecFormat::ZH);
    static const wstring toString_CC(ChessInstance& ci);
    static void writeBIN(const string& filenameconst, ChessInstance& ci);
    static void writeJSON(const string& filenameconst, ChessInstance& ci);

    static const wstring __pieceCharsToFEN(const wstring& pieceChars);
    static const wstring __fenToPieChars(const wstring fen);
    static const wstring getNumChars(const PieceColor color);
    static const string getExtName(const RecFormat fmt);
    static const RecFormat getRecFormat(const string& ext);
    static const bool isKing(const wchar_t name);
    static const bool isPawn(const wchar_t name);
    static const bool isAdvBishop(const wchar_t name);
    static const bool isStronge(const wchar_t name);
    static const bool isLine(const wchar_t name);
    static const bool isPiece(const wchar_t name);
};

#endif