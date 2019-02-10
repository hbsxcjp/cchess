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

    wstring toString();
    wstring toLocaleString();

    void write(string filename, RecFormat fmt=RecFormat::zh);
    static void transdir(string dirfrom, string ext, RecFormat fmt=RecFormat::zh);

    // void loadViews(views);
    // void notifyViews();
private:
    Info info;
    Board board;
    Moves moves;
};

#endif