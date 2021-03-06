#include "board.h"
#include "instance.h"
#include "tools.h"
#include <chrono>
#include <iostream>
#include <locale>
//#define NDEBUG

int main(int argc, char const* argv[])
{
    using namespace std::chrono;
    //std::locale loc = std::locale::global(std::locale(""));
    setlocale(LC_ALL, "");
    std::ios_base::sync_with_stdio(false);

    auto time0 = steady_clock::now();
    //*
    BoardSpace::Board board{};
    Tools::writeTxt("board.txt", board.test());
    InstanceSpace::Instance ci{};
    Tools::writeTxt("instance.txt", ci.test());
    //*/
    //*
    if (argc == 7)
        InstanceSpace::testTransDir(std::stoi(argv[1]), std::stoi(argv[2]),
            std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
    else {
        //InstanceSpace::testTransDir(0, 2, 0, 6, 1, 6);
        //std::cout << "------------------------------------------------------------------" << std::endl;
        InstanceSpace::testTransDir(0, 2, 0, 1, 1, 6);
        InstanceSpace::testTransDir(0, 2, 1, 5, 5, 6);
        InstanceSpace::testTransDir(0, 2, 5, 6, 1, 2);
        //std::cout << "------------------------------------------------------------------" << std::endl;
        //InstanceSpace::testTransDir(0, 2, 2, 3, 1, 5);
    }
    //*/

    auto time_d = steady_clock::now() - time0;
    std::cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //std::locale::global(loc);
    return 0;
}
