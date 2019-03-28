#include "board.h"
#include "chessInstance.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include <chrono>
#include <iostream>
#include <locale>

int main(int argc, char const* argv[])
{
    using namespace std::chrono;
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
    auto board = BoardSpace::Board();
    std::wcout << board.test() << std::endl;
    //auto instance = InstanceSpace::Instance("01.xqf");
    //std::wcout << instance.test() << std::endl;
    //*/

    auto time_d = steady_clock::now() - time0;
    std::cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //std::locale::global(loc);
    return 0;
}
