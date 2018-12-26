#include "base.h"

void print_Names()
{
    //参见《MinGW-W64使得printf、cout、wprintf、wcout显示出中文的种种》
    setlocale(LC_ALL, "chs");   
    ios_base::sync_with_stdio(false); 

    using namespace Piece;
    cout << "Piece::Side red:" << static_cast<int>(Side::red) << "  black:" << static_cast<int>(Side::black) << endl;    
    wcout << L"Piece::Char_Names: ";
    for (const auto m : Char_Names) {
        wcout  << m.first << L": " << m.second << ' ';
    }
    wcout << L"\nPiece::Feature_Names: ";    
    for (const auto m : Feature_Names) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << L"\n\n";

    using namespace Board;
    wcout << L"Board::FEN: " << FEN << endl;
    wcout << L"Board::ColChars: " << ColChars << endl;
    wcout << L"Board::Side_ChNums: "; 
    for (const auto m : Side_ChNums) {
        wcout  << static_cast<int>(m.first) << L": " << m.second << ' ';
    }
    wcout << L"\nBoard::ChNum_Indexs: ";    
    for (const auto m : ChNum_Indexs) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << L"\nBoard::Direction_Nums: ";    
    for (const auto m : Direction_Nums) {
        wcout << m.first << L": " << m.second << ' ';
    }
    wcout << endl;
    wcout << L"文本空棋盘：\n" << TextBlankBoard;
}


int main(int argc, char const *argv[])
{
    print_Names();

    return 0;
}
