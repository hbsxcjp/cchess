#include "Info.h"
#include "../json/json.h"
#include "board.h"
#include "instance.h"
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

namespace InfoSpace {

// Info
Info::Info()
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
    , infoRecord_{ std::make_shared<PGN_ZHInfoRecord>() }
{
}

void Info::read(std::ifstream& ifs, RecFormat fmt)
{
    info_ = getInfoRecord(fmt)->read(infilename);
    std::wcout << L"readFile finished!" << toString() << std::endl;
}

void Info::write(std::ofstream& ofs, RecFormat fmt) const
{
    getInfoRecord(fmt)->write(outfilename, info_);
}

std::shared_ptr<InfoRecord>& Info::getInfoRecord(RecFormat fmt)
{
    if (fmt == infoRecord_->getRecFormat())
        return infoRecord_;
    else {
        switch (fmt) {
        case RecFormat::XQF:
            return infoRecord_ = std::make_shared<XQFInfoRecord>();
        case RecFormat::ICCS:
            return infoRecord_ = std::make_shared<PGN_ICCSInfoRecord>();
        case RecFormat::ZH:
            return infoRecord_ = std::make_shared<PGN_ZHInfoRecord>();
        case RecFormat::CC:
            return infoRecord_ = std::make_shared<PGN_CCInfoRecord>();
        case RecFormat::BIN:
            return infoRecord_ = std::make_shared<BINInfoRecord>();
        case RecFormat::JSON:
            return infoRecord_ = std::make_shared<JSONInfoRecord>();
        default:
            return infoRecord_ = std::make_shared<PGN_ZHInfoRecord>();
        }
    }
}

RecFormat XQFInfoRecord::getRecFormat() { return RecFormat::XQF; }

std::map<std::wstring, std::wstring> XQFInfoRecord::read(std::ifstream& ifs) //std::ifstream& ifs
{
    unsigned char Signature[3], version, headKeyMask, //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
        headKeyOrA, headKeyOrB, headKeyOrC, headKeyOrD,
        headKeysSum, headKeyXY, headKeyXYf, headKeyXYt, // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headQiziXY[32], // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)PlayStepNo[2],
        headWhoPlay, headPlayResult, // PlayNodes[4], PTreePos[4], Reserved1[4],
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16], TitleA[65], TitleB[65], //对局类型(开,中,残等)
        Event[65], Date[17], Site[17], Red[17], Black[17],
        Opening[65], Redtime[17], Blktime[17], Reservedh[33],
        RMKWriter[17], Author[17]; // 棋谱评论员/文件的作者
    int16_t PlayStepNo;
    int32_t ProductId, PlayNodes, PTreePos, Reserved1;
    std::function<void(unsigned char*, int)> getChars = [&](unsigned char target[], int lenght) {
        for (int i = 0; i < lenght; ++i)
            ifs >> target[i];
    };

    getChars(Signature, 2);
    ifs >> Version_xqf >> headKeyMask >> ProductId // = 8 bytes
        >> headKeyOrA >> headKeyOrB >> headKeyOrC >> headKeyOrD
        >> headKeysSum >> headKeyXY >> headKeyXYf >> headKeyXYt; // = 16 bytes
    getChars(headQiziXY, 32); // = 48 bytes
    ifs >> PlayStepNo >> headWhoPlay >> headPlayResult >> PlayNodes >> PTreePos >> Reserved1; // = 64 bytes
    for (std::pair<unsigned char*, int>& tarlen : (std::vector<std::pair<unsigned char*, int>>{
             { headCodeA_H, 16 }, { TitleA, 64 }, { TitleB, 64 }, { Event, 64 },
             { Date, 16 }, { Site, 16 }, { Red, 16 }, { Black, 16 },
             { Opening, 64 }, { Redtime, 16 }, { Blktime, 16 }, { Reservedh, 32 },
             { RMKWriter, 16 }, { Author, 16 } }))
        getChars(tarlen.first, tarlen.second);
    ifs.ignore(528); // = 1024 bytes

