#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include "board.h"
#include "info.h"
#include "move.h"

#include <string>
using std::string;
using std::wstring;

class ChessInstance {
public:
    ChessInstance(string filename);

    void write(string filename, string ext, RecFormat fmt = RecFormat::ZH);
    
    static void transDir(string dirfrom, string ext, RecFormat fmt = RecFormat::ZH);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

    // void loadViews(views);
    // void notifyViews();
private:
    Info info;
    Board board;
    Moves moves;
};

#endif