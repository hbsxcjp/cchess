#include "chessInstance.h"
#include "instance.h"
#include "tools.h"
#include <algorithm>
#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <iomanip>
using namespace std;

void ChessInstance::transDir(const string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string extensions{ ".xqf.pgn1.pgn2.pgn3.bin.json" };
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + Instance::getExtName(fmt) };
    function<void(string, string)> __trans = [&](const string& dirfrom, string dirto) {
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
                    string ext_old{ Tools::getExt(fname) };
                    if (extensions.find(ext_old) != string::npos) {
                        fcount += 1;
                        
                        cout << filename << endl;
                        Instance ci(filename);

                        ci.write(fileto, fmt);
                        cout << fileto << endl;

                        movcount += ci.getMovCount();
                        remcount += ci.getRemCount();
                        if (remlenmax < ci.getRemLenMax())
                            remlenmax = ci.getRemLenMax();
                    } else
                        Tools::copyFile(filename.c_str(), (fileto + ext_old).c_str());
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    };

    __trans(dirfrom, dirto);
    cout << dirfrom + " =>" << Instance::getExtName(fmt) << ": 转换" << fcount << "个文件, "
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
    vector<RecFormat> fmts{ RecFormat::XQF, RecFormat::ICCS, RecFormat::ZH, RecFormat::CC,
        RecFormat::BIN, RecFormat::JSON };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex != fIndex)
                    transDir(dirfroms[dir] + Instance::getExtName(fmts[fIndex]), fmts[tIndex]);
}