    if (Signature[0] != 0x58 || Signature[1] != 0x51)
        std::wcout << Tools::s2ws(Signature) << L" 文件标记不对。Signature != (0x58, 0x51)\n";
    if ((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 != 0)
        std::wcout << headKeysSum << headKeyXY << headKeyXYf << headKeyXYt << L" 检查密码校验和不对，不等于0。\n";
    if (version > 18)
        std::wcout << version << L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

    unsigned char KeyXY, KeyXYf, KeyXYt;
    int KeyRMKSize;
    auto __subbyte = [](const int a, const int b) { return (256 + a - b) % 256; };
    std::function<unsigned char(unsigned char, unsigned char)> __calkey = [](unsigned char bKey, unsigned char cKey) {
        return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey % 256; // 保持为<256
    };
    if (version <= 10) { // 兼容1.0以前的版本
        KeyXY = KeyXYf = KeyXYt = 0;
        KeyRMKSize = 0;
    } else {
        KeyXY = __calkey(headKeyXY, headKeyXY);
        KeyXYf = __calkey(headKeyXYf, KeyXY);
        KeyXYt = __calkey(headKeyXYt, KeyXYf);
        KeyRMKSize = ((headKeysSum * 256 + headKeyXY) % 32000) + 767; // % 65536
        if (version >= 12) { // 棋子位置循环移动
            std::vector<unsigned char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // headQiziXY 不是数组，不能用
            for (int i = 0; i != 32; ++i)
                headQiziXY[(i + KeyXY + 1) % 32] = Qixy[i];
        }
        for (int i = 0; i != 32; ++i) // 棋子位置解密
            headQiziXY[i] = __subbyte(headQiziXY[i], KeyXY); // 保持为8位无符号整数，<256
    }

    unsigned char KeyBytes[]{ (headKeysSum & headKeyMask) | headKeyOrA,
        (headKeyXY & headKeyMask) | headKeyOrB,
        (headKeyXYf & headKeyMask) | headKeyOrC,
        (headKeyXYt & headKeyMask) | headKeyOrD };
    std::string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    std::vector<int> F32Keys(32, 0);
    for (int i = 0; i != 32; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    std::wstring pieceChars(90, L'_');
    std::wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    for (int i = 0; i != 32; ++i) {
        int xy = headQiziXY[i];
        if (xy < 90) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    std::map<std::wstring, std::wstring> info{
        { L"Version_xqf", std::to_wstring(version) },
        { L"Result", (std::map<unsigned char, std::wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult] },
        { L"PlayType", (std::map<unsigned char, std::wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]] },
        { L"TitleA", Tools::s2ws(TitleA) },
        { L"Event", Tools::s2ws(Event) },
        { L"Date", Tools::s2ws(Date) },
        { L"Site", Tools::s2ws(Site) },
        { L"Red", Tools::s2ws(Red) },
        { L"Black", Tools::s2ws(Black) },
        { L"Opening", Tools::s2ws(Opening) },
        { L"RMKWriter", Tools::s2ws(RMKWriter) },
        { L"Author", Tools::s2ws(Author) },
        { L"FEN", getFEN(pieceChars) }
    };
    //std::wcout << info[L"FEN"] << std::endl;

    return info, KeyXYf, KeyXYt, KeyRMKSize, F32Keys,
}

void XQFInfoRecord::write(std::ofstream& ofs) const
{
}

RecFormat PGN_ICCSInfoRecord::getRecFormat() { return RecFormat::PGN_ICCS; }

void PGN_ICCSInfoRecord::read(std::ifstream& ifs, Info& instance)
{
    __readMove_ICCSZH(__readInfo_getMoveStr(infilename, instance), instance, RecFormat::ICCS);
}

RecFormat PGN_ZHInfoRecord::getRecFormat() { return RecFormat::PGN_ZH; }

void PGN_ZHInfoRecord::read(std::ifstream& ifs, Info& instance)
{
    __readMove_ICCSZH(__readInfo_getMoveStr(infilename, instance), instance, RecFormat::ZH);
}

RecFormat PGN_CCInfoRecord::getRecFormat() { return RecFormat::PGN_CC; }

void PGN_CCInfoRecord::read(std::ifstream& ifs, Info& instance)
{
    __readMove_CC(__readInfo_getMoveStr(infilename, instance), instance);
}

const std::wstring InfoRecord::__readInfo_getMoveStr(std::ifstream& ifs, Info& instance)
{
    std::wstring pgnTxt{ Tools::readTxt(infilename) };
    auto pos = pgnTxt.find(L"》");
    std::wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    std::wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    std::map<std::wstring, std::wstring> info{};
    for (std::wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != std::wsregex_iterator{}; ++p)
        info[(*p)[1]] = (*p)[2];
    instance.setInfo(info);
    return moveStr;
}

void InfoRecord::__readMove_ICCSZH(const std::wstring& moveStr, Info& instance, const RecFormat fmt)
{
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                             : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](std::shared_ptr<Move> move, const std::wstring mvstr, bool isOther) { //# 非递归
        for (std::wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != std::wsregex_iterator{}; ++p) {
            auto newMove = isOther ? move->addOther() : move->addNext();
            fmt == RecFormat::ZH ? (newMove->zh_ = (*p)[1]) : (newMove->iccs_ = (*p)[1]);
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
    instance.setRootMove(rootMove);
}

void InfoRecord::__readMove_CC(const std::wstring& moveStr, Info& instance)
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
    instance.setRootMove(std::make_shared<Move>(rootMove));
}

void PGN_ICCSInfoRecord::write(std::ofstream& ofs) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_ICCSZH(instance, RecFormat::ICCS));
}

