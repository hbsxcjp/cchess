#include "chessInstance.h"

#include <fstream>
#include <iostream>
#include <functional>
#include <direct.h>
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
    } else if (ext == ".xqf") {
        vector<int> Keys(4, 0);
        vector<int> F32Keys(32, 0);
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs, Keys, F32Keys);
        board = Board(info);
        //wcout << info.toString() << endl;
        //wcout << board.toString() << endl;
        moves = Moves(ifs, Keys, F32Keys, board);
    } else if (ext == ".bin") {
    } else if (ext == ".json") {
    }
}

void ChessInstance::write(string filename, string ext, RecFormat fmt)
{
    if (ext == ".pgn") {
        wstring ws{};
        switch (fmt) {
        case RecFormat::zh:
            ws = info.toString() + L"\n" + moves.toString_zh();
            break;
        case RecFormat::CC:
            ws = info.toString() + L"\n" + board.toString() + L"\n" + moves.toString_CC();
            break;
        default:
            ws = info.toString() + L"\n" + moves.toString_ICCS();
            break;
        }
        writeTxt(filename + "_" + to_string(static_cast<int>(fmt)) + ext, ws);
    } else if (ext == ".bin") {
    } else if (ext == ".json") {
    }
}

void ChessInstance::transDir(string dirfrom, string ext, RecFormat fmt)
{
    string extensions{ ".xqf.pgn.bin.json" };
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + ext };
    string ext_old{};
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
                    string filename{ dirfrom + "/" + fname };
                    string fileto{ dirto + "/" + fname.substr(0, fname.rfind('.')) };
                    ext_old = getExt(fname);
                    fcount += 1;
                    if (extensions.find(ext_old) != string::npos) {
                        ChessInstance ci(filename);
                        ci.write(fileto, ext, fmt);
                        movcount += ci.moves.movCount;
                        remcount += ci.moves.remCount;
                        if (ci.moves.remLenMax > remlenmax)
                            remlenmax = ci.moves.remLenMax;
                    } else
                        copyFile(filename.c_str(), (fileto + ext_old).c_str());
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    };

    __trans(dirfrom, dirto);
    cout << dirfrom + " =>" << static_cast<int>(fmt) << ext << ": 转换" << fcount << "个文件, "
         << dcount << "个目录成功！\n   着法数量: "
         << movcount << ", 注释数量: " << remcount << ", 最大注释长度: " << remlenmax << endl;
}

void ChessInstance::testTransDir()
{
    vector<string> dirfroms{ "c:\\棋谱\\示例文件",
        "c:\\棋谱\\象棋杀着大全",
        "c:\\棋谱\\疑难文件",
        "c:\\棋谱\\中国象棋棋谱大全" };
    vector<string> fexts{ ".xqf", ".pgn", ".bin", ".json" };
    vector<string> texts{ ".pgn", ".bin", ".json" };
    RecFormat fmts[]{ RecFormat::ICCS, RecFormat::zh, RecFormat::CC };

    int dc{ 2 }, fec{ 2 }, tec{ 1 }, fmc{ 3 };
    for (int i = 0; i != dc; ++i)
        for (int j = 0; j != fec; ++j)
            for (int k = 0; k != tec; ++k)
                if (texts[k] == fexts[j])
                    continue;
                else
                    for (int n = 0; n != fmc; ++n)
                        transDir(dirfroms[i] + fexts[j], texts[k], fmts[n]);
}
