#include "Instance.h"
#include "../json/json.h"
#include "move.h"
#include "board.h"
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
    : info_{ { L"Author", L"" },
        { L"Black", L"" },
        { L"BlackTeam", L"" },
        { L"Date", L"" },
        { L"ECCO", L"" },
        { L"Event", L"" },
        { L"FEN", L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1" },
        { L"Format", L"ZH" },
        { L"Game", L"Chinese Chess" },
        { L"Opening", L"" },
        { L"PlayType", L"" },
        { L"RMKWriter", L"" },
        { L"Red", L"" },
        { L"RedTeam", L"" },
        { L"Result", L"" },
        { L"Round", L"" },
        { L"Site", L"" },
        { L"Title", L"" },
        { L"Variation", L"" },
        { L"Version", L"" } }
    , board_{ std::make_shared<BoardSpace::Board>() }
    , rootMove_{ std::make_shared<Move>() }
    , currentMove_{ rootMove_ }
    , firstColor_{ PieceColor::RED }
{
}

void Instance::read(const std::string& infilename)
{
    RecFormat fmt{ getRecFormat(Tools::getExt(infilename)) };
    info_[L"Format"] = Tools::s2ws(getExtName(fmt));
    //getInstanceRecord(fmt)->read(infilename, *this);
    std::wcout << L"readFile finished!" << std::endl;
    setBoard();
    std::wcout << L"setBoard finished!" << std::endl;
    setMoves(fmt);
    std::wcout << L"setMoves finished!" << std::endl;
}

void Instance::write(const std::string& outfilename) const
{
    RecFormat fmt{ getRecFormat(Tools::getExt(outfilename)) };
    //getInstanceRecord(fmt)->write(outfilename, *this);
}

void Instance::setFEN(const std::wstring& pieceChars)
{
    info_[L"FEN"] = getFEN(pieceChars) + L" " + (firstColor_ == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
    std::wstring rfen{ info_[L"FEN"] };
    assert(getPieceChars(rfen.substr(0, rfen.find(L' '))) == pieceChars);
}

void Instance::setBoard()
{
    std::wstring rfen{ info_[L"FEN"] };
    board_->putPieces(getPieceChars(rfen.substr(0, rfen.find(L' '))));
}

// （rootMove）调用, 设置树节点的seat or zh'  // C++primer P512
void Instance::setMoves(const RecFormat fmt)
{
    std::function<void(Move&)> __setRemData = [&](const Move& move) {
        if (!move.remark_.empty()) {
            ++remCount;
            remLenMax = std::max(remLenMax, static_cast<int>(move.remark_.size()));
        }
    };

    std::function<void(Move&)> __set = [&](Move& move) {
        if (fmt == RecFormat::ICCS)
            move.setSeats(board_->getMoveSeatFromIccs(move.iccs_));
        else if (fmt == RecFormat::ZH || fmt == RecFormat::CC)
            move.setSeats(board_->getMoveSeatFromZh(move.zh_));
        else {
            move.setSeats(board_->getSeat(move.frowcol_), board_->getSeat(move.trowcol_));
        }
//assert(move.fseat_->piece());
#ifndef NDEBUG
        if (!move.fseat_->piece())
            std::wcout << board_->toString() << move.toString() << std::endl;
#endif

        if (fmt != RecFormat::ZH && fmt != RecFormat::CC)
            move.zh_ = board_->getZh(move.fseat_, move.tseat_);
        if (fmt != RecFormat::ICCS) //RecFormat::XQF RecFormat::BIN RecFormat::JSON
            move.iccs_ = board_->getIccs(move.fseat_, move.tseat_);

        assert(move.zh_.size() == 4);
        assert(move.iccs_.size() == 4);

        ++movCount;
        maxCol = std::max(maxCol, move.o_);
        maxRow = std::max(maxRow, move.n_);
        move.CC_Col_ = maxCol; // # 本着在视图中的列数
        __setRemData(move);

        move.done();
        if (move.next_)
            __set(*move.next_);
        move.undo();
        if (move.other_) {
            ++maxCol;
            __set(*move.other_);
        }
    };

    __setRemData(*rootMove_);
    if (rootMove_->next_)
        __set(*rootMove_->next_); // 驱动函数
}

//const PieceColor Instance::currentColor() const
//{
//    return currentMove_->n_ % 2 == 0 ? firstColor_ : PieceSpace::getOthColor(firstColor_);
//}

const bool Instance::isStart() const { return !currentMove_->prev_.lock(); }

const bool Instance::isLast() const { return !currentMove_->next_; }

// 基本走法
void Instance::go()
{
    if (!isLast()) {
        currentMove_ = currentMove_->next_->done();
    }
}

void Instance::back()
{
    if (!isStart()) {
        currentMove_->undo();
        currentMove_ = currentMove_->prev_.lock();
    }
}

//'移动到当前节点的另一变着'
void Instance::goOther()
{
    if (currentMove_->other_) {
        currentMove_->undo();
        currentMove_ = currentMove_->other_->done();
        //auto toMove = currentMove_->other_;
        //board_->back(*currentMove_);
        //board_->go(*toMove);
        //currentMove_ = toMove;
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
    setFEN(board_->getPieceChars());

    if (ct == ChangeType::EXCHANGE)
        firstColor_ = firstColor_ == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
    else {
        auto getChangeSeat = std::mem_fn(ct == ChangeType::ROTATE ? &BoardSpace::Board::getRotateSeat : &BoardSpace::Board::getSymmetrySeat);
        std::function<void(Move&)> __setSeat = [&](Move& move) {
            move.setSeats(getChangeSeat(this->board_, move.fseat_), getChangeSeat(this->board_, move.tseat_));
            if (move.next_)
                __setSeat(*move.next_);
            if (move.other_)
                __setSeat(*move.other_);
        };
        if (rootMove_->next_)
            __setSeat(*rootMove_->next_); // 驱动调用递归函数
    }

    if (ct != ChangeType::ROTATE)
        setMoves(RecFormat::BIN); //借用RecFormat::BIN
    for (auto& move : curmove->getPrevMoves())
        move->done();
}

const std::wstring Instance::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
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
    case RecFormat::ICCS:
        return ".pgn1";
    case RecFormat::ZH:
        return ".pgn2";
    case RecFormat::CC:
        return ".pgn3";
    default:
        return ".pgn3";
    }
}

const RecFormat getRecFormat(const std::string& ext)
{
    if (ext == ".xqf")
        return RecFormat::XQF;
    else if (ext == ".bin")
        return RecFormat::BIN;
    else if (ext == ".json")
        return RecFormat::JSON;
    else if (ext == ".pgn1")
        return RecFormat::ICCS;
    else if (ext == ".pgn2")
        return RecFormat::ZH;
    else if (ext == ".pgn3")
        return RecFormat::CC;
    else
        return RecFormat::CC;
}

void transDir(const std::string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    std::string extensions{ ".xqf.pgn1.pgn2.pgn3.bin.json" };
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
                        Instance ci{};
                        ci.read(infilename);
                        ci.write(fileto + getExtName(fmt));
                        //std::cout << fileto << std::endl;

                        movcount += ci.getMovCount();
                        remcount += ci.getRemCount();
                        if (remlenmax < ci.getRemLenMax())
                            remlenmax = ci.getRemLenMax();
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
    std::vector<RecFormat> fmts{ RecFormat::XQF, RecFormat::ICCS, RecFormat::ZH, RecFormat::CC,
        RecFormat::BIN, RecFormat::JSON };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}

const std::wstring Instance::test() const
{
    std::wstringstream wss{};
    for (const auto& fen : { L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR",
             L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" }) {
        auto pieceChars = getPieceChars(fen);
        board_->putPieces(pieceChars);
        wss << "fen:" << fen << "\nget:" << getFEN(pieceChars)
            << "\ngetChars:" << pieceChars << "\nboardGet:" << board_->getPieceChars() << L'\n';

        wss << L'\n' << board_->test() << L'\n';
    }
    //wss << toString_ICCSZH() << L'\n' << toString_ICCSZH(RecFormat::ICCS)
    //    << L'\n' << moveInfo() << toString();
    return wss.str();
}
}