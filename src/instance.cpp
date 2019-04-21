#include "instance.h"
#include "../json/json.h"
#include "info.h"
#include "board.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <direct.h>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace InstanceSpace {

// Instance
Instance::Instance()
    : info_{ std::make_shared<InfoSpace::Info>() }
    , board_{ std::make_shared<BoardSpace::Board>() }
    , MoveOwner_{ std::make_shared<MoveSpace::MoveOwner>() }
    , currentMove_{}    
{
}

void Instance::read(const std::string& infilename)
{
    RecFormat fmt = getRecFormat(Tools::getExt(infilename));
    std::ifstream ifs;
    ifs = fmt != RecFormat::XQF ? std::ifstream(infilename) : std::ifstream(infilename, std::ios_base::binary);
    info_ = std::make_shared<InfoSpace::Info>();
    info_->read(ifs, fmt);
    //std::wcout << L"info finished!" << info_->toString() << info_->getPieceChars() << std::endl;//
    board_->reset(info_->getPieceChars());
    //std::wcout << L"board_->reset finished!\n" << board_->toString() << std::endl;//
    MoveOwner_ = std::make_shared<MoveSpace::MoveOwner>();
    MoveOwner_->read(ifs, fmt, *board_, info_->getKey());
    std::wcout << L"rootMove finished!" << std::endl;
    currentMove_ = nullptr;
}

void Instance::write(const std::string& outfilename)
{
    RecFormat fmt = getRecFormat(Tools::getExt(outfilename));
    std::ofstream ofs(outfilename);
    info_->write(ofs, fmt);
    //std::wcout << L"writeInfo finished!\n" << info_->toString() << std::endl;
    MoveOwner_->write(ofs, fmt);
    //std::wcout << L"writeMove finished!\n" << MoveOwner_->toString() << std::endl;
}

const bool Instance::isStart() const { return !currentMove_; }

const bool Instance::isLast() const { return !currentMove_ || !currentMove_->next(); }

// 基本走法
void Instance::go()
{
    if (!isLast()) {
        currentMove_ = currentMove_->next()->done();
    }
}

void Instance::back()
{
    if (!isStart()) {
        currentMove_->undo();
        currentMove_ = currentMove_->prev();
    }
}

//'移动到当前节点的另一变着'
void Instance::goOther()
{
    if (currentMove_->other()) {
        currentMove_->undo();
        currentMove_ = currentMove_->other()->done();
    }
}

void Instance::backFirst()
{
    while (!isStart())
        back();
}

void Instance::goLast()
{
    while (!isLast())
        go();
}

void Instance::goInc(const int inc)
{
    //std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::go : &Instance::back);
    for (int i = abs(inc); i != 0; --i)
        fbward(this);
}

void Instance::changeSide(const ChangeType ct) // 未测试
{
    auto prevMoves = currentMove_->getPrevMoves();
    backFirst();
    board_->changeSide(ct);
    if (ct != ChangeType::EXCHANGE) {
        auto changeRowcol = std::mem_fn(ct == ChangeType::ROTATE ? &BoardSpace::Board::getRotate : &BoardSpace::Board::getSymmetry);
        MoveOwner_->changeSide(&(*board_), changeRowcol);
    }
    //auto color = prevMoves.empty() ? PieceColor::RED : prevMoves[0]->fseat()->piece()->color();
    //info_->setFEN(board_->getPieceChars(), color);
    for (auto& move : prevMoves)
        move->done();
}

const std::wstring Instance::toString() const
{
    std::wstringstream wss{};
    wss << info_->toString() << board_->toString() << MoveOwner_->toString();
    return wss.str();
}

const std::string getExtName(const RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::XQF:
        return ".xqf";
    case RecFormat::BIN:
        return ".bin";
    case RecFormat::JSON:
        return ".json";
    case RecFormat::PGN_ICCS:
        return ".pgn_iccs";
    case RecFormat::PGN_ZH:
        return ".pgn_zh";
    case RecFormat::PGN_CC:
        return ".pgn_cc";
    default:
        return ".pgn_cc";
    }
}

