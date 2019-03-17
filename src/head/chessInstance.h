#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H
// 中国象棋棋盘布局类型 by-cjp

#include <string>
enum class RecFormat;
using namespace std;


class ChessInstance {
public:
    //ChessInstance();
    
    static void transDir(const string& dirfrom, const RecFormat fmt);
    static void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

private:

};

#endif