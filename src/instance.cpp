#include "Instance.h"
#include "../json/json.h"
#include "Info.h"
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
    : format_{ RecFormat::XQF }
    , info_{ std::make_shared<InfoSpace::Info>() }
    , board_{ std::make_shared<BoardSpace::Board>() }
    , rootMove_{ std::make_shared<MoveSpace::RootMove>() }
    , currentMove_(rootMove_)
{
}

void Instance::read(const std::string& infilename)
{
    std::ifstream ifs(infilename);
    format_ = getRecFormat(Tools::getExt(infilename));
    info_->read(ifs, format_);
    board_->putPieces(info_->getPieceChars());
    std::wcout << L"setBoard finished!" << std::endl;
    rootMove_->read(ifs, format_, *board_);
    currentMove_ = rootMove_;
    std::wcout << L"readFile finished!" << std::endl;
}

void Instance::write(const std::string& outfilename)
{
    std::ofstream ofs(outfilename);
    format_ = getRecFormat(Tools::getExt(outfilename));
    info_->write(ofs, format_);
    rootMove_->write(ofs, format_);
}

const bool Instance::isStart() const { return !currentMove_->prev(); }

const bool Instance::isLast() const { return !currentMove_->next(); }

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
    auto curmove = currentMove_;
    backFirst();
    board_->changeSide(ct);
    info_->setFEN(board_->getPieceChars());

    if (ct != ChangeType::EXCHANGE) {
        auto getChangeSeat = std::mem_fn(ct == ChangeType::ROTATE ? &BoardSpace::Board::getRotateSeat : &BoardSpace::Board::getSymmetrySeat);
        std::function<void(MoveSpace::Move&)> __setSeat = [&](MoveSpace::Move& move) {
            move.setSeats(getChangeSeat(this->board_, move.fseat()), getChangeSeat(this->board_, move.tseat()));
            if (move.next())
                __setSeat(*move.next());
            if (move.other())
                __setSeat(*move.other());
        };
        if (rootMove_->next())
            __setSeat(*rootMove_->next()); // 驱动调用递归函数
    }

    rootMove_->setMoves(format_, *board_); //借用RecFormat::BIN
    for (auto& move : curmove->getPrevMoves())
        move->done();
}

const std::wstring Instance::toString() const
{
    std::wstringstream wss{};
    wss << board_->toString();
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
        return ".pgn3";
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

void Instance::transDir(const std::string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    std::string extensions{ ".xqf.pgn_iccs.pgn_zh.pgn_cc.bin.json" };
    std::string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + getExtName(fmt) };
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
                        this->read(infilename);
                        this->write(fileto + getExtName(fmt));
                        //std::cout << fileto << std::endl;

                        movcount += rootMove_->getMovCount();
                        remcount += rootMove_->getRemCount();
                        remlenmax = std::max(remlenmax, rootMove_->getRemLenMax());
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
    Instance ci{};
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex != fIndex)
                    ci.transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}

const std::wstring Instance::test() const
{
    std::wstringstream wss{};
    for (const auto& fen : { L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
             L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" }) {
        auto pieceChars = InfoSpace::getPieceChars(fen);
        board_->putPieces(pieceChars);
        wss << "fen:" << fen << "\nget:" << InfoSpace::getFEN(pieceChars)
            << "\ngetChars:" << pieceChars << "\nboardGet:" << board_->getPieceChars() << L'\n';

        wss << L'\n' << board_->test() << L'\n';
    }
    return wss.str();
}
}