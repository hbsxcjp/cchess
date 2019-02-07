#include "chessInstance.h"
#include "info.h"

#include <codecvt>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <utility>

using namespace std;
using namespace Board_base;

ChessInstance::ChessInstance(string filename)
{
    string ext{ filename.substr(filename.size() - 4) };
    for (auto& c : ext)
        c = tolower(c);
    if (ext == ".pgn") {
        wstring pgnTxt{ readTxt(filename) };
        auto pos = pgnTxt.find(L"1.");
        info = Info(pgnTxt.substr(0, pos));
        board = Board(info);
        moves = Moves(pgnTxt.substr(pos), info, board);
    } else if (ext == "json") {
    } else if (ext == ".xqf") {
        vector<int> Keys(4, 0);
        vector<int> F32Keys(32, 0);
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs, Keys, F32Keys);
        board = Board(info);

        wcout << info.toString() << endl;
        wcout << board.toString() << endl;
        moves = Moves(ifs, Keys, F32Keys, board);
    }
}

wstring ChessInstance::toString()
{
    return info.toString() + L"\n" + moves.toString();
}

wstring ChessInstance::toLocaleString()
{
    return info.toString() + L"\n" + moves.toString() + L"\n" + moves.toLocaleString(); //board.toString() + L"\n" +
}
