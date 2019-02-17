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
        wstring pgnTxt{ readTxt(filename) }, infoTxt{}, movesTxt{};
        auto pos = pgnTxt.find(L"\n1. ");
        infoTxt = pgnTxt.substr(0, pos);
        movesTxt = pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"";

        //cout << filename << endl;//------------------------------------------------------\n"
        //<< ws2s(infoTxt) << "------------------------------------------------------"
        //<< ws2s(movesTxt) << endl;

        info = Info(infoTxt);
        //cout << "Info OK!" << endl;
        board = Board(info);
        //cout << "Board OK!" << endl;
        moves = Moves(movesTxt, info, board);
        //cout << "Moves OK!" << endl;
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
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs);
        // cout << "Info OK!" << endl;
        board = Board(info);
        //cout << "Board OK!" << endl;
        //wcout << info.toString() << endl;
        //wcout << board.toString() << endl;
        moves = Moves(ifs, board);
        //cout << "Moves OK!" << endl;
        ifs.close();
    } else if (ext == ".json") {
    }
}

void ChessInstance::write(string filename, string ext, RecFormat fmt)
{
    if (ext == ".pgn") {
        writeTxt(filename + "_" + to_string(static_cast<int>(fmt)) + ext,
            info.toString(fmt) + L"\n" + moves.toString(fmt));
    } else if (ext == ".bin") {
        ofstream ofs(filename + ext, ios_base::binary);
        info.toBin(ofs);
        moves.toBin(ofs);
        ofs.close();
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

                        //cout << filename << endl;

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

void ChessInstance::testTransDir(int fd, int td, int ff, int ft, int tf, int tt)
{
    vector<string> dirfroms{
        "c:\\棋谱\\示例文件",
        "c:\\棋谱\\象棋杀着大全",
        "c:\\棋谱\\疑难文件",
        "c:\\棋谱\\中国象棋棋谱大全"
    };
    vector<string> exts{ ".xqf", ".pgn", ".bin", ".json" };

    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex) {
                string dirName{ dirfroms[dir] + exts[fIndex] };
                if (tIndex == fIndex)
                    continue;
                switch (tIndex) {
                case 1:
                    transDir(dirName, ".pgn", RecFormat::ICCS);
                    transDir(dirName, ".pgn", RecFormat::ZH);
                    transDir(dirName, ".pgn", RecFormat::CC);
                    break;
                case 2:
                    transDir(dirName, ".bin", RecFormat::BIN);
                    break;
                case 3:
                    transDir(dirName, ".json", RecFormat::JSON);
                    break;
                default:
                    break;
                }
            }
}
