#include "Info.h"
#include "../json/json.h"
#include "instance.h"
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

namespace InfoSpace {

// Info
Info::Info()
    : infoMap_{ { L"Author", L"" },
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
{
}

void Info::read(std::ifstream& ifs, RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(ifs);
        break;
    case RecFormat::BIN:
        readBIN(ifs);
        break;
    case RecFormat::JSON:
        readJSON(ifs);
        break;
    default:
        readPGN(ifs);
        break;
    }
    infoMap_[L"Format"] = Tools::s2ws(InstanceSpace::getExtName(fmt));
}

void Info::write(std::ofstream& ofs, RecFormat fmt) const
{
    switch (fmt) {
    case RecFormat::XQF:
        writeXQF(ofs);
        break;
    case RecFormat::BIN:
        writeBIN(ofs);
        break;
    case RecFormat::JSON:
        writeJSON(ofs);
        break;
    default:
        writePGN(ofs);
        break;
    }
}

void Info::setFEN(const std::wstring& pieceChars)
{
    //infoMap_[L"FEN"] = getFEN(pieceChars) + L" " + (firstColor_ == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
    //std::wstring rfen{ infoMap_[L"FEN"] };
    //assert(getPieceChars(rfen.substr(0, rfen.find(L' '))) == pieceChars);
}

const std::wstring Info::getPieceChars() const
{
    std::wstring rfen{ infoMap_[L"FEN"] };
    std::wstring fen{ rfen.substr(0, rfen.find(L' ')) };
    return getPieceChars(fen);
}

void Info::readXQF(std::ifstream& ifs)
{
    const int pieceNum{ 32 };
    char Signature[3], Version_XQF, headKeyMask, //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
        headKeyOrA, headKeyOrB, headKeyOrC, headKeyOrD,
        headKeysSum, headKeyXY, headKeyXYf, headKeyXYt; // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
    unsigned char headQiziXY[pieceNum]; // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)PlayStepNo[2],
    char headWhoPlay, headPlayResult, // PlayNodes[4], PTreePos[4], Reserved1[4],
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16], TitleA[65], TitleB[65], //对局类型(开,中,残等)
        Event[65], Date[17], Site[17], Red[17], Black[17],
        Opening[65], Redtime[17], Blktime[17], Reservedh[33],
        RMKWriter[17], Author[17]; // 棋谱评论员/文件的作者
    int16_t PlayStepNo;
    int32_t ProductId, PlayNodes, PTreePos, Reserved1;
    auto getChars = [&](char target[], int lenght) {
        for (int i = 0; i < lenght; ++i)
            ifs >> target[i];
    };

    getChars(Signature, 2);
    ifs >> Version_XQF >> headKeyMask >> ProductId // = 8 bytes
        >> headKeyOrA >> headKeyOrB >> headKeyOrC >> headKeyOrD
        >> headKeysSum >> headKeyXY >> headKeyXYf >> headKeyXYt; // = 16 bytes
    getChars(headQiziXY, pieceNum); // = 48 bytes
    ifs >> PlayStepNo >> headWhoPlay >> headPlayResult >> PlayNodes >> PTreePos >> Reserved1; // = 64 bytes
    for (std::pair<char*, int>& tarlen : (std::vector<std::pair<char*, int>>{
             { headCodeA_H, 16 }, { TitleA, 64 }, { TitleB, 64 }, { Event, 64 },
             { Date, 16 }, { Site, 16 }, { Red, 16 }, { Black, 16 },
             { Opening, 64 }, { Redtime, 16 }, { Blktime, 16 }, { Reservedh, 32 },
             { RMKWriter, 16 }, { Author, 16 } }))
        getChars(tarlen.first, tarlen.second);
    //ifs.ignore(528); // = 1024 bytes

