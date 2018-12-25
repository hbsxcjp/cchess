#include "base.h"

void print_CharNames()
{
    for (const auto m : CharNames) {
        wcout  << m.first << L": " << m.second << endl; //
    }
}

void print_Feature_PieceNames()
{
    for (const auto m : Feature_PieceNames) {
        wcout << m.first << L": " << m.second << endl;
    }
}

int main(int argc, char const *argv[])
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");   
    ios_base::sync_with_stdio(false); 

    print_CharNames();
    print_Feature_PieceNames();

    wcout << BlankBoard;
    string s{"OK\n"};
    cout << s;


    return 0;
}
