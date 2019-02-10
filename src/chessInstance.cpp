#include "chessInstance.h"

#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
//#include <sstream>
//#include <locale>
//#include <regex>
//#include <codecvt>
//#include <utility>

using namespace std;
using namespace Board_base;

ChessInstance::ChessInstance(string filename)
{
    string ext{ getExt(filename) };
    if (ext == ".pgn") {
        wstring pgnTxt{ readTxt(filename) };
        auto pos = pgnTxt.find(L"1.");
        info = Info(pgnTxt.substr(0, pos));
        board = Board(info);
        moves = Moves(pgnTxt.substr(pos), info, board);
    } else if (ext == ".json") {
    } else if (ext == ".xqf") {
        vector<int> Keys(4, 0);
        vector<int> F32Keys(32, 0);
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs, Keys, F32Keys);
        board = Board(info);
        //wcout << info.toString() << endl;
        //wcout << board.toString() << endl;
        moves = Moves(ifs, Keys, F32Keys, board);
    }
}

wstring ChessInstance::toString()
{
    return info.toString() + L"\n" + moves.toString();
}

wstring ChessInstance::toLocaleString()
{
    return info.toString() + L"\n" + moves.toLocaleString(); //board.toString() + L"\n" +
}

void ChessInstance::write(string filename, RecFormat fmt)
{
    if (fmt == RecFormat::zh)
        writeTxt(filename, toString()); // toLocaleString()
}

void ChessInstance::transdir(string dirfrom, string ext, RecFormat fmt)
{
    string extensions{ ".xqf.bin.xml.pgn" };
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + ext };
    function<void(string, string)> __trans = [&](string dirfrom, string dirto) {
        long hFile = 0; //文件句柄
        struct _finddata_t fileinfo; //文件信息
        if (access(dirto.c_str(), 0) != 0)
            mkdir(dirto.c_str());
        if ((hFile = _findfirst((dirfrom + "/*").c_str(), &fileinfo)) != -1) {
            do {
                string fname{ fileinfo.name };
                if (fileinfo.attrib & _A_SUBDIR) { //如果是目录,迭代之
                    if (fname != "." && fname != "..") {
                        dcount += 1;
                        __trans(dirfrom + "/" + fname, dirto + "/" + fname);
                    }
                } else { //如果是文件,执行转换
                    string filefrom{ dirfrom + "/" + fname };
                    string fileto{ dirto + "/" + fname.substr(0, fname.rfind('.')) };
                    string ext_old{ getExt(fname) };
                    fcount += 1;
                    if (extensions.find(ext_old) != string::npos) {
                        ChessInstance ci(filefrom);
                        ci.write(fileto + ext, fmt);
                        movcount += ci.moves.movCount;
                        remcount += ci.moves.remCount;
                        if (ci.moves.remLenMax > remlenmax)
                            remlenmax = ci.moves.remLenMax;
                    } else
                        copyFile(filefrom.c_str(), (fileto + ext_old).c_str());
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    };

    __trans(dirfrom, dirto);
    cout << dirfrom + ": " << fcount << "个文件, "
         << dcount << "个目录转换成功！\n着法数量: "
         << movcount << ", 注释数量: " << remcount << ", 最大注释长度: " << remlenmax << endl;
}