RecFormat getRecFormat(const std::string& ext)
{
    if (ext == ".xqf")
        return RecFormat::XQF;
    else if (ext == ".bin")
        return RecFormat::BIN;
    else if (ext == ".json")
        return RecFormat::JSON;
    else if (ext == ".pgn_iccs")
        return RecFormat::PGN_ICCS;
    else if (ext == ".pgn_zh")
        return RecFormat::PGN_ZH;
    else if (ext == ".pgn_cc")
        return RecFormat::PGN_CC;
    else
        return RecFormat::PGN_CC;
}

const int Instance::getMovCount() const { return MoveOwner_->getMovCount(); }
const int Instance::getRemCount() const { return MoveOwner_->getRemCount(); }
const int Instance::getRemLenMax() const { return MoveOwner_->getRemLenMax(); }
const int Instance::getMaxRow() const { return MoveOwner_->getMaxRow(); }
const int Instance::getMaxCol() const { return MoveOwner_->getMaxCol(); }

void transDir(const std::string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    std::string extensions{ ".xqf.pgn_iccs.pgn_zh.pgn_cc.bin.json" };
    std::string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + getExtName(fmt) };
    Instance ci{};
    std::function<void(std::string, std::string)> __trans = [&](const std::string& dirfrom, std::string dirto) {
        long hFile = 0; //文件句柄
        struct _finddata_t fileinfo; //文件信息
        if (access(dirto.c_str(), 0) != 0)
            mkdir(dirto.c_str());
        if ((hFile = _findfirst((dirfrom + "/*").c_str(), &fileinfo)) != -1) {
            do {
                std::string filename{ fileinfo.name };
                if (fileinfo.attrib & _A_SUBDIR) { //如果是目录,迭代之
                    if (filename != "." && filename != "..") {
                        dcount += 1;
                        __trans(dirfrom + "/" + filename, dirto + "/" + filename);
                    }
                } else { //如果是文件,执行转换
                    std::string infilename{ dirfrom + "/" + filename };
                    std::string fileto{ dirto + "/" + filename.substr(0, filename.rfind('.')) };
                    std::string ext_old{ Tools::getExt(filename) };
                    if (extensions.find(ext_old) != std::string::npos) {
                        fcount += 1;

                        //std::cout << infilename << std::endl;
                        ci.read(infilename);
                        //std::cout << infilename << " read finished!" << std::endl;
                        ci.write(fileto + getExtName(fmt));
                        //std::cout << fileto + getExtName(fmt) << " write finished!" << std::endl;
                        //std::cout << fileto << std::endl;

                        movcount += ci.getMovCount();
                        remcount += ci.getRemCount();
                        remlenmax = std::max(remlenmax, ci.getRemLenMax());
                    } else
                        Tools::copyFile(infilename.c_str(), (fileto + ext_old).c_str());
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    };

    __trans(dirfrom, dirto);
    std::cout << dirfrom + " =>" << getExtName(fmt) << ": 转换" << fcount << "个文件, "
              << dcount << "个目录成功！\n   着法数量: "
              << movcount << ", 注释数量: " << remcount << ", 最大注释长度: " << remlenmax << std::endl;
}

void testTransDir(int fd, int td, int ff, int ft, int tf, int tt)
{
    std::vector<std::string> dirfroms{
        "c:\\棋谱\\示例文件",
        "c:\\棋谱\\象棋杀着大全",
        "c:\\棋谱\\疑难文件",
        "c:\\棋谱\\中国象棋棋谱大全"
    };
    std::vector<RecFormat> fmts{ RecFormat::XQF, RecFormat::PGN_ICCS, RecFormat::PGN_ZH, RecFormat::PGN_CC,
        RecFormat::BIN, RecFormat::JSON };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}

const std::wstring Instance::test()
{
    std::wstringstream wss{};    
    read("01.xqf"); 
    write("01.pgn_iccs");

    read("01.pgn_iccs");
    write("01.pgn_zh"); 

    read("01.pgn_zh"); 
    write("01.pgn_cc"); 
    
    read("01.pgn_cc");
    //write("01.bin"); 
    
    //read("01.bin");
    //write("01.json"); 

    //read("01.json");
    wss << toString(); 
    
    
    return wss.str();
}
}