void PGN_ZHInfoRecord::write(std::ofstream& ofs) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_ICCSZH(instance, RecFormat::ZH));
}

void PGN_CCInfoRecord::write(std::ofstream& ofs) const
{
    Tools::writeTxt(outfilename, __getPGNInfo(instance) + __getPGNTxt_CC(instance));
}

const std::wstring InfoRecord::__getPGNInfo(const Info& instance) const
{
    std::wstringstream wss{};
    auto& info = instance.getInfo();
    for (auto& kv : info)
        wss << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
    return wss.str() + L"》";
}

const std::wstring InfoRecord::__getPGNTxt_ICCSZH(const Info& instance, const RecFormat fmt) const
{
    std::wstringstream wss{};
    std::function<void(const Move&)> __remark = [&](const Move& move) {
        if (!move.remark_.empty())
            wss << (L"\n{" + move.remark_ + L"}\n");
    };

    std::function<void(const Move&, bool)> __moveStr = [&](const Move& move, bool isOther) {
        std::wstring boutNum{ std::to_wstring((move.n_ + 1) / 2) };
        bool isEven{ move.n_ % 2 == 0 };
        wss << (isOther ? L"(" + boutNum + L". " + (isEven ? L"... " : L"") : (isEven ? L" " : boutNum + L". "))
            << (fmt == RecFormat::ZH ? move.zh_ : move.iccs_) << L' ';
        __remark(move);
        if (move.other_) {
            __moveStr(*move.other_, true);
            wss << L") ";
        }
        if (move.next_)
            __moveStr(*move.next_, false);
    };

    auto& rootMove = instance.getRootMove();
    __remark(*rootMove);
    if (rootMove->next_)
        __moveStr(*rootMove->next_, false);
    return wss.str();
}