    assert(Signature[0] == 0x58 || Signature[1] == 0x51);
    assert((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 == 0); // L" 检查密码校验和不对，不等于0。\n";
    assert(Version_XQF <= 18); // L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

    // 计算密钥值，存入类静态变量
    version = Version_XQF;
    if (version > 10) { // 兼容1.0以前的版本 if(version <= 10) KeyXYf = KeyXYt = KeyRMKSize = 0;
        auto __calkey = [](char bKey, char cKey) {
            return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey; // % 256; // 保持为<256
        };
        char KeyXY = __calkey(headKeyXY, headKeyXY);
        KeyXYf = __calkey(headKeyXYf, KeyXY);
        KeyXYt = __calkey(headKeyXYt, KeyXYf);
        KeyRMKSize = ((headKeysSum * 256 + headKeyXY) % 32000) + 767; // % 65536
        if (version >= 12) { // 棋子位置循环移动
            std::vector<char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // 数组不能拷贝
            for (int i = 0; i != pieceNum; ++i)
                headQiziXY[(i + KeyXY + 1) % pieceNum] = Qixy[i];
        }
        for (int i = 0; i != pieceNum; ++i) // 棋子位置解密
            headQiziXY[i] -= static_cast<unsigned char>(KeyXY); // 保持为8位无符号整数，<256
    }
    unsigned char KeyBytes[]{
        static_cast<unsigned char>((headKeysSum & headKeyMask) | headKeyOrA),
        static_cast<unsigned char>((headKeyXY & headKeyMask) | headKeyOrB),
        static_cast<unsigned char>((headKeyXYf & headKeyMask) | headKeyOrC),
        static_cast<unsigned char>((headKeyXYt & headKeyMask) | headKeyOrD)
    };
    const std::string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    for (int i = 0; i != pieceNum; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    // 取得棋子字符串
    std::wstring pieceChars(90, BoardSpace::Board::nullChar);
    std::wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    for (int i = 0; i != pieceNum; ++i) {
        int xy = headQiziXY[i];
        if (xy < 90) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }

    return (std::map<std::wstring, std::wstring>{
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
        { L"FEN", getFEN(pieceChars) } });
}

void Info::writeXQF(std::ofstream& ofs) const {} // 不做实现

void Info::readPGN(std::ifstream& ifs)
{
    std::stringstream ss{};
    std::string line{};
    ifs >> std::noskipws;
    while (std::getline(ifs, line) && !line.empty()) // 以空行为分割
        ss << line << '\n';
    std::wstring infoStr{ Tools::s2ws(ss.str()) };
    std::wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (std::wsregex_iterator p(infoStr.begin(), infoStr.end(), pat); p != std::wsregex_iterator{}; ++p)
        infoMap_[(*p)[1]] = (*p)[2];
}

void Info::writePGN(std::ofstream& ofs) const
{
    std::wstringstream wss{};
    std::for_each(infoMap_.begin(), infoMap_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        wss << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
    });
    wss << L'\n'; // 以空行为分割
    ofs << Tools::ws2s(wss.str());
}

void Info::readBIN(std::ifstream& ifs)
{
    std::size_t size{};
    std::string key{}, value{};
    ifs >> std::noskipws >> size;
    for (std::size_t i = 0; i < size; ++i) { // 以size为分割
        std::getline(ifs, key);
        std::getline(ifs, value);
        infoMap_[Tools::s2ws(key)] = Tools::s2ws(value);
    }
}

void Info::writeBIN(std::ofstream& ofs) const
{
    std::wstringstream wss{};
    std::for_each(infoMap_.begin(), infoMap_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        wss << kv.first << L'\n' << kv.second << L'\n';
    });
    ofs << infoMap_.size() << Tools::ws2s(wss.str()) << "\n";
}

void Info::readJSON(std::ifstream& ifs)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    Json::Value infoItem{ root["infoMap_"] };
    for (auto& key : infoItem.getMemberNames())
        infoMap_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
}

void Info::writeJSON(std::ofstream& ofs) const
{
    Json::Value root{}, infoItem{};
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::for_each(infoMap_.begin(), infoMap_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
    });
    root["infoMap_"] = infoItem;
    writer->write(root, &ofs); // 能否与move同步?
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
            wss << (isdigit(wch) ? std::wstring(wch - 48, BoardSpace::Board::nullChar) : std::wstring{ wch }); // ASCII: 0:48
        pieceChars.insert(0, wss.str());
    }

    assert(fen == getFEN(pieceChars));
    return pieceChars;
}
}