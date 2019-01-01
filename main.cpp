#include "board.h"
//#include <fstream>
#include <locale>

#include <iostream>

using namespace std;


int main(int argc, char const *argv[]) {
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");
    ios_base::sync_with_stdio(false);

    //wcout << test_info();
    //wcout << test_piece();

    wcout << test_board();

    return 0;
}