const std::wstring InfoRecord::__getPGNTxt_CC(const Info& instance) const
{
    std::wstringstream remStrs{};
    std::wstring lstr((instance.getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((instance.getMaxRow() + 1) * 2, lstr);
    std::function<void(const Move&)> __setChar = [&](const Move& move) {
        int firstcol{ move.CC_Col_ * 5 }, row{ move.n_ * 2 };
        lineStr.at(row).replace(firstcol, 4, move.zh_);
        if (!move.remark_.empty())
            remStrs << L"(" << move.n_ << L"," << move.CC_Col_ << L"): {" << move.remark_ << L"}\n";
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

    auto& rootMove = instance.getRootMove();
    __setChar(*rootMove);
    std::wstringstream wss{};
    lineStr.front().replace(0, 2, L"开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << instance.moveInfo();
    return wss.str();
}

RecFormat BINInfoRecord::getRecFormat() { return RecFormat::BIN; }

void BINInfoRecord::read(std::ifstream& ifs, Info& instance)
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
    instance.setInfo(info);
    instance.setRootMove(std::make_shared<Move>(rootMove));
}

void BINInfoRecord::write(std::ofstream& ofs) const
{
    std::ofstream ofs(outfilename, std::ios_base::binary);
    auto& info = instance.getInfo();
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

    __write(*instance.getRootMove());
}

RecFormat JSONInfoRecord::getRecFormat() { return RecFormat::JSON; }

void JSONInfoRecord::read(std::ifstream& ifs, Info& instance)
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
    instance.setRootMove(std::make_shared<Move>(rootMove));
}

void JSONInfoRecord::write(std::ofstream& ofs) const
{
    std::ofstream ofs(outfilename);
    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value infoItem;
    auto& info = instance.getInfo();
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

    root["moves"] = __writeItem(*instance.getRootMove());
    writer->write(root, &ofs);
}

const std::wstring getFEN(const std::wstring& pieceChars)
{
    //'下划线字符串对应数字字符'
    std::vector<std::pair<std::wstring, std::wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    std::wstring fen{};
    for (int i = 81; i >= 0; i -= 9)
        fen += pieceChars.substr(i, 9) + L"/";
    fen.erase(fen.size() - 1, 1);
    std::wstring::size_type pos;
    for (auto& linenum : line_nums)
        while ((pos = fen.find(linenum.first)) != std::wstring::npos)
            fen.replace(pos, linenum.first.size(), linenum.second);
    /*
    assert(pieceChars.size() == 90);
    std::wstring fen{};
    std::wregex linerg{ LR"(.{9})" }, sp{ LR"()" + board_->getNullChar() + LR"({1,9})" };
    std::wsregex_token_iterator end_it{};
    //std::wstringstream wss{};
    for (std::wsregex_token_iterator lineIter{ pieceChars.begin(), pieceChars.end(), linerg, 0 }; lineIter != end_it; ++lineIter) {
        std::wstringstream wss{};
        for (std::wsregex_token_iterator charIter{ (*lineIter).begin(), (*lineIter).end(), sp, -1 }; charIter != end_it; ++charIter) {
            wss << *charIter;
            wss << num; // sp.size?
        }
        wss << L'/';
        fen.insert(0, wss.str());
    }*/
    //assert(getPieceChars(fen) == pieceChars);
    return fen;
}

const std::wstring getPieceChars(const std::wstring& fen)
{
    std::wstring pieceChars{};
    std::wregex sp{ LR"(/)" };
    for (std::wsregex_token_iterator fenLineIter{ fen.begin(), fen.end(), sp, -1 };
         fenLineIter != std::wsregex_token_iterator{}; ++fenLineIter) {
        std::wstringstream wss{};
        for (auto wch : std::wstring{ *fenLineIter })
            wss << (isdigit(wch) ? std::wstring(wch - 48, BoardSpace::Board::getNullChar()) : std::wstring{ wch }); // ASCII: 0:48
        pieceChars.insert(0, wss.str());
    }

    assert(fen == getFEN(pieceChars));
    return pieceChars;
}
}