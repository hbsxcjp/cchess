#include "Instance.h"
#include "../json/json.h"
#include "board.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <cmath>
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
    , rootMove_{ std::make_shared<Instance::Move>() }
    , currentMove_{ rootMove_ }
    , firstColor_{ PieceColor::RED }
{
}

Instance::Instance(const std::string& filename)
    : Instance()
{
    RecFormat fmt{ getRecFormat(Tools::getExt(filename)) };
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(filename);
        break;
    case RecFormat::BIN:
        readBIN(filename);
        break;
    case RecFormat::JSON:
        readJSON(filename);
        break;
    default:
        readPGN(filename, fmt);
        break;
    }
    std::wcout << L"readFile finished!" << std::endl;
    setBoard();
    std::wcout << L"setBoard finished!" << std::endl;
    setMoves(fmt);
    std::wcout << L"setMoves finished!" << std::endl;
}

void Instance::write(const std::string& fname, const RecFormat fmt) // const
{
    std::string filename{ fname + getExtName(fmt) };
    info_[L"Format"] = Tools::s2ws(getExtName(fmt));
    switch (fmt) {
    case RecFormat::BIN:
        writeBIN(filename);
        break;
    case RecFormat::JSON:
        writeJSON(filename);
        break;
    default:
        writePGN(filename, fmt);
        break;
    }
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
void Instance::forwardOther()
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

void Instance::moveInc(const int inc)
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
        std::function<void(Move&)> __setSeat = [&](Move& move) {
            move.setSeats(board_->getOthSeat(move.fseat_, ct), board_->getOthSeat(move.tseat_, ct));
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

void Instance::readXQF(const std::string& filename)
{
    std::ifstream ifs(filename, std::ios_base::binary);
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
    info_[L"Version_xqf"] = std::to_wstring(version);
    info_[L"Result"] = (std::map<char, std::wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult[0]];
    info_[L"PlayType"] = (std::map<char, std::wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]];
    info_[L"TitleA"] = Tools::s2ws(TitleA);
    info_[L"Event"] = Tools::s2ws(Event);
    info_[L"Date"] = Tools::s2ws(Date);
    info_[L"Site"] = Tools::s2ws(Site);
    info_[L"Red"] = Tools::s2ws(Red);
    info_[L"Black"] = Tools::s2ws(Black);
    info_[L"Opening"] = Tools::s2ws(Opening);
    info_[L"RMKWriter"] = Tools::s2ws(RMKWriter);
    info_[L"Author"] = Tools::s2ws(Author);

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
    setFEN(pieceChars);
    //std::wcout << info_[L"FEN"] << std::endl;

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
        move.setSeats(board_->getSeat(fcolrow % 10, fcolrow / 10), board_->getSeat(tcolrow % 10, tcolrow / 10));
        //int frowcol{ fcolrow % 10 * 10 + fcolrow / 10 }, trowcol{ tcolrow % 10 * 10 + tcolrow / 10 }; // 行列转换
        //move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));

        //std::wcout << frowcol << L' ' << trowcol << std::endl;
        //const std::shared_ptr<Seat>&fseat{ board_->getSeat(frowcol % 10, frowcol / 10) }, &tseat{ board_->getSeat(trowcol % 10, trowcol / 10) };
        //std::wcout << move.toString() << std::endl;
        //move.setSeats(fseat, tseat);
        //move.setSeats(__byteToSeat(data[0], 0X18 + KeyXYf), __byteToSeat(data[1], 0X20 + KeyXYt));

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

    __read(*rootMove_);
}

void Instance::readPGN(const std::string& filename, const RecFormat fmt)
{
    std::wstring pgnTxt{ Tools::readTxt(filename) };
    auto pos = pgnTxt.find(L"》");
    std::wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    std::wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (std::wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != std::wsregex_iterator{}; ++p)
        info_[(*p)[1]] = (*p)[2];
    fmt == RecFormat::CC ? __readCC(moveStr) : __readICCSZH(moveStr, fmt);
}

void Instance::__readICCSZH(const std::wstring& moveStr, const RecFormat fmt)
{
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                             : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](std::shared_ptr<Instance::Move> move, const std::wstring mvstr, bool isOther) { //# 非递归
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

    std::shared_ptr<Instance::Move> move;
    std::vector<std::shared_ptr<Instance::Move>> othMoves{ rootMove_ };
    std::wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    std::wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    std::wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        rootMove_->remark_ = wsm.str(1);
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

void Instance::__readCC(const std::wstring& fullMoveStr)
{
    auto pos = fullMoveStr.find(L"\n(");
    std::wstring moveStr{ fullMoveStr.substr(0, pos) }, remStr{ pos < fullMoveStr.size() ? fullMoveStr.substr(pos) : L"" };
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

    __setRem(*rootMove_, 0, 0);
    if (int(movv.size()) > 0)
        __read(*rootMove_, 1, 0, false);
}

void Instance::readBIN(const std::string& filename)
{
    std::ifstream ifs(filename, std::ios_base::binary);
    char size{}, klen{}, vlen{};
    ifs.get(size);
    for (int i = 0; i != size; ++i) {
        ifs.get(klen);
        char key[klen + 1]{};
        ifs.read(key, klen);
        ifs.get(vlen);
        char value[vlen + 1]{};
        ifs.read(value, vlen);
        info_[Tools::s2ws(key)] = Tools::s2ws(value);
    }
    std::function<void(Move&)> __read = [&](Move& move) {
        char frowcol{}, trowcol{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        ifs.get(frowcol).get(trowcol).get(tag);
        move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
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

    __read(*rootMove_);
}

void Instance::readJSON(const std::string& filename)
{
    std::ifstream ifs(filename);
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    Json::Value infoItem{ root["info_"] };
    for (auto& key : infoItem.getMemberNames())
        info_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    std::function<void(Move&, Json::Value&)> __read = [&](Move& move, Json::Value& item) {
        int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
        move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        if (item.isMember("r"))
            move.remark_ = Tools::s2ws(item["r"].asString());
        if (item.isMember("n")) //# 有左子树
            __read(*move.addNext(), item["n"]);
        if (item.isMember("o")) // # 有右子树
            __read(*move.addOther(), item["o"]);
    };

    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __read(*rootMove_, rootItem);
}

void Instance::writeBIN(const std::string& filename) const
{
    std::ofstream ofs(filename, std::ios_base::binary);
    ofs.put(char(info_.size()));
    for (auto& kv : info_) {
        std::string keys{ Tools::ws2s(kv.first) }, values{ Tools::ws2s(kv.second) };
        char klen{ char(keys.size()) }, vlen{ char(values.size()) };
        ofs.put(klen).write(keys.c_str(), klen).put(vlen).write(values.c_str(), vlen);
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

    __write(*rootMove_);
}

void Instance::writeJSON(const std::string& filename) const
{
    std::ofstream ofs(filename);
    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value infoItem;
    for (auto& kv : info_)
        infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
    root["info_"] = infoItem;

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

    root["moves"] = __writeItem(*rootMove_);
    writer->write(root, &ofs);
}

void Instance::writePGN(const std::string& filename, const RecFormat fmt) const
{
    std::wstringstream wss{};
    for (const auto& kv : info_)
        wss << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
    Tools::writeTxt(filename, wss.str() + L"》" + (fmt == RecFormat::CC ? toString_CC() : toString_ICCSZH(fmt)));
}

const std::wstring Instance::toString_ICCSZH(const RecFormat fmt) const
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

    __remark(*rootMove_);
    if (rootMove_->next_)
        __moveStr(*rootMove_->next_, false);
    return wss.str();
}

const std::wstring Instance::toString_CC() const
{
    std::wstringstream remStrs{};
    std::wstring lstr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, lstr);
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

    __setChar(*rootMove_);
    std::wstringstream wss{};
    lineStr.front().replace(0, 2, L"开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << __moveInfo();
    return wss.str();
}

const std::wstring Instance::__moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

void Instance::setFEN(const std::wstring& pieceChars)
{
    info_[L"FEN"] = getFEN(pieceChars) + L" " + (firstColor_ == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
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
        if (fmt == RecFormat::ICCS || fmt == RecFormat::ZH || fmt == RecFormat::CC) {
            move.setSeats(board_->getMoveSeat(fmt == RecFormat::ICCS ? move.iccs_ : move.zh_, fmt));
            std::cout << "getMoveSeat() finished! " << std::endl;
        }

#ifndef NDEBUG
        if (!move.fseat_->piece())
            std::cout << "Error! " << __FILE__ << ": in function: " << __func__ << ", at line: " << __LINE__ << std::endl;
#endif

        if (fmt != RecFormat::ZH && fmt != RecFormat::CC) {
            std::wcout << L"getZh() : " << move.fseat_->toString() << L' ' << move.tseat_->toString()
                << L'\n' << board_->toString() << std::endl;
            move.zh_ = board_->getZh(move.fseat_, move.tseat_);
            std::cout << "getZh() finished! " << std::endl;
        }
        if (fmt != RecFormat::ICCS) { //RecFormat::XQF RecFormat::BIN RecFormat::JSON
            move.iccs_ = board_->getIccs(move.fseat_, move.tseat_);
            std::cout << "getIccs() finished! " << std::endl;
        }

#ifndef NDEBUG
        if (move.zh_.size() != 4)
            std::cout << "Error! " << __FILE__ << ": in function: " << __func__ << ", at line: " << __LINE__ << std::endl;
        if (move.iccs_.size() != 4)
            std::cout << "Error! " << __FILE__ << ": in function: " << __func__ << ", at line: " << __LINE__ << std::endl;
#endif

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
    return fen;
}

const std::wstring getPieceChars(const std::wstring& fen)
{
    std::wstring pieceChars{};
    std::wregex sp{ LR"(/)" };
    for (std::wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != std::wsregex_token_iterator{}; ++wti) {
        std::wstringstream line{};
        for (const auto& wch : std::wstring{ *wti })
            line << (isdigit(wch) ? std::wstring(wch - 48, BoardSpace::nullChar) : std::wstring{ wch }); // ASCII: 0:48
        pieceChars.insert(0, line.str());
    }
    return pieceChars;
}

const std::wstring Instance::toString() const
{
    std::wstringstream wss{};
    wss << board_->toString() << toString_CC();
    return wss.str();
}

const std::shared_ptr<Instance::Move>& Instance::Move::setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat,
    const std::shared_ptr<SeatSpace::Seat>& tseat)
{
    fseat_ = fseat;
    tseat_ = tseat;
    return std::move(shared_from_this());
}

const std::shared_ptr<Instance::Move>& Instance::Move::setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>,
    const std::shared_ptr<SeatSpace::Seat>>& seats)
{
    return setSeats(seats.first, seats.second);
}

const std::shared_ptr<Instance::Move>& Instance::Move::addNext()
{
    auto next = std::make_shared<Instance::Move>();
    next->n_ = n_ + 1; // 步序号
    next->o_ = o_; // 变着层数
    next->prev_ = std::weak_ptr<Move>(shared_from_this());
    return next_ = next;
}

const std::shared_ptr<Instance::Move>& Instance::Move::addOther()
{
    auto other = std::make_shared<Instance::Move>();
    other->n_ = n_; // 与premove的步数相同
    other->o_ = o_ + 1; // 变着层数
    other->prev_ = std::weak_ptr<Move>(shared_from_this());
    return other_ = other;
}

std::vector<std::shared_ptr<Instance::Move>> Instance::Move::getPrevMoves()
{
    std::shared_ptr<Instance::Move> this_move{ shared_from_this() }, prev_move{};
    std::vector<std::shared_ptr<Instance::Move>> moves{ this_move };
    while ((prev_move = this_move->prev_.lock()) && prev_move->prev_.lock()) { // 排除rootMove
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const std::shared_ptr<Instance::Move>& Instance::Move::done()
{
    eatPie_ = fseat_->to(tseat_);
    return next_;
}

const std::shared_ptr<Instance::Move>& Instance::Move::undo()
{
    tseat_->to(fseat_, eatPie_);
    return std::move(prev_.lock());
}

const std::wstring Instance::Move::toString() const
{
    std::wstringstream wss{};
    wss << fseat_->toString() << L'>' << tseat_->toString() << L'-' << (eatPie_ ? eatPie_->name() : L' ')
        << iccs_ << L' ' << zh_ << L' ' << n_ << L' ' << o_ << L' ' << CC_Col_ << L' ' << remark_;
    return wss.str();
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
    wss << toString_ICCSZH() << L'\n' << toString_ICCSZH(RecFormat::ICCS)
        << L'\n' << __moveInfo() << toString();
    return wss.str();
}
}