#include "info.h"
#include "piece.h"
#include <iostream>
//#include <fstream>
#include <locale>
//#include "board.h"

using namespace std;

wstring test_info()
{
    Info ainfo{};
    return ainfo.toString();
}

wstring test_piece()
{
    wstringstream wss{};
    Pieces pieces = Pieces();

    wss << L'\n' << L"红棋King：" << pieces.getKing(Side::red).chName()
        << L'\n' << L"黑棋King：" << pieces.getKing(Side::black).chName() << L"\n对称棋子：\n";
    for (auto pie : pieces.getPies())
        wss << pie.chName() << L"->" << pieces.getOthSidePiece(pie).chName() << L' ';

    wss << L'\n' + pieces.toString();
    wss << L"特征棋子串：\n"
        << Pieces::kingNames << L' ' << Pieces::pawnNames << L' ' << Pieces::advbisNames
        << L' ' << Pieces::strongeNames << L' ' << Pieces::lineNames << L' ' << Pieces::allNames;

    return wss.str();
    /*

    wcout << L"FEN: " << FEN << endl;
    wcout << L"ColChars: " << ColChars << endl;
    wcout << L"Side_ChNums: ";
    for (const auto m : Side_ChNums) {
        wcout << static_cast<int>(m.first) << L": " << m.second << ' ';
    }
    wcout << L"\nChNum_Indexs: ";
    for (const auto m : ChNum_Indexs) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << L"\nDirection_Nums: ";
    for (const auto m : Direction_Nums) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << endl;
    wcout << L"TextBlankBoard 文本空棋盘：\n"
          << TextBlankBoard << endl;
*/
}

int main(int argc, char const* argv[])
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");
    ios_base::sync_with_stdio(false);

    wcout << (test_info() + test_piece());
    return 0;
}
