#include <chrono>
#include <fstream>
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

    /*
    unsigned char a{ 100 }, b{ 200 }, c{};
    std::cout << int(a) << "+" << int(b) << "=" << int(c = a + b) << std::endl;
    std::cout << int(a) << "-" << int(b) << "=" << int(c = a - b) << std::endl;
    */
    //std::stringstream ss{};
    std::ifstream ifs("a.txt");
    std::string line{};
    ifs >> std::noskipws;
    //while (std::getline(ifs, line) && !line.empty()) {
    while ((ifs >> line) && !line.empty()) {
        std::cout << line << std::endl;
    }

    auto time_d = steady_clock::now() - time0;
    std::cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    //std::locale::global(loc);
    return 0;
}