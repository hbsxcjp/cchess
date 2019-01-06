#include "info.h"
#include "board_base.h"
//#include "piece.h"
#include "board.h"
//#include <fstream>
#include <locale>

#include <iostream>

using namespace std;


int main(int argc, char const *argv[]) {
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");
    ios_base::sync_with_stdio(false);
    //wstringstream wss{};

    //wcout << Info().test_info();
    //wcout << Board_base::test_board_base();
    
    //Pieces ps{Pieces()};
    //wcout << ps.test_piece();
    wcout << Board().test_board();

    return 0;
}
