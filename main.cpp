#include "chessInstance.h"

#include <iostream>
using std::wcout;
using std::cout;

#include <locale>

int main(int argc, char const* argv[])
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    //std::locale loc = std::locale::global(std::locale(""));

    setlocale(LC_ALL, ""); //"chs"
    std::ios_base::sync_with_stdio(false);

    //wcout << Board_base::test();
    //wcout << Info().test();
    //wcout << Pieces().test();
    //wcout << Board().test(); // L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"

    //*
    if (argc > 1) {
        string filename{ argv[1] };
        ChessInstance ci(filename);
        writeTxt(filename + "_out.txt", ci.toLocaleString()); // + L"\n\n" + ci.toString()
    }
    /*/
    vector<string> files;
    getFiles("C:\\棋谱文件", files);
    for (auto s : files)
        cout << s << '\n';

    //std::locale::global(loc);*/

    return 0;
}
