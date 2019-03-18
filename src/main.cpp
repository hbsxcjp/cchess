#include "piece.h"
#include "seat.h"
#include "move.h"
#include "board.h"
#include "instance.h"
#include "chessInstance.h"
#include <chrono>
#include <iostream>
#include <locale>
using namespace std;
using namespace std::chrono;

int main(int argc, char const* argv[])
{
    //std::locale loc = std::locale::global(std::locale(""));
    setlocale(LC_ALL, "");
    std::ios_base::sync_with_stdio(false);

    auto time0 = steady_clock::now();
    /*
    if (argc == 7)
        ChessInstance::testTransDir(stoi(argv[1]), stoi(argv[2]),
            stoi(argv[3]), stoi(argv[4]), stoi(argv[5]), stoi(argv[6]));
    else
        ChessInstance::testTransDir(0, 2, 0, 1, 3, 4);
    //*/
    
    //*
    auto board = Board();
    wcout << board.test() << endl;
    //auto instance = Instance("01.xqf");
    //wcout << instance.test() << endl;
    //*/

    auto time_d = steady_clock::now() - time0;
    cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //std::locale::global(loc);
    return 0;
}
