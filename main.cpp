//#include "board_base.h"
//#include "info.h"
//#include "piece.h"
//#include "board.h"
//#include "move.h"
#include "chessInstance.h"

//#include <fstream>
#include <iostream>
using std::wcout;

#include <locale>

int main(int argc, char const *argv[]) {
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    //std::locale loc = std::locale::global(std::locale(""));

    setlocale(LC_ALL, "chs");
    std::ios_base::sync_with_stdio(false);

    //wcout << Board_base::test();
    //wcout << Info().test();
    //wcout << Pieces().test();
    //wcout << Board().test(); // L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"
    wcout << Moves::test();
    
    /*
    if(argc > 1){
        ChessInstance ci(argv[1]);
        wcout << ci.test();
    }
    */

    //std::locale::global(loc);

    return 0;
}
