#include "instance.h"
#include "../json/json.h"
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

using namespace PieceSpace;
using namespace SeatSpace;
using namespace BoardSpace;
namespace InstanceSpace {

// Instance
Instance::Instance()
    : info_{ { L"FEN", PieceManager::getFENStr() } }
{
}

Instance::Instance(const std::string& infilename)
{
    read(infilename);
}

void Instance::go()
{
    if (currentMove_->next()) {
        currentMove_ = currentMove_->next();
        currentMove_->done();
    }
}

void Instance::back()
{
    if (currentMove_->prev()) {
        currentMove_->undo();
        currentMove_ = currentMove_->prev();
    }
}

void Instance::backTo(const std::shared_ptr<Move>& move)
{
    while (currentMove_ != rootMove_ && currentMove_ != move)
        back();
}

void Instance::goOther()
{
    if (currentMove_ != rootMove_ && currentMove_->other()) {
        currentMove_->undo();
        currentMove_ = currentMove_->other();
        currentMove_->done();
    }
}

void Instance::goInc(int inc)
{
    //std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::go : &Instance::back);
    for (int i = abs(inc); i != 0; --i)
        fbward(this);
}

void Instance::changeSide(ChangeType ct)
{
    std::vector<std::shared_ptr<Move>> prevMoves{};
    if (currentMove_ != rootMove_)
        prevMoves = currentMove_->getPrevMoves();
    backTo(rootMove_);
    board_->changeSide(ct);
    if (ct != ChangeType::EXCHANGE) {
        auto changeRowcol = (ct == ChangeType::ROTATE
                ? &SeatManager::getRotate
                : &SeatManager::getSymmetry);
        //auto changeRowcol = std::mem_fn(ct == ChangeType::ROTATE ? &SeatManager::getRotate : &SeatManager::getSymmetry);
        std::function<void(const std::shared_ptr<Move>&)>
            __resetMove = [&](const std::shared_ptr<Move>& move) {
                __setMoveFromRowcol(move, changeRowcol(move->fseat()->rowcol()),
                    changeRowcol(move->tseat()->rowcol()), move->remark());
                if (move->next())
                    __resetMove(move->next());
                if (move->other())
                    __resetMove(move->other());
            };
        if (rootMove_->next())
            __resetMove(rootMove_->next());
    }
    __setFEN(board_->getPieceChars(),
        (rootMove_->next()->fseat()
                ? rootMove_->next()->fseat()->piece()->color()
                : PieceColor::RED));
    if (ct != ChangeType::ROTATE)
        __setMoveZhStrAndNums();
    for (auto& move : prevMoves)
        move->done();
}

void Instance::read(const std::string& infilename)
{
    __reset();
    RecFormat fmt = getRecFormat(Tools::getExt(infilename));
    std::ifstream is{};
    std::wifstream wis{};
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        is = std::ifstream(infilename, std::ios_base::binary);
    else
        wis = std::wifstream(infilename);
    switch (fmt) {
    case RecFormat::XQF:
        __readXQF(is);
        break;
    case RecFormat::BIN:
        __readBIN(is);
        break;
    case RecFormat::JSON:
        __readJSON(is);
        break;
    case RecFormat::PGN_ICCS:
        __readInfo_PGN(wis);
        __readMove_PGN_ICCSZH(wis, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __readInfo_PGN(wis);
        __readMove_PGN_ICCSZH(wis, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __readInfo_PGN(wis);
        __readMove_PGN_CC(wis);
        break;
    default:
        break;
    }
    currentMove_ = rootMove_;
    __setMoveZhStrAndNums();
}

void Instance::write(const std::string& outfilename)
{
    RecFormat fmt = getRecFormat(Tools::getExt(outfilename));
    std::ofstream os{};
    std::wofstream wos{};
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        os = std::ofstream(outfilename, std::ios_base::binary);
    else
        wos = std::wofstream(outfilename);
    switch (fmt) {
    case RecFormat::XQF:
        break;
    case RecFormat::BIN:
        __writeBIN(os);
        break;
    case RecFormat::JSON:
        __writeJSON(os);
        break;
    case RecFormat::PGN_ICCS:
        __writeInfo_PGN(wos);
        __writeMove_PGN_ICCSZH(wos, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __writeInfo_PGN(wos);
        __writeMove_PGN_ICCSZH(wos, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __writeInfo_PGN(wos);
        __writeMove_PGN_CC(wos);
        break;
    default:
        break;
    }
}

const std::wstring& Instance::remark() const { return rootMove_->remark(); }

const std::wstring Instance::toString()
{
    std::wostringstream wos{};
    __writeInfo_PGN(wos);
    __writeMove_PGN_CC(wos);

    backTo(rootMove_);
    std::vector<std::shared_ptr<Move>> preMoves{};
    std::function<void(bool)>
        __printMoveBoard = [&](bool isOther) {
            isOther ? goOther() : go();
            wos << board_->toString() << currentMove_->toString() << L"\n\n";
            if (currentMove_->other()) {
                preMoves.push_back(currentMove_);
                __printMoveBoard(true);
                // 变着之前着在返回时，应予执行
                if (!preMoves.empty()) {
                    preMoves.back()->done();
                    preMoves.pop_back();
                }
            }
            if (currentMove_->next()) {
                __printMoveBoard(false);
            }
            back();
        };
    if (currentMove_->next())
        __printMoveBoard(false);

    return wos.str();
}

const std::wstring Instance::test()
{
    read("4.xqf");
    //read("01.xqf");

    write("01.bin");
    read("01.bin");

    write("01.json");
    read("01.json");

    write("01.pgn_iccs");
    read("01.pgn_iccs");

    write("01.pgn_zh");
    read("01.pgn_zh");

    write("01.pgn_cc");
    read("01.pgn_cc");

    auto str0 = toString();
    changeSide(ChangeType::EXCHANGE);
    auto str1 = toString();
    changeSide(ChangeType::ROTATE);
    auto str2 = toString();
    changeSide(ChangeType::SYMMETRY);
    auto str3 = toString();
    return str0 + str1 + str2 + str3;
}

void Instance::__reset()
{
    info_ = std::map<std::wstring, std::wstring>{};
    board_ = std::make_shared<Board>();
    currentMove_ = rootMove_ = std::make_shared<Move>();
    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
}

void Instance::__readXQF(std::istream& is)
{
    const int pieceNum{ 32 };
    char Signature[3]{}, Version{}, headKeyMask{}, ProductId[4]{}, //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
        headKeyOrA{}, headKeyOrB{}, headKeyOrC{}, headKeyOrD{},
        headKeysSum{}, headKeyXY{}, headKeyXYf{}, headKeyXYt{}, // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headQiziXY[pieceNum]{}, // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)PlayStepNo[2],
        PlayStepNo[2]{},
        headWhoPlay{}, headPlayResult{}, PlayNodes[4]{}, PTreePos[4]{}, Reserved1[4]{},
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16]{}, TitleA[65]{}, TitleB[65]{}, //对局类型(开,中,残等)
        Event[65]{}, Date[17]{}, Site[17]{}, Red[17]{}, Black[17]{},
        Opening[65]{}, Redtime[17]{}, Blktime[17]{}, Reservedh[33]{},
        RMKWriter[17]{}, Author[17]{}; //, Other[528]{}; // 棋谱评论员/文件的作者

    is.read(Signature, 2).get(Version).get(headKeyMask).read(ProductId, 4) // = 8 bytes
        .get(headKeyOrA)
        .get(headKeyOrB)
        .get(headKeyOrC)
        .get(headKeyOrD)
        .get(headKeysSum)
        .get(headKeyXY)
        .get(headKeyXYf)
        .get(headKeyXYt) // = 16 bytes
        .read(headQiziXY, pieceNum) // = 48 bytes
        .read(PlayStepNo, 2)
        .get(headWhoPlay)
        .get(headPlayResult)
        .read(PlayNodes, 4)
        .read(PTreePos, 4)
        .read(Reserved1, 4) // = 64 bytes
        .read(headCodeA_H, 16)
        .read(TitleA, 64)
        .read(TitleB, 64)
        .read(Event, 64)
        .read(Date, 16)
        .read(Site, 16)
        .read(Red, 16)
        .read(Black, 16)
        .read(Opening, 64)
        .read(Redtime, 16)
        .read(Blktime, 16)
        .read(Reservedh, 32)
        .read(RMKWriter, 16)
        .read(Author, 16);

    assert(Signature[0] == 0x58 || Signature[1] == 0x51);
    assert((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 == 0); // L" 检查密码校验和不对，不等于0。\n";
    assert(Version <= 18); // L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

    unsigned char KeyXY{}, KeyXYf{}, KeyXYt{}, F32Keys[pieceNum], *head_QiziXY{ (unsigned char*)headQiziXY };
    int KeyRMKSize{};
    if (Version <= 10) { // version <= 10 兼容1.0以前的版本
        KeyRMKSize = KeyXYf = KeyXYt = 0;
    } else {
        std::function<unsigned char(unsigned char, unsigned char)> __calkey = [](unsigned char bKey, unsigned char cKey) {
            return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey; // % 256; // 保持为<256
        };
        KeyXY = __calkey(headKeyXY, headKeyXY);
        KeyXYf = __calkey(headKeyXYf, KeyXY);
        KeyXYt = __calkey(headKeyXYt, KeyXYf);
        KeyRMKSize = (static_cast<unsigned char>(headKeysSum) * 256 + static_cast<unsigned char>(headKeyXY)) % 32000 + 767; // % 65536
        if (Version >= 12) { // 棋子位置循环移动
            std::vector<unsigned char> Qixy(std::begin(headQiziXY), std::end(headQiziXY)); // 数组不能拷贝
            for (int i = 0; i != pieceNum; ++i)
                head_QiziXY[(i + KeyXY + 1) % pieceNum] = Qixy[i];
        }
        for (int i = 0; i != pieceNum; ++i)
            head_QiziXY[i] -= KeyXY; // 保持为8位无符号整数，<256
    }
    int KeyBytes[4]{
        (headKeysSum & headKeyMask) | headKeyOrA,
        (headKeyXY & headKeyMask) | headKeyOrB,
        (headKeyXYf & headKeyMask) | headKeyOrC,
        (headKeyXYt & headKeyMask) | headKeyOrD
    };
    const std::string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    for (int i = 0; i != pieceNum; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    // 取得棋子字符串
    std::wstring pieceChars(90, PieceManager::nullChar());
    std::wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    for (int i = 0; i != pieceNum; ++i) {
        int xy = head_QiziXY[i];
        if (xy <= 89) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    board_->reset(pieceChars);
    info_ = std::map<std::wstring, std::wstring>{
        { L"Version", std::to_wstring(Version) },
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
        { L"FEN", pieCharsToFEN(pieceChars) } // 可能存在不是红棋先走的情况？
    };

    std::function<unsigned char(unsigned char, unsigned char)>
        __sub = [](unsigned char a, unsigned char b) {
            return a - b;
        }; // 保持为<256
    auto __readBytes = [&](char* bytes, int size) {
        int pos = is.tellg();
        is.read(bytes, size);
        if (Version > 10) // '字节解密'
            for (int i = 0; i != size; ++i)
                bytes[i] = __sub(bytes[i], F32Keys[(pos + i) % 32]);
    };
    char data[4]{}, &frc{ data[0] }, &trc{ data[1] }, &tag{ data[2] };
    auto __getRemarksize = [&]() {
        char clen[4]{};
        __readBytes(clen, 4);
        return *(int*)clen - KeyRMKSize;
    };
    std::function<std::wstring()>
        __readDataAndGetRemark = [&]() {
            __readBytes(data, 4);
            int RemarkSize{};
            if (Version <= 10) {
                tag = ((tag & 0xF0) ? 0x80 : 0) | ((tag & 0x0F) ? 0x40 : 0);
                RemarkSize = __getRemarksize();
            } else {
                tag &= 0xE0;
                if (tag & 0x20)
                    RemarkSize = __getRemarksize();
            }
            if (RemarkSize > 0) { // # 如果有注解
                char rem[RemarkSize + 1]{};
                __readBytes(rem, RemarkSize);
                return Tools::s2ws(rem);
            } else
                return std::wstring{};
        };
    std::function<void(const std::shared_ptr<Move>&)>
        __readMove = [&](const std::shared_ptr<Move>& move) {
            auto remark = __readDataAndGetRemark();
            //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
            int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
            assert(fcolrow <= 89 && tcolrow <= 89);
            __setMoveFromRowcol(move, (fcolrow % 10) * 10 + fcolrow / 10,
                (tcolrow % 10) * 10 + tcolrow / 10, remark);

            char ntag{ tag };
            if (ntag & 0x80) //# 有左子树
                __readMove(move->addNext());
            if (ntag & 0x40) // # 有右子树
                __readMove(move->addOther());
        };

    is.seekg(1024);
    rootMove_->setRemark(__readDataAndGetRemark());
    char rtag{ tag };
    if (rtag & 0x80) //# 有左子树
        __readMove(rootMove_->addNext());
}

void Instance::__readBIN(std::istream& is)
{
    char len[sizeof(int)]{};
    std::function<std::wstring()> __readWstring = [&]() {
        is.read(len, sizeof(int));
        int length{ *(int*)len };
        char rem[length + 1]{};
        is.read(rem, length);
        return Tools::s2ws(rem);
    };
    char frowcol{}, trowcol{};
    std::function<void(const std::shared_ptr<Move>&)>
        __readMove = [&](const std::shared_ptr<Move>& move) {
            char tag{};
            is.get(frowcol).get(trowcol).get(tag);
            __setMoveFromRowcol(move, frowcol, trowcol, (tag & 0x20) ? __readWstring() : L"");

            if (tag & 0x80)
                __readMove(move->addNext());
            if (tag & 0x40)
                __readMove(move->addOther());
            return move;
        };

    char atag{};
    is.get(atag);
    if (atag & 0x80) {
        char len{};
        is.get(len);
        std::wstring key{}, value{};
        for (int i = 0; i < len; ++i) {
            key = __readWstring();
            value = __readWstring();
            info_[key] = value;
        }
    }
    board_->reset(__pieceChars());

    if (atag & 0x40)
        rootMove_->setRemark(__readWstring());
    if (atag & 0x20)
        __readMove(rootMove_->addNext());
}

void Instance::__writeBIN(std::ostream& os) const
{
    auto __writeWstring = [&](const std::wstring& wstr) {
        std::string str{ Tools::ws2s(wstr) };
        int len = str.size();
        os.write((char*)&len, sizeof(int)).write(str.c_str(), len);
    };
    std::function<void(const std::shared_ptr<Move>&)>
        __writeMove = [&](const std::shared_ptr<Move>& move) {
            char tag = ((move->next() ? 0x80 : 0x00)
                | (move->other() ? 0x40 : 0x00)
                | (!move->remark().empty() ? 0x20 : 0x00));
            os.put(move->frowcol()).put(move->trowcol()).put(tag);
            if (tag & 0x20)
                __writeWstring(move->remark());
            if (tag & 0x80)
                __writeMove(move->next());
            if (tag & 0x40)
                __writeMove(move->other());
        };

    char tag = ((!info_.empty() ? 0x80 : 0x00)
        | (!rootMove_->remark().empty() ? 0x40 : 0x00)
        | (rootMove_->next() ? 0x20 : 0x00));
    os.put(tag);
    if (tag & 0x80) {
        os.put(info_.size());
        std::for_each(info_.begin(), info_.end(),
            [&](const std::pair<std::wstring, std::wstring>& kv) {
                __writeWstring(kv.first);
                __writeWstring(kv.second);
            });
    }
    if (tag & 0x40)
        __writeWstring(rootMove_->remark());
    if (tag & 0x20)
        __writeMove(rootMove_->next());
}

void Instance::__readJSON(std::istream& is)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, is, &root, &errs))
        return;

    Json::Value infoItem{ root["info"] };
    for (auto& key : infoItem.getMemberNames())
        info_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    board_->reset(__pieceChars());

    std::function<void(const std::shared_ptr<Move>&, Json::Value&)>
        __readMove = [&](const std::shared_ptr<Move>& move, Json::Value& item) {
            int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
            __setMoveFromRowcol(move, frowcol, trowcol,
                (item.isMember("r")) ? Tools::s2ws(item["r"].asString()) : L"");

            if (item.isMember("n"))
                __readMove(move->addNext(), item["n"]);
            if (item.isMember("o"))
                __readMove(move->addOther(), item["o"]);
        };

    rootMove_->setRemark(Tools::s2ws(root["remark"].asString()));
    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __readMove(rootMove_->addNext(), rootItem);
}

void Instance::__writeJSON(std::ostream& os) const
{
    Json::Value root{}, infoItem{};
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::for_each(info_.begin(), info_.end(),
        [&](const std::pair<std::wstring, std::wstring>& kv) {
            infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
        });
    root["info"] = infoItem;
    std::function<Json::Value(const std::shared_ptr<Move>&)>
        __writeItem = [&](const std::shared_ptr<Move>& move) {
            Json::Value item{};
            item["f"] = move->frowcol();
            item["t"] = move->trowcol();
            if (!move->remark().empty())
                item["r"] = Tools::ws2s(move->remark());
            if (move->next())
                item["n"] = __writeItem(move->next());
            if (move->other())
                item["o"] = __writeItem(move->other());
            return item;
        };
    root["remark"] = Tools::ws2s(rootMove_->remark());
    if (rootMove_->next())
        root["moves"] = __writeItem(rootMove_->next());
    writer->write(root, &os);
}

void Instance::__readInfo_PGN(std::wistream& wis)
{
    std::wstring line{};
    std::wregex info{ LR"(\[(\w+)\s+\"([\s\S]*?)\"\])" };
    while (std::getline(wis, line) && !line.empty()) { // 以空行为终止特征
        std::wsmatch matches;
        if (std::regex_match(line, matches, info))
            info_[matches[1]] = matches[2];
    }
    board_->reset(__pieceChars());
}

void Instance::__readMove_PGN_ICCSZH(std::wistream& wis, RecFormat fmt)
{
    const std::wstring moveStr{ getWString(wis) };
    bool isPGN_ZH{ fmt == RecFormat::PGN_ZH };
    std::wstring otherBeginStr{ LR"((\()?)" };
    std::wstring boutStr{ LR"((\d+\.)?[\s...]*\b)" };
    std::wstring ICCSZhStr{ LR"(([)"
        + (isPGN_ZH ? PieceManager::getZhChars() : PieceManager::getICCSChars())
        + LR"(]{4})\b)" };
    std::wstring remarkStr{ LR"((?:\s*\{([\s\S]*?)\})?)" };
    std::wstring otherEndStr{ LR"(\s*(\)+)?)" }; // 可能存在多个右括号
    std::wregex moveReg{ otherBeginStr + boutStr + ICCSZhStr + remarkStr + otherEndStr },
        remReg{ remarkStr + LR"(1\.)" };
    std::wsmatch wsm{};
    if (std::regex_search(moveStr, wsm, remReg))
        rootMove_->setRemark(wsm.str(1));
    std::shared_ptr<Move> preMove{ rootMove_ }, move{ rootMove_ };
    std::vector<std::shared_ptr<Move>> preOtherMoves{};
    for (std::wsregex_iterator wtiMove{ moveStr.begin(), moveStr.end(), moveReg }, wtiEnd{};
         wtiMove != wtiEnd; ++wtiMove) {
        if ((*wtiMove)[1].matched) {
            move = preMove->addOther();
            preOtherMoves.push_back(preMove);
            if (isPGN_ZH)
                preMove->undo();
        } else
            move = preMove->addNext();
        __setMoveFromStr(move, (*wtiMove)[3], fmt, (*wtiMove)[4]);
        if (isPGN_ZH)
            ; // std::wcout << (*wtiMove).str() << L'\n' << move->toString() << std::endl;
        if (isPGN_ZH)
            move->done(); // 推进board的状态变化
        if (isPGN_ZH)
            ; // std::wcout << board_->toString() << std::endl;

        if ((*wtiMove)[5].matched)
            for (int num = (*wtiMove).length(5); num > 0; --num) {
                preMove = preOtherMoves.back();
                preOtherMoves.pop_back();
                if (isPGN_ZH) {
                    do {
                        move->undo();
                    } while ((move = move->prev()) != preMove);
                    preMove->done();
                }
            }
        else
            preMove = move;
    }
    if (isPGN_ZH)
        while (move != rootMove_) {
            move->undo();
            move = move->prev();
        }
}

void Instance::__writeInfo_PGN(std::wostream& wos) const
{
    std::for_each(info_.begin(), info_.end(),
        [&](const std::pair<std::wstring, std::wstring>& kv) {
            wos << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
        });
    wos << L'\n';
}

void Instance::__writeMove_PGN_ICCSZH(std::wostream& wos, RecFormat fmt) const
{
    bool isPGN_ZH{ fmt == RecFormat::PGN_ZH };
    auto __getRemarkStr = [&](const std::shared_ptr<Move>& move) {
        return (move->remark().empty()) ? L"" : (L" \n{" + move->remark() + L"}\n ");
    };
    std::function<void(const std::shared_ptr<Move>&, bool)>
        __writeMove = [&](const std::shared_ptr<Move>& move, bool isOther) {
            std::wstring boutStr{ std::to_wstring((move->nextNo() + 1) / 2) + L". " };
            bool isEven{ move->nextNo() % 2 == 0 };
            wos << (isOther ? L"(" + boutStr + (isEven ? L"... " : L"")
                            : (isEven ? std::wstring{ L" " } : boutStr))
                << (isPGN_ZH ? move->zh() : move->iccs()) << L' '
                << __getRemarkStr(move);

            if (move->other()) {
                __writeMove(move->other(), true);
                wos << L")";
            }
            if (move->next())
                __writeMove(move->next(), false);
        };

    wos << __getRemarkStr(rootMove_);
    if (rootMove_->next())
        __writeMove(rootMove_->next(), false);
}

void Instance::__readMove_PGN_CC(std::wistream& wis)
{
    const std::wstring move_remStr{ getWString(wis) };
    auto pos0 = move_remStr.find(L"\n("), pos1 = move_remStr.find(L"\n【");
    std::wstring moveStr{ move_remStr.substr(0, std::min(pos0, pos1)) },
        remStr{ move_remStr.substr(std::min(pos0, move_remStr.size()), pos1) };
    std::wregex line_rg{ LR"(\n)" }, moveStrrg{ LR"(.{5})" },
        moverg{ LR"(([^…　]{4}[…　]))" },
        remrg{ LR"(\s*(\(\d+,\d+\)): \{([\s\S]*?)\})" };
    std::map<std::wstring, std::wstring> rems{};
    for (std::wsregex_iterator rp{ remStr.begin(), remStr.end(), remrg };
         rp != std::wsregex_iterator{}; ++rp)
        rems[(*rp)[1]] = (*rp)[2];

    std::vector<std::vector<std::wstring>> moveLines{};
    for (std::wsregex_token_iterator lineStrit{ moveStr.begin(), moveStr.end(), line_rg, -1 },
         end{};
         lineStrit != end; ++++lineStrit) {
        std::vector<std::wstring> line{};
        for (std::wsregex_token_iterator moveit{
                 (*lineStrit).first, (*lineStrit).second, moveStrrg, 0 };
             moveit != end; ++moveit)
            line.push_back(*moveit);
        moveLines.push_back(line);
    }
    std::function<void(const std::shared_ptr<Move>&, int, int)>
        __readMove = [&](const std::shared_ptr<Move>& move, int row, int col) {
            std::wstring zhStr{ moveLines[row][col] };
            if (regex_match(zhStr, moverg)) {
                __setMoveFromStr(move, zhStr.substr(0, 4), RecFormat::PGN_CC,
                    rems[L'(' + std::to_wstring(row) + L',' + std::to_wstring(col) + L')']);

                if (zhStr.back() == L'…')
                    __readMove(move->addOther(), row, col + 1);
                if (int(moveLines.size()) - 1 > row
                    && moveLines[row + 1][col][0] != L'　') {
                    move->done();
                    __readMove(move->addNext(), row + 1, col);
                    move->undo();
                }
            } else if (moveLines[row][col][0] == L'…') {
                while (moveLines[row][++col][0] == L'…')
                    ;
                __readMove(move, row, col);
            }
        };

    rootMove_->setRemark(rems[L"(0,0)"]);
    if (!moveLines.empty())
        __readMove(rootMove_->addNext(), 1, 0);
}

void Instance::__writeMove_PGN_CC(std::wostream& wos) const
{
    std::wstringstream remWss{};
    std::wstring blankStr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, blankStr);
    std::function<void(const std::shared_ptr<Move>&)>
        __setMovePGN_CC = [&](const std::shared_ptr<Move>& move) {
            int firstcol{ move->CC_ColNo() * 5 }, row{ move->nextNo() * 2 };
            lineStr.at(row).replace(firstcol, 4, move->zh());
            if (!move->remark().empty())
                remWss << L"(" << move->nextNo() << L"," << move->CC_ColNo() << L"): {"
                       << move->remark() << L"}\n";

            if (move->next()) {
                lineStr.at(row + 1).at(firstcol + 2) = L'↓';
                __setMovePGN_CC(move->next());
            }
            if (move->other()) {
                int fcol{ firstcol + 4 }, num{ move->other()->CC_ColNo() * 5 - fcol };
                lineStr.at(row).replace(fcol, num, std::wstring(num, L'…'));
                __setMovePGN_CC(move->other());
            }
        };

    if (!remark().empty())
        remWss << L"(0,0): {" << remark() << L"}\n";
    lineStr.front().replace(0, 3, L"　开始");
    lineStr.at(1).at(2) = L'↓';
    if (rootMove_->next())
        __setMovePGN_CC(rootMove_->next());
    for (auto& line : lineStr)
        wos << line << L'\n';
    wos << remWss.str() << __moveInfo();
}

void Instance::__setMoveFromRowcol(const std::shared_ptr<Move>& move,
    int frowcol, int trowcol, const std::wstring& remark) const
{
    move->setFTSeat(board_->getSeat(frowcol), board_->getSeat(trowcol));
    move->setRemark(remark);
}

void Instance::__setMoveFromStr(const std::shared_ptr<Move>& move,
    const std::wstring& str, RecFormat fmt, const std::wstring& remark) const
{
    if (fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC) {
        auto ftseat = board_->getMoveSeat(str);
        move->setFTSeat(ftseat.first, ftseat.second);
    } else
        move->setFTSeat(board_->getSeat(PieceManager::getRowFromICCSChar(str.at(1)),
                            PieceManager::getColFromICCSChar(str.at(0))),
            board_->getSeat(PieceManager::getRowFromICCSChar(str.at(3)),
                PieceManager::getColFromICCSChar(str.at(2))));
    move->setRemark(remark);
}

void Instance::__setMoveZhStrAndNums()
{
    std::function<void(const std::shared_ptr<Move>&)>
        __setZhStrAndNums = [&](const std::shared_ptr<Move>& move) {
            ++movCount_;
            maxCol_ = std::max(maxCol_, move->otherNo());
            maxRow_ = std::max(maxRow_, move->nextNo());
            move->setCC_ColNo(maxCol_); // # 本着在视图中的列数
            if (!move->remark().empty()) {
                ++remCount_;
                remLenMax_ = std::max(remLenMax_, static_cast<int>(move->remark().size()));
            }
            move->setZhStr(board_->getZhStr(move->fseat(), move->tseat()));

            move->done();
            if (move->next())
                __setZhStrAndNums(move->next());
            move->undo();

            if (move->other()) {
                ++maxCol_;
                __setZhStrAndNums(move->other());
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->next())
        __setZhStrAndNums(rootMove_->next()); // 驱动函数
}

void Instance::__setFEN(const std::wstring& pieceChars, PieceColor color)
{
    info_[L"FEN"] = (pieCharsToFEN(pieceChars) + L" "
        + (color == PieceColor::RED ? L"r" : L"b") + L" - - 0 1");
}

const std::wstring Instance::__pieceChars() const
{
    std::wstring rfen{ info_.at(L"FEN") }, fen{ rfen.substr(0, rfen.find(L' ')) };
    return FENTopieChars(fen);
}

const std::wstring Instance::__moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow_ << L", 视图宽度：" << maxCol_ << L", 着法数量：" << movCount_
        << L", 注解数量：" << remCount_ << L", 注解最长：" << remLenMax_ << L"】\n";
    return wss.str();
}

int Instance::Move::frowcol() const { return fseat_->rowcol(); }

int Instance::Move::trowcol() const { return tseat_->rowcol(); }

const std::wstring Instance::Move::iccs() const
{
    std::wstringstream wss{};
    wss << PieceManager::getColICCSChar(fseat_->col()) << fseat_->row()
        << PieceManager::getColICCSChar(tseat_->col()) << tseat_->row();
    return wss.str();
}

const std::shared_ptr<Instance::Move>& Instance::Move::addNext()
{
    auto nextMove = std::make_shared<Move>();
    nextMove->setNextNo(nextNo_ + 1);
    nextMove->setOtherNo(otherNo_);
    nextMove->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return next_ = nextMove;
}

const std::shared_ptr<Instance::Move>& Instance::Move::addOther()
{
    auto otherMove = std::make_shared<Move>();
    otherMove->setNextNo(nextNo_);
    otherMove->setOtherNo(otherNo_ + 1);
    otherMove->setPrev(std::weak_ptr<Move>(shared_from_this()));
    return other_ = otherMove;
}

std::vector<std::shared_ptr<Instance::Move>> Instance::Move::getPrevMoves()
{
    std::shared_ptr<Move> thisMove{ shared_from_this() }, preMove{};
    std::vector<std::shared_ptr<Move>> moves{ thisMove };
    while (preMove = thisMove->prev()) {
        moves.push_back(preMove);
        thisMove = preMove;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

void Instance::Move::done()
{
    eatPie_ = fseat_->movTo(*tseat_);
}

void Instance::Move::undo() const
{
    tseat_->movTo(*fseat_, eatPie_);
}

const std::wstring Instance::Move::toString() const
{
    std::wstringstream wss{};
    wss << std::setw(2) << frowcol() << L'_' << std::setw(2) << trowcol()
        << L'-' << std::setw(4) << iccs() << L':' << std::setw(4)
        << zh() << L'{' << remark() << L'}';
    return wss.str();
}

const std::wstring getWString(std::wistream& wis)
{
    std::wstringstream wss{};
    wis >> std::noskipws >> wss.rdbuf(); // C++ standard library p847
    return wss.str();
}

const std::wstring pieCharsToFEN(const std::wstring& pieceChars)
{
    assert(pieceChars.size() == 90);
    std::wstring fen{};
    std::wregex linerg{ LR"(.{9})" };
    for (std::wsregex_token_iterator lineIter{
             pieceChars.begin(), pieceChars.end(), linerg, 0 },
         end{};
         lineIter != end; ++lineIter) {
        std::wstringstream wss{};
        int num{ 0 };
        for (auto wch : (*lineIter).str()) {
            if (wch != PieceManager::nullChar()) {
                if (num) {
                    wss << num;
                    num = 0;
                }
                wss << wch;
            } else
                ++num;
        }
        if (num)
            wss << num;
        fen.insert(0, wss.str()).insert(0, L"/");
    }
    fen.erase(0, 1);

    //assert(FENTopieChars(fen) == pieceChars);
    return fen;
}

const std::wstring FENTopieChars(const std::wstring& fen)
{
    std::wstring pieceChars{};
    std::wregex linerg{ LR"(/)" };
    for (std::wsregex_token_iterator lineIter{ fen.begin(), fen.end(), linerg, -1 };
         lineIter != std::wsregex_token_iterator{}; ++lineIter) {
        std::wstringstream wss{};
        for (auto wch : std::wstring{ *lineIter })
            wss << (isdigit(wch)
                    ? std::wstring(wch - L'0', PieceManager::nullChar())
                    : std::wstring{ wch }); // ASCII: 0:48
        pieceChars.insert(0, wss.str());
    }

    //assert(fen == pieCharsToFEN(pieceChars));
    return pieceChars;
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

void transDir(const std::string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    std::string extensions{ ".xqf.pgn_iccs.pgn_zh.pgn_cc.bin.json" };
    std::string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + getExtName(fmt) };
    std::function<void(const std::string&, const std::string&)>
        __trans = [&](const std::string& dirfrom, const std::string& dirto) {
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
                            Instance ci(infilename);
                            //std::cout << infilename << " read finished!" << std::endl;
                            //std::cout << fileto << std::endl;
                            ci.write(fileto + getExtName(fmt));
                            //std::cout << fileto + getExtName(fmt) << " write finished!" << std::endl;

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
    std::vector<RecFormat> fmts{
        RecFormat::XQF, RecFormat::BIN, RecFormat::JSON,
        RecFormat::PGN_ICCS, RecFormat::PGN_ZH, RecFormat::PGN_CC
    };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex > 0 && tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}
}