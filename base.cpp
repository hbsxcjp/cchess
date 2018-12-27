#include "base.h"

void print_consts()
{
    cout << "Side red:" << static_cast<int>(Side::red) << "  black:" << static_cast<int>(Side::black) << endl;
    wcout << L"Char_Names: ";
    for (const auto m : Char_Names) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << L"\nPiece::Feature_Names: ";
    for (const auto m : Feature_Names) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << L"\n\n";

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
}

void print_info()
{
    Info ainfo{};
    wcout << ainfo.toString() << endl;
}

void print_piece()
{
    wcout << L"棋子：" << endl;
    for (auto p : "KABNRCP_kabnrcp") {
        Piece pie(p);
        wcout << static_cast<int>(pie.side()) << pie.ch() << pie.chName() << pie.isKing() << pie.isStronge() << L'\n';
    }
}

int main(int argc, char const* argv[])
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");
    ios_base::sync_with_stdio(false);

    //print_consts();
    //print_info();
    print_piece();

    return 0;
}
