#include "move.h"
#include "../json/json.h"
#include "info.h"
#include "board.h"
#include "instance.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace MoveSpace {

const std::shared_ptr<Move>& Move::setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
    return std::move(shared_from_this());
}

const std::shared_ptr<Move>& Move::setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>,
    const std::shared_ptr<SeatSpace::Seat>>& seats)
{
    return setSeats(seats.first, seats.second);
}

const std::shared_ptr<Move>& Move::addNext()
{
    auto next = std::make_shared<Move>();
    next->setNextNo(nextNo_ + 1); // 步序号
    next->setOtherNo(otherNo_); // 变着层数
    next->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return next_ = next;
}

const std::shared_ptr<Move>& Move::addOther()
{
    auto other = std::make_shared<Move>();
    other->setNextNo(nextNo_); // 与premove的步数相同
    other->setOtherNo(otherNo_ + 1); // 变着层数
    other->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return other_ = other;
}

std::vector<std::shared_ptr<Move>> Move::getPrevMoves()
{
    std::shared_ptr<Move> this_move{ shared_from_this() }, prev_move{};
    std::vector<std::shared_ptr<Move>> moves{ this_move };
    while ((prev_move = this_move->prev()) && prev_move->prev()) { // 排除rootMove
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const std::shared_ptr<Move>& Move::done()
{
    eatPie_ = fseat_->to(tseat_);
    return next_;
}

const std::shared_ptr<Move>& Move::undo()
{
    tseat_->to(fseat_, eatPie_);
    return std::move(prev());
}

const std::wstring Move::toString() const
{
    std::wstringstream wss{};
    wss << fseat()->toString() << L'>' << tseat()->toString() << L'-' << (eatPie() ? eatPie()->name() : L' ')
        << iccs() << L' ' << zh() << L' ' << nextNo() << L' ' << otherNo() << L' ' << CC_ColNo() << L' ' << remark();
    return wss.str();
}

void RootMove::read(std::ifstream& ifs, RecFormat fmt, const BoardSpace::Board& board)
{
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(ifs);
        break;
    case RecFormat::PGN_ICCS:
        readPGN_ICCSZH(ifs, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        readPGN_ICCSZH(ifs, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        readPGN_CC(ifs);
        break;
    case RecFormat::BIN:
        readBIN(ifs);
        break;
    case RecFormat::JSON:
        readJSON(ifs);
        break;
    default:
        break;
    }
    setMoves(fmt, board);
}

void RootMove::write(std::ofstream& ofs, RecFormat fmt) const
{
    switch (fmt) {
    case RecFormat::XQF:
        writeXQF(ofs);
        break;
    case RecFormat::PGN_ICCS:
        writePGN_ICCSZH(ofs, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        writePGN_ICCSZH(ofs, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        writePGN_CC(ofs);
        break;
    case RecFormat::BIN:
        writeBIN(ofs);
        break;
    case RecFormat::JSON:
        writeJSON(ofs);
        break;
    default:
        break;
    }
}

// （rootMove）调用, 设置树节点的seat or zh'  // C++primer P512
void RootMove::setMoves(RecFormat fmt, const BoardSpace::Board& board)
{
    std::function<void(Move&)> __setRemData = [&](const Move& move) {
        if (!move.remark().empty()) {
            ++remCount;
            remLenMax = std::max(remLenMax, static_cast<int>(move.remark().size()));
        }
    };

    std::function<void(Move&)> __set = [&](Move& move) {
        if (fmt == RecFormat::PGN_ICCS)
            move.setSeats(board.getMoveSeatFromIccs(move.iccs()));
        else if (fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC)
            move.setSeats(board.getMoveSeatFromZh(move.zh()));
        else {
            move.setSeats(board.getSeat(move.frowcol()), board.getSeat(move.trowcol()));
        }
//assert(move.fseat()->piece());
#ifndef NDEBUG
        if (!move.fseat()->piece())
            std::wcout << board.toString() << move.toString() << std::endl;
#endif

        if (fmt != RecFormat::PGN_ZH && fmt != RecFormat::PGN_CC)
            move.setZh(board.getZh(move.fseat(), move.tseat()));
        if (fmt != RecFormat::PGN_ICCS) //RecFormat::XQF RecFormat::BIN RecFormat::JSON
            move.setIccs(board.getIccs(move.fseat(), move.tseat()));

        assert(move.zh().size() == 4);
        assert(move.iccs().size() == 4);

        ++movCount;
        maxCol = std::max(maxCol, move.otherNo());
        maxRow = std::max(maxRow, move.nextNo());
        move.setCC_ColNo(maxCol); // # 本着在视图中的列数
        __setRemData(move);

        move.done();
        if (move.next())
            __set(*move.next());
        move.undo();
        if (move.other()) {
            ++maxCol;
            __set(*move.other());
        }
    };

    __setRemData(*this);
    if (this->next())
        __set(*this->next()); // 驱动函数
}

const std::wstring RootMove::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}


void RootMove::readXQF(std::ifstream& ifs)
{
    char data[4]{};
    std::function<void(Move&)> __read = [&](Move& move) {
        auto __readbytes = [&](char* byteStr, int size) {
            int pos = ifs.tellg();
            ifs.read(byteStr, size);
            if (InfoSpace::key.version > 10) // '字节串解密'
                for (int i = 0; i != size; ++i)
                    byteStr[i] = static_cast<unsigned char>(byteStr[i]) - static_cast<unsigned char>(InfoSpace::key.F32Keys[(pos + i) % 32]);
        };
        auto __readremarksize = [&]() {
            __readbytes(data, 4);
            int size{ *(int*)(unsigned char*)data };
            return size - InfoSpace::key.KeyRMKSize;
        };

        __readbytes(data, 4);
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        unsigned char fcolrow = static_cast<unsigned char>(data[0]) - static_cast<unsigned char>(0X18 + InfoSpace::key.KeyXYf),
                      tcolrow = static_cast<unsigned char>(data[1]) - static_cast<unsigned char>(0X20 + InfoSpace::key.KeyXYt);

        if (fcolrow <= 89 && tcolrow <= 89) { // col<=8, row<=9
            move.setFrowcol((fcolrow % 10) << 4 | (fcolrow / 10));
            move.setTrowcol((tcolrow % 10) << 4 | (tcolrow / 10));
        }

        unsigned char ChildTag = data[2];
        int RemarkSize = 0;
        if (InfoSpace::key.version <= 0x0A) {
            unsigned char b = 0;
            if (ChildTag & 0xF0)
                b = b | 0x80;
            if (ChildTag & 0x0F)
                b = b | 0x40;
            ChildTag = b;
            RemarkSize = __readremarksize();
        } else {
            ChildTag = ChildTag & 0xE0;
            if (ChildTag & 0x20)
                RemarkSize = __readremarksize();
        }
        if (RemarkSize > 0) { // # 如果有注解
            char rem[RemarkSize + 1]{};
            __readbytes(rem, RemarkSize);
            move.setRemark(Tools::s2ws(rem));

            //std::wcout << move.remark() << std::endl;
        }

        if (ChildTag & 0x80) //# 有左子树
            __read(*move.addNext());
        if (ChildTag & 0x40) // # 有右子树
            __read(*move.addOther());
    };

    ifs.seekg(1024);
    __read(*this);
}

void RootMove::writeXQF(std::ofstream& ofs) const {}

const std::wstring RootMove::getMoveStr(std::ifstream& ifs) const
{
    std::stringstream ss{};
    std::string line{};
    ifs >> std::noskipws;
    while (ifs >> line) // 以空行为分割，接info read之后
        ss << line;
    return Tools::s2ws(ss.str());
}

void RootMove::readPGN_ICCSZH(std::ifstream& ifs, RecFormat fmt)
{
    const std::wstring moveStr{ getMoveStr(ifs) };
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::PGN_ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                                 : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](std::shared_ptr<Move> move, const std::wstring mvstr, bool isOther) { //# 非递归
        for (std::wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != std::wsregex_iterator{}; ++p) {
            auto newMove = isOther ? move->addOther() : move->addNext();
            fmt == RecFormat::PGN_ZH ? (newMove->setZh((*p)[1])) : (newMove->setIccs((*p)[1]));
            newMove->setRemark((*p)[2]);
            isOther = false; // # 仅第一步可为other，后续全为next
            move = newMove;
        }
        return move;
    };

    std::shared_ptr<Move> move{};
    std::vector<std::shared_ptr<Move>> othMoves{ std::make_shared<Move>(*this) };
    std::wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    std::wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    std::wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        this->setRemark(wsm.str(1));
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {
        //std::wcout << *wtleft << L"\n---------------------------------------------\n" << std::endl;
        std::wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            //std::wcout << *wtright << L"\n---------------------------------------------\n" << std::endl;
            move = setMoves(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(move);
        isOther = true;
    }
}

void RootMove::writePGN_ICCSZH(std::ofstream& ofs, RecFormat fmt) const
{
    std::wstringstream wss{};
    std::function<void(const Move&)> __remark = [&](const Move& move) {
        if (!move.remark().empty())
            wss << (L"\n{" + move.remark() + L"}\n");
    };

    std::function<void(const Move&, bool)> __moveStr = [&](const Move& move, bool isOther) {
        std::wstring boutNum{ std::to_wstring((move.nextNo() + 1) / 2) };
        bool isEven{ move.nextNo() % 2 == 0 };
        wss << (isOther ? L"(" + boutNum + L". " + (isEven ? L"... " : L"") : (isEven ? L" " : boutNum + L". "))
            << (fmt == RecFormat::PGN_ZH ? move.zh() : move.iccs()) << L' ';
        __remark(move);
        if (move.other()) {
            __moveStr(*move.other(), true);
            wss << L") ";
        }
        if (move.next())
            __moveStr(*move.next(), false);
    };

    __remark(*this);
    if (this->next())
        __moveStr(*this->next(), false);
    ofs << Tools::ws2s(wss.str());
}

void RootMove::readPGN_CC(std::ifstream& ifs)
{
    const std::wstring moveStr{ getMoveStr(ifs) };
    auto pos = moveStr.find(L"\n(");
    std::wstring remStr{ pos < moveStr.size() ? moveStr.substr(pos) : L"" };
    std::wregex spfat{ LR"(\n)" }, mstrfat{ LR"(.{5})" },
        movefat{ LR"(([^…　]{4}[…　]))" }, remfat{ LR"(\(\s*(\d+,\d+)\): \{([\s\S]*?)\})" };
    int row{ 0 };
    std::vector<std::vector<std::wstring>> movv{};
    for (std::wsregex_token_iterator msp{ moveStr.begin(), moveStr.end(), spfat, -1 }, end{}; msp != end; ++msp)
        if (row++ % 2 == 0) {
            std::vector<std::wstring> linev{};
            for (std::wsregex_token_iterator mp{ (*msp).first, (*msp).second, mstrfat, 0 }; mp != end; ++mp)
                linev.push_back(*mp);
            movv.push_back(linev);
        }
    std::map<std::wstring, std::wstring> remm{};
    for (std::wsregex_iterator rp{ remStr.begin(), remStr.end(), remfat }; rp != std::wsregex_iterator{}; ++rp)
        remm[(*rp)[1]] = (*rp)[2];

    auto __setRem = [&](Move& move, int row, int col) {
        move.setRemark(remm[std::to_wstring(row) + L',' + std::to_wstring(col)]);
    };
    std::function<void(Move&, int, int, bool)> __read = [&](Move& move, int row, int col, bool isOther) {
        std::wstring zh{ movv[row][col] };
        if (regex_match(zh, movefat)) {
            auto newMove = isOther ? move.addOther() : move.addNext();
            newMove->setZh(zh.substr(0, 4));
            __setRem(*newMove, row, col);
            if (zh.back() == L'…')
                __read(*newMove, row, col + 1, true);
            if (int(movv.size()) - 1 > row)
                __read(*newMove, row + 1, col, false);
        } else if (isOther) {
            while (movv[row][col][0] == L'…')
                ++col;
            __read(move, row, col, true);
        }
    };

    __setRem(*this, 0, 0);
    if (!movv.empty())
        __read(*this, 1, 0, false);
}

void RootMove::writePGN_CC(std::ofstream& ofs) const
{
    std::wstringstream remStrs{};
    std::wstring lstr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, lstr);
    std::function<void(const Move&)> __setChar = [&](const Move& move) {
        int firstcol{ move.CC_ColNo() * 5 }, row{ move.nextNo() * 2 };
        lineStr.at(row).replace(firstcol, 4, move.zh());
        if (!move.remark().empty())
            remStrs << L"(" << move.nextNo() << L"," << move.CC_ColNo() << L"): {" << move.remark() << L"}\n";
        if (move.next()) {
            lineStr.at(row + 1).at(firstcol + 2) = L'↓';
            __setChar(*move.next());
        }
        if (move.other()) {
            int fcol{ firstcol + 4 }, num{ move.other()->CC_ColNo() * 5 - fcol };
            lineStr.at(row).replace(fcol, num, std::wstring(num, L'…'));
            __setChar(*move.other());
        }
    };

    __setChar(*this);
    std::wstringstream wss{};
    lineStr.front().replace(0, 2, L"开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str(); // << moveInfo();
    ofs << Tools::ws2s(wss.str());
}

void RootMove::readBIN(std::ifstream& ifs)
{
    std::function<void(Move&)> __read = [&](Move& move) {
        char frowcol{}, trowcol{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        ifs.get(frowcol).get(trowcol).get(tag);
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        //move.setSeats(board.getSeat(frowcol), board.getSeat(trowcol));
        hasNext = tag & 0x80;
        hasOther = tag & 0x40;
        hasRemark = tag & 0x08;
        if (hasRemark) {
            char len[sizeof(int)]{};
            ifs.read(len, sizeof(int));
            int length{ *(int*)len };

            char rem[length + 1]{};
            ifs.read(rem, length);
            move.setRemark(Tools::s2ws(rem));
        }
        if (hasNext)
            __read(*move.addNext());
        if (hasOther)
            __read(*move.addOther());
    };

    __read(*this);
}

void RootMove::writeBIN(std::ofstream& ofs) const
{
    std::function<void(const Move&)> __write = [&](const Move& move) {
        std::string remark{ Tools::ws2s(move.remark()) };
        ofs.put(static_cast<char>(move.fseat()->rowcolValue()))
            .put(static_cast<char>(move.tseat()->rowcolValue()))
            .put(static_cast<char>((move.next() ? 0x80 : 0x00) | (move.other() ? 0x40 : 0x00) | (remark.empty() ? 0x00 : 0x08)));
        if (!remark.empty()) {
            int len = remark.size();
            ofs.write((char*)&len, sizeof(int)).write(remark.c_str(), len);
        }
        if (move.next())
            __write(*move.next());
        if (move.other())
            __write(*move.other());
    };

    __write(*this);
}

void RootMove::readJSON(std::ifstream& ifs)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    std::function<void(Move&, Json::Value&)> __read = [&](Move& move, Json::Value& item) {
        int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        //move.setSeats(board.getSeat(frowcol), board.getSeat(trowcol));
        if (item.isMember("r"))
            move.setRemark(Tools::s2ws(item["r"].asString()));
        if (item.isMember("n")) //# 有左子树
            __read(*move.addNext(), item["n"]);
        if (item.isMember("o")) // # 有右子树
            __read(*move.addOther(), item["o"]);
    };

    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __read(*this, rootItem);
}

void RootMove::writeJSON(std::ofstream& ofs) const
{
    std::function<Json::Value(const Move&)> __writeItem = [&](const Move& move) {
        Json::Value item{};
        item["f"] = move.fseat()->rowcolValue();
        item["t"] = move.tseat()->rowcolValue();
        if (!move.remark().empty())
            item["r"] = Tools::ws2s(move.remark());
        if (move.next())
            item["n"] = __writeItem(*move.next());
        if (move.other())
            item["o"] = __writeItem(*move.other());
        return std::move(item);
    };

    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    root["moves"] = __writeItem(*this);
    writer->write(root, &ofs);
}

/*
void XQFInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    std::ifstream ifs(infilename, std::ios_base::binary);
    auto __subbyte = [](const int a, const int b) { return (256 + a - b) % 256; };
    std::function<unsigned char(unsigned char, unsigned char)> __calkey = [](unsigned char bKey, unsigned char cKey) {
        return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey % 256; // 保持为<256
    };
    std::wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    char Signature[3], Version_xqf[1], headKeyMask[1], ProductId[4], //文件标记'XQ'=$5158/版本/加密掩码/产品(厂商的产品号)
        headKeyOrA[1], headKeyOrB[1], headKeyOrC[1], headKeyOrD[1],
        headKeysSum[1], headKeyXY[1], headKeyXYf[1], headKeyXYt[1], // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headQiziXY[32], // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)
        PlayStepNo[2], headWhoPlay[1], headPlayResult[1], PlayNodes[4], PTreePos[4], Reserved1[4],
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16], TitleA[65], TitleB[65], //对局类型(开,中,残等)
        Event[65], Date[17], Site[17], Red[17], Black[17],
        Opening[65], Redtime[17], Blktime[17], Reservedh[33],
        RMKWriter[17], Author[17]; // 棋谱评论员/文件的作者

    ifs.read(Signature, 2).read(Version_xqf, 1).read(headKeyMask, 1).read(ProductId, 4); // = 8 bytes
    ifs.read(headKeyOrA, 1).read(headKeyOrB, 1).read(headKeyOrC, 1).read(headKeyOrD, 1).read(headKeysSum, 1).read(headKeyXY, 1).read(headKeyXYf, 1).read(headKeyXYt, 1); // = 16 bytes
    ifs.read(headQiziXY, 32); // = 48 bytes
    ifs.read(PlayStepNo, 2).read(headWhoPlay, 1).read(headPlayResult, 1).read(PlayNodes, 4).read(PTreePos, 4).read(Reserved1, 4); // = 64 bytes
    ifs.read(headCodeA_H, 16).read(TitleA, 64).read(TitleB, 64); // 80 + 128 = 208 bytes
    ifs.read(Event, 64).read(Date, 16).read(Site, 16).read(Red, 16).read(Black, 16); // = 336 bytes
    ifs.read(Opening, 64).read(Redtime, 16).read(Blktime, 16).read(Reservedh, 32); // = 464 bytes
    ifs.read(RMKWriter, 16).read(Author, 16); // = 496 bytes
    ifs.ignore(528); // = 1024 bytes

    int version{ Version_xqf[0] };
    std::map<std::wstring, std::wstring> info{};
    info[L"Version_xqf"] = std::to_wstring(version);
    info[L"Result"] = (std::map<char, std::wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult[0]];
    info[L"PlayType"] = (std::map<char, std::wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]];
    info[L"TitleA"] = Tools::s2ws(TitleA);
    info[L"Event"] = Tools::s2ws(Event);
    info[L"Date"] = Tools::s2ws(Date);
    info[L"Site"] = Tools::s2ws(Site);
    info[L"Red"] = Tools::s2ws(Red);
    info[L"Black"] = Tools::s2ws(Black);
    info[L"Opening"] = Tools::s2ws(Opening);
    info[L"RMKWriter"] = Tools::s2ws(RMKWriter);
    info[L"Author"] = Tools::s2ws(Author);

    unsigned char head_KeyXY{ (unsigned char)(headKeyXY[0]) }, head_KeyXYf{ (unsigned char)(headKeyXYf[0]) },
        head_KeyXYt{ (unsigned char)(headKeyXYt[0]) }, head_KeysSum{ (unsigned char)(headKeysSum[0]) };
    unsigned char* head_QiziXY{ (unsigned char*)headQiziXY };

    if (Signature[0] != 0x58 || Signature[1] != 0x51)
        std::wcout << Tools::s2ws(Signature) << L" 文件标记不对。Signature != (0x58, 0x51)\n";
    if ((head_KeysSum + head_KeyXY + head_KeyXYf + head_KeyXYt) % 256 != 0)
        std::wcout << head_KeysSum << head_KeyXY << head_KeyXYf << head_KeyXYt << L" 检查密码校验和不对，不等于0。\n";
    if (version > 18)
        std::wcout << version << L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

    unsigned char KeyXY, KeyXYf, KeyXYt;
    int KeyRMKSize;
    if (version <= 10) { // 兼容1.0以前的版本
        KeyXY = KeyXYf = KeyXYt = 0;
        KeyRMKSize = 0;
    } else {
        KeyXY = __calkey(head_KeyXY, head_KeyXY);
        KeyXYf = __calkey(head_KeyXYf, KeyXY);
        KeyXYt = __calkey(head_KeyXYt, KeyXYf);
        KeyRMKSize = ((head_KeysSum * 256 + head_KeyXY) % 32000) + 767; // % 65536
        if (version >= 12) { // 棋子位置循环移动
            std::vector<unsigned char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // head_QiziXY 不是数组，不能用
            for (int i = 0; i != 32; ++i)
                head_QiziXY[(i + KeyXY + 1) % 32] = Qixy[i];
        }
        for (int i = 0; i != 32; ++i) // 棋子位置解密
            head_QiziXY[i] = __subbyte(head_QiziXY[i], KeyXY); // 保持为8位无符号整数，<256
    }

    char KeyBytes[4];
    KeyBytes[0] = (headKeysSum[0] & headKeyMask[0]) | headKeyOrA[0];
    KeyBytes[1] = (headKeyXY[0] & headKeyMask[0]) | headKeyOrB[0];
    KeyBytes[2] = (headKeyXYf[0] & headKeyMask[0]) | headKeyOrC[0];
    KeyBytes[3] = (headKeyXYt[0] & headKeyMask[0]) | headKeyOrD[0];
    std::string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    std::vector<int> F32Keys(32, 0);
    for (int i = 0; i != 32; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)
    std::wstring pieceChars(90, L'_');
    for (int i = 0; i != 32; ++i) {
        int xy = head_QiziXY[i];
        if (xy < 90) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    info[L"FEN"] = getFEN(pieceChars);
    //std::wcout << info[L"FEN"] << std::endl;

    std::function<void(Move&)> __read = [&](Move& move) {
        //auto __byteToSeat = [&](int a, int b) {
        //    int xy = __subbyte(a, b);
        //    return getSeat(xy % 10, xy / 10);
        //};
        auto __readbytes = [&](char* byteStr, const int size) {
            int pos = ifs.tellg();
            ifs.read(byteStr, size);
            if (version > 10) // '字节串解密'
                for (int i = 0; i != size; ++i)
                    byteStr[i] = __subbyte((unsigned char)(byteStr[i]), F32Keys[(pos + i) % 32]);
        };
        auto __readremarksize = [&]() {
            char byteSize[4]{}; // 一定要初始化:{}
            __readbytes(byteSize, 4);
            int size{ *(int*)(unsigned char*)byteSize };
            return size - KeyRMKSize;
        };

        char data[4]{};
        __readbytes(data, 4);
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow{ __subbyte(data[0], 0X18 + KeyXYf) }, tcolrow{ __subbyte(data[1], 0X20 + KeyXYt) };

        if (fcolrow <= 89 && tcolrow <= 89) // col<=8, row<=9
        {
            move.frowcol_ = (fcolrow % 10) << 4 | fcolrow / 10;
            move.trowcol_ = (tcolrow % 10) << 4 | tcolrow / 10;
        }
        //move.setSeats(board_->getSeat(fcolrow % 10, fcolrow / 10), board_->getSeat(tcolrow % 10, tcolrow / 10));
        //else // rootMove存在>=90情况
        //move.setSeats(board_->getSeat(0, 0), board_->getSeat(0, 0));

        char ChildTag = data[2];
        int RemarkSize = 0;
        if (version <= 0x0A) {
            char b = 0;
            if (ChildTag & 0xF0)
                b = b | 0x80;
            if (ChildTag & 0x0F)
                b = b | 0x40;
            ChildTag = b;
            RemarkSize = __readremarksize();
        } else {
            ChildTag = ChildTag & 0xE0;
            if (ChildTag & 0x20)
                RemarkSize = __readremarksize();
        }
        if (RemarkSize > 0) { // # 如果有注解
            char rem[RemarkSize + 1]{};
            __readbytes(rem, RemarkSize);
            move.remark_ = Tools::s2ws(rem);

            //std::wcout << move.remark_ << std::endl;
        }

        if (ChildTag & 0x80) //# 有左子树
            __read(*move.addNext());
        if (ChildTag & 0x40) // # 有右子树
            __read(*move.addOther());
    };

    Move rootMove{};
    __read(rootMove);
    setInfo(info);
    setRootMove(std::make_shared<Move>(rootMove));
}

void XQFInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
}

void PGN_ICCSInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    __readMove_ICCSZH(__readInfo_getMoveStr(infilename, instance), instance, RecFormat::PGN_ICCS);
}

void PGN_ZHInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    __readMove_ICCSZH(__readInfo_getMoveStr(infilename, instance), instance, RecFormat::PGN_ZH);
}

void PGN_CCInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    __readMove_CC(__readInfo_getMoveStr(infilename, instance), instance);
}

const std::wstring InstanceRecord::__readInfo_getMoveStr(const std::string& infilename, Instance& instance)
{
    std::wstring pgnTxt{ Tools::readTxt(infilename) };
    auto pos = pgnTxt.find(L"》");
    std::wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    std::wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    std::map<std::wstring, std::wstring> info{};
    for (std::wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != std::wsregex_iterator{}; ++p)
        info[(*p)[1]] = (*p)[2];
    setInfo(info);
    return moveStr;
}

void InstanceRecord::__readMove_ICCSZH(const std::wstring& moveStr, Instance& instance, const RecFormat fmt)
{
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::PGN_ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                             : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](std::shared_ptr<Move> move, const std::wstring mvstr, bool isOther) { //# 非递归
        for (std::wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != std::wsregex_iterator{}; ++p) {
            auto newMove = isOther ? move->addOther() : move->addNext();
            fmt == RecFormat::PGN_ZH ? (newMove->zh_ = (*p)[1]) : (newMove->iccs_ = (*p)[1]);
            newMove->remark_ = (*p)[2];
            isOther = false; // # 仅第一步可为other，后续全为next
            move = newMove;
        }
        return move;
    };

    std::shared_ptr<Move> rootMove{}, move{};
    std::vector<std::shared_ptr<Move>> othMoves{ rootMove };
    std::wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    std::wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    std::wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        rootMove->remark_ = wsm.str(1);
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {
        //std::wcout << *wtleft << L"\n---------------------------------------------\n" << std::endl;
        std::wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            //std::wcout << *wtright << L"\n---------------------------------------------\n" << std::endl;
            move = setMoves(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(move);
        isOther = true;
    }
    setRootMove(rootMove);
}

void InstanceRecord::__readMove_CC(const std::wstring& moveStr, Instance& instance)
{
    auto pos = moveStr.find(L"\n(");
    std::wstring remStr{ pos < moveStr.size() ? moveStr.substr(pos) : L"" };
    std::wregex spfat{ LR"(\n)" }, mstrfat{ LR"(.{5})" },
        movefat{ LR"(([^…　]{4}[…　]))" }, remfat{ LR"(\(\s*(\d+,\d+)\): \{([\s\S]*?)\})" };
    int row{ 0 };
    std::vector<std::vector<std::wstring>> movv{};
    for (std::wsregex_token_iterator msp{ moveStr.begin(), moveStr.end(), spfat, -1 }, end{}; msp != end; ++msp)
        if (row++ % 2 == 0) {
            std::vector<std::wstring> linev{};
            for (std::wsregex_token_iterator mp{ (*msp).first, (*msp).second, mstrfat, 0 }; mp != end; ++mp)
                linev.push_back(*mp);
            movv.push_back(linev);
        }
    std::map<std::wstring, std::wstring> remm{};
    for (std::wsregex_iterator rp{ remStr.begin(), remStr.end(), remfat }; rp != std::wsregex_iterator{}; ++rp)
        remm[(*rp)[1]] = (*rp)[2];

    auto __setRem = [&](Move& move, int row, int col) {
        move.remark_ = remm[std::to_wstring(row) + L',' + std::to_wstring(col)];
    };
    std::function<void(Move&, int, int, bool)> __read = [&](Move& move, int row, int col, bool isOther) {
        std::wstring zh{ movv[row][col] };
        if (regex_match(zh, movefat)) {
            auto newMove = isOther ? move.addOther() : move.addNext();
            newMove->zh_ = zh.substr(0, 4);
            __setRem(*newMove, row, col);
            if (zh.back() == L'…')
                __read(*newMove, row, col + 1, true);
            if (int(movv.size()) - 1 > row)
                __read(*newMove, row + 1, col, false);
        } else if (isOther) {
            while (movv[row][col][0] == L'…')
                ++col;
            __read(move, row, col, true);
        }
    };

    Move rootMove{};
    __setRem(rootMove, 0, 0);
    if (!movv.empty())
        __read(rootMove, 1, 0, false);
    setRootMove(std::make_shared<Move>(rootMove));
}

void BINInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    std::ifstream ifs(infilename, std::ios_base::binary);
    char size{}, klen{}, vlen{};
    std::map<std::wstring, std::wstring> info{};
    ifs.get(size);
    for (int i = 0; i != size; ++i) {
        ifs.get(klen);
        char key[klen + 1]{};
        ifs.read(key, klen);
        ifs.get(vlen);
        char value[vlen + 1]{};
        ifs.read(value, vlen);
        info[Tools::s2ws(key)] = Tools::s2ws(value);
    }
    std::function<void(Move&)> __read = [&](Move& move) {
        char frowcol{}, trowcol{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        ifs.get(frowcol).get(trowcol).get(tag);
        move.frowcol_ = frowcol;
        move.trowcol_ = trowcol;
        //move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        hasNext = tag & 0x80;
        hasOther = tag & 0x40;
        hasRemark = tag & 0x08;
        if (hasRemark) {
            char len[sizeof(int)]{};
            ifs.read(len, sizeof(int));
            int length{ *(int*)len };

            char rem[length + 1]{};
            ifs.read(rem, length);
            move.remark_ = Tools::s2ws(rem);
        }
        if (hasNext)
            __read(*move.addNext());
        if (hasOther)
            __read(*move.addOther());
    };

    Move rootMove{};
    __read(rootMove);
    setInfo(info);
    setRootMove(std::make_shared<Move>(rootMove));
}

void JSONInstanceRecord::read(const std::string& infilename, Instance& instance)
{
    std::ifstream ifs(infilename);
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    Json::Value infoItem{ root["info"] };
    std::map<std::wstring, std::wstring> info{};
    for (auto& key : infoItem.getMemberNames())
        info[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    std::function<void(Move&, Json::Value&)> __read = [&](Move& move, Json::Value& item) {
        int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
        move.frowcol_ = frowcol;
        move.trowcol_ = trowcol;
        //move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        if (item.isMember("r"))
            move.remark_ = Tools::s2ws(item["r"].asString());
        if (item.isMember("n")) //# 有左子树
            __read(*move.addNext(), item["n"]);
        if (item.isMember("o")) // # 有右子树
            __read(*move.addOther(), item["o"]);
    };

    Json::Value rootItem{ root["moves"] };
    Move rootMove{};
    if (!rootItem.isNull())
        __read(rootMove, rootItem);
    setRootMove(std::make_shared<Move>(rootMove));
}

void PGN_ICCSInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_ICCSZH(instance, RecFormat::PGN_ICCS));
}

void PGN_ZHInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_ICCSZH(instance, RecFormat::PGN_ZH));
}

void PGN_CCInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_CC(instance));
}

const std::wstring InstanceRecord::__getPGNInfo(const Instance& instance) const
{
    std::wstringstream wss{};
    auto& info = getInfo();
    for (auto& kv : info)
        wss << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
    return wss.str() + L"》";
}

const std::wstring InstanceRecord::__getPGNTxt_ICCSZH(const Instance& instance, const RecFormat fmt) const
{
    std::wstringstream wss{};
    std::function<void(const Move&)> __remark = [&](const Move& move) {
        if (!move.remark_.empty())
            wss << (L"\n{" + move.remark_ + L"}\n");
    };

    std::function<void(const Move&, bool)> __moveStr = [&](const Move& move, bool isOther) {
        std::wstring boutNum{ std::to_wstring((move.nextNo_ + 1) / 2) };
        bool isEven{ move.nextNo_ % 2 == 0 };
        wss << (isOther ? L"(" + boutNum + L". " + (isEven ? L"... " : L"") : (isEven ? L" " : boutNum + L". "))
            << (fmt == RecFormat::PGN_ZH ? move.zh_ : move.iccs_) << L' ';
        __remark(move);
        if (move.other_) {
            __moveStr(*move.other_, true);
            wss << L") ";
        }
        if (move.next_)
            __moveStr(*move.next_, false);
    };

    auto& rootMove = getRootMove();
    __remark(*rootMove);
    if (rootMove->next_)
        __moveStr(*rootMove->next_, false);
    return wss.str();
}

const std::wstring InstanceRecord::__getPGNTxt_CC(const Instance& instance) const
{
    std::wstringstream remStrs{};
    std::wstring lstr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, lstr);
    std::function<void(const Move&)> __setChar = [&](const Move& move) {
        int firstcol{ move.CC_Col_ * 5 }, row{ move.nextNo_ * 2 };
        lineStr.at(row).replace(firstcol, 4, move.zh_);
        if (!move.remark_.empty())
            remStrs << L"(" << move.nextNo_ << L"," << move.CC_Col_ << L"): {" << move.remark_ << L"}\n";
        if (move.next_) {
            lineStr.at(row + 1).at(firstcol + 2) = L'↓';
            __setChar(*move.next_);
        }
        if (move.other_) {
            int fcol{ firstcol + 4 }, num{ move.other_->CC_Col_ * 5 - fcol };
            lineStr.at(row).replace(fcol, num, std::wstring(num, L'…'));
            __setChar(*move.other_);
        }
    };

    auto& rootMove = getRootMove();
    __setChar(*rootMove);
    std::wstringstream wss{};
    lineStr.front().replace(0, 2, L"开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << moveInfo();
    return wss.str();
}

void BINInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
    std::ofstream ofs(outfilename, std::ios_base::binary);
    auto& info = getInfo();
    ofs.put(char(info.size()));
    for (auto& kv : info) {
        std::string key{ Tools::ws2s(kv.first) }, value{ Tools::ws2s(kv.second) };
        char klen{ char(key.size()) }, vlen{ char(value.size()) };
        ofs.put(klen).write(key.c_str(), klen).put(vlen).write(value.c_str(), vlen);
    }
    std::function<void(const Move&)> __write = [&](const Move& move) {
        std::string remark{ Tools::ws2s(move.remark_) };
        int len{ int(remark.size()) };
        ofs.put(char(move.fseat_->rowcolValue())).put(char(move.tseat_->rowcolValue()));
        ofs.put(char(move.next_ ? 0x80 : 0x00) | char(move.other_ ? 0x40 : 0x00) | char(len > 0 ? 0x08 : 0x00));
        if (len > 0)
            ofs.write((char*)&len, sizeof(int)).write(remark.c_str(), len);
        if (move.next_)
            __write(*move.next_);
        if (move.other_)
            __write(*move.other_);
    };

    __write(*getRootMove());
}

void JSONInstanceRecord::write(const std::string& outfilename, const Instance& instance) const
{
    std::ofstream ofs(outfilename);
    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value infoItem;
    auto& info = getInfo();
    for (auto& kv : info)
        infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
    root["info"] = infoItem;

    std::function<Json::Value(const Move&)> __writeItem = [&](const Move& move) {
        Json::Value item{};
        item["f"] = move.fseat_->rowcolValue();
        item["t"] = move.tseat_->rowcolValue();
        if (!move.remark_.empty())
            item["r"] = Tools::ws2s(move.remark_);
        if (move.next_)
            item["n"] = __writeItem(*move.next_);
        if (move.other_)
            item["o"] = __writeItem(*move.other_);
        return std::move(item);
    };

    root["moves"] = __writeItem(*getRootMove());
    writer->write(root, &ofs);
}
*/
}