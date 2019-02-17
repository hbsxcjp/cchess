#include "chessInstance.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>

using namespace std;
using namespace std::chrono;

int main(int argc, char const* argv[])
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    //std::locale loc = std::locale::global(std::locale(""));
    setlocale(LC_ALL, ""); //"chs"
    std::ios_base::sync_with_stdio(false);

    auto time0 = steady_clock::now();

    ChessInstance::testTransDir(stoi(argv[1]), stoi(argv[2]),
        stoi(argv[3]), stoi(argv[4]), stoi(argv[5]), stoi(argv[6]));

    auto time_d = steady_clock::now() - time0;
    cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //wcout << Board_base::test();
    //wcout << Info().test();
    //wcout << Pieces().test();
    //wcout << Board().test(); // L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5"

    /*
    if (argc > 1) {
        string filename{ argv[1] };
        ChessInstance ci(filename);
        writeTxt(filename + "_out.txt", ci.toLocaleString()); // + L"\n\n" + ci.toString()
    }
    */
    //std::locale::global(loc);*/

    return 0;
}
