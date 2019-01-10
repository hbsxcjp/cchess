#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include "info.h"
#include "board.h"
#include "move.h"

#include <string>
using std::wstring;

class ChessInstance {
    ChessInstance();
    ChessInstance(wstring pgnText);

    wstring toString();
    wstring toLocaleString();

    // void loadViews(views);
    // void notifyViews();

  private:
    Info info;
    Board board;
    Moves moves;
};

#endif