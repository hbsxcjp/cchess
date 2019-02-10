#include "chessInstance.h"

#include <chrono>
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

    auto time0 = steady_clock::now();

    string dirfrom0 = "C:\\棋谱\\示例文件.xqf";
    ChessInstance::transdir(dirfrom0, ".pgn", RecFormat::zh);
    string dirfrom1 = "C:\\棋谱\\象棋杀着大全.xqf";
    ChessInstance::transdir(dirfrom1, ".pgn", RecFormat::zh);
    string dirfrom2 = "C:\\棋谱\\疑难文件.xqf";
    //ChessInstance::transdir(dirfrom2, ".pgn", RecFormat::zh);

    auto time_d = steady_clock::now() - time0;
    cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //std::locale::global(loc);*/

    return 0;
}
