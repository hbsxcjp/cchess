#include "instance.h"
#include "../json/json.h"
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
    : board_{ std::make_shared<BoardSpace::Board>() }
    , rootMove_{ std::make_shared<MoveSpace::Move>() }
{
}

const bool Instance::isLast() const { return !currentMove_ || !currentMove_->next(); }

// 基本走法
void Instance::go()
{
    if (!isLast())
        currentMove_ = currentMove_->next()->done();
}

void Instance::back()
{
    if (!isStart())
        currentMove_ = currentMove_->undo();
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

void Instance::goInc(int inc)
{
    //std::function<void(Instance*)> fbward = inc > 0 ? &Instance::go : &Instance::back;
    auto fbward = std::mem_fn(inc > 0 ? &Instance::go : &Instance::back);
    for (int i = abs(inc); i != 0; --i)
        fbward(this);
}

void Instance::changeSide(ChangeType ct) // 未测试
{
    auto prevMoves = currentMove_->getPrevMoves();
    backFirst();
    board_->changeSide(ct);
    if (ct != ChangeType::EXCHANGE) {
        auto changeRowcol = std::mem_fn(ct == ChangeType::ROTATE ? &BoardSpace::Board::getRotate : &BoardSpace::Board::getSymmetry);
        std::function<void(MoveSpace::Move&)> __setRowcol = [&](MoveSpace::Move& move) {
            move.setFrowcol(changeRowcol(board_, move.fseat()->rowcol()));
            move.setTrowcol(changeRowcol(board_, move.tseat()->rowcol()));
            if (move.next())
                __setRowcol(*move.next());
            if (move.other())
                __setRowcol(*move.other());
        };
        if (rootMove_->fseat())
            __setRowcol(*rootMove_); // 驱动调用递归函数
        setMoves(RecFormat::BIN); //借用RecFormat::BIN
    }
    auto color = rootMove_->fseat() ? rootMove_->fseat()->piece()->color() : PieceColor::RED;
    setFEN(board_->getPieceChars(), color);
    for (auto& move : prevMoves)
        move->done();
}

const std::wstring Instance::toString() const
{
    std::ostringstream ss{};
    __writeInfo_PGN(ss);
    __writeMove_PGN_CC(ss);
    return Tools::s2ws(ss.str());
}

void Instance::read(const std::string& infilename)
{
    RecFormat fmt = getRecFormat(Tools::getExt(infilename));
    std::ifstream is{ (fmt == RecFormat::XQF || fmt == RecFormat::BIN) ? std::ifstream(infilename, std::ios_base::binary)
                                                                       : std::ifstream(infilename) };
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(is);
        break;
    case RecFormat::PGN_ICCS:
        __readInfo_PGN(is);
        __readMove_PGN_ICCSZH(is, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __readInfo_PGN(is);
        __readMove_PGN_ICCSZH(is, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __readInfo_PGN(is);
        __readMove_PGN_CC(is);
        break;
    case RecFormat::BIN:
        readBIN(is);
        break;
    case RecFormat::JSON:
        readJSON(is);
        break;
    default:
        break;
    }
    //std::wcout << L"readFile finished!" << std::endl;

    board_->reset(pieceChars());
    //std::wcout << board_->toString() << std::endl;

    setMoves(fmt);
    //std::wcout << L"setMoves finished!" << std::endl;
    currentMove_ = nullptr;
}

void Instance::write(const std::string& outfilename)
{
    RecFormat fmt = getRecFormat(Tools::getExt(outfilename));
    std::ofstream os{ (fmt == RecFormat::XQF || fmt == RecFormat::BIN) ? std::ofstream(outfilename, std::ios_base::binary)
                                                                       : std::ofstream(outfilename) };
    switch (fmt) {
    case RecFormat::XQF:
        break;
    case RecFormat::PGN_ICCS:
        __writeInfo_PGN(os);
        __writeMove_PGN_ICCSZH(os, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __writeInfo_PGN(os);
        __writeMove_PGN_ICCSZH(os, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __writeInfo_PGN(os);
        __writeMove_PGN_CC(os);
        break;
    case RecFormat::BIN:
        writeBIN(os);
        break;
    case RecFormat::JSON:
        writeJSON(os);
        break;
    default:
        break;
    }
    //std::wcout << L"writeMove finished!\n" << MoveOwner_->toString() << std::endl;
}

void Instance::readXQF(std::istream& is)
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

    is.read(Signature, 2).get(Version).get(headKeyMask).read(ProductId, 4); // = 8 bytes
    is.get(headKeyOrA).get(headKeyOrB).get(headKeyOrC).get(headKeyOrD);
    is.get(headKeysSum).get(headKeyXY).get(headKeyXYf).get(headKeyXYt); // = 16 bytes
    is.read(headQiziXY, pieceNum); // = 48 bytes
    is.read(PlayStepNo, 2).get(headWhoPlay).get(headPlayResult);
    is.read(PlayNodes, 4).read(PTreePos, 4).read(Reserved1, 4); // = 64 bytes
    is.read(headCodeA_H, 16).read(TitleA, 64).read(TitleB, 64);
    is.read(Event, 64).read(Date, 16).read(Site, 16).read(Red, 16).read(Black, 16);
    is.read(Opening, 64).read(Redtime, 16).read(Blktime, 16).read(Reservedh, 32);
    is.read(RMKWriter, 16).read(Author, 16);

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
    std::wstring pieceChars(90, BoardSpace::Board::nullChar);
    std::wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    for (int i = 0; i != pieceNum; ++i) {
        int xy = head_QiziXY[i];
        if (xy <= 89) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
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
        { L"FEN", getFEN(pieceChars) } // 可能存在不是红棋先走的情况？
    };

    std::function<unsigned char(unsigned char, unsigned char)> __sub = [](unsigned char a, unsigned char b) {
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
    std::function<std::wstring()> __readDataAndGetRemark = [&]() {
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
    std::function<void(MoveSpace::Move&)> __readMove = [&](MoveSpace::Move& move) {
        move.setRemark(__readDataAndGetRemark());
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
        assert(fcolrow <= 89 && tcolrow <= 89);
        move.setFrowcol((fcolrow % 10) * 10 + fcolrow / 10);
        move.setTrowcol((tcolrow % 10) * 10 + tcolrow / 10);
        //std::wcout << move.toString() << std::endl;

        char ntag{ tag };
        if (ntag & 0x80) //# 有左子树
            __readMove(*move.addNext());
        if (ntag & 0x40) // # 有右子树
            __readMove(*move.addOther());
    };

    is.seekg(1024);
    remark_ = __readDataAndGetRemark();
    char rtag{ tag };
    if (rtag & 0x80) //# 有左子树
        __readMove(*rootMove_);
    if (rtag & 0x40) { // # 有右子树
        std::wcout << L"*rootMove_ 有右子树!" << std::endl;
        __readMove(*rootMove_);
    }
}

const std::wstring Instance::__getMoveStr(std::istream& is) const
{
    std::stringstream ss{};
    std::string line{};
    is >> std::noskipws;
    while (std::getline(is, line)) // 以空行为分割，接info read之后
        ss << line << '\n';
    return Tools::s2ws(ss.str());
}

void Instance::__readInfo_PGN(std::istream& is)
{
    std::stringstream ss{};
    std::string line{};
    is >> std::noskipws;
    while (std::getline(is, line) && !line.empty()) // 以空行为分割
        ss << line << '\n';
    std::wstring infoStr{ Tools::s2ws(ss.str()) };
    std::wregex pat{ LR"(\[(\w+)\s+\"([\s\S]*?)\"\])" };
    for (std::wsregex_iterator p(infoStr.begin(), infoStr.end(), pat); p != std::wsregex_iterator{}; ++p)
        info_[(*p)[1]] = (*p)[2];
}

void Instance::__writeInfo_PGN(std::ostream& os) const
{
    std::wstringstream wss{};
    std::for_each(info_.begin(), info_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        wss << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
    });
    wss << L'\n'; // 以空行为分割
    os << Tools::ws2s(wss.str());
}

void Instance::__readMove_PGN_ICCSZH(std::istream& is, RecFormat fmt)
{
    const std::wstring moveStr{ __getMoveStr(is) };
    //std::wcout << moveStr << std::endl;
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::PGN_ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                                 : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr }, rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" },
        spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    std::wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, wtiend{};
    std::wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        remark_ = wsm.str(1);
    bool isRoot{ true }, isOther{ false }; // 首次非变着
    auto __setZhIccs = std::mem_fn(fmt == RecFormat::PGN_ZH ? &MoveSpace::Move::setZh : &MoveSpace::Move::setIccs);
    auto __readMove = [&](std::shared_ptr<MoveSpace::Move> pMove_, const std::wstring mvstr, bool isOther_) { //# 非递归
        for (std::wsregex_iterator wi(mvstr.begin(), mvstr.end(), moveReg), wiend{}; wi != wiend; ++wi) {
            if (!isRoot)
                pMove_ = isOther_ ? pMove_->addOther() : pMove_->addNext();
            __setZhIccs(pMove_, (*wi)[1]);
            pMove_->setRemark((*wi)[2]);
            //std::wcout << pMove_->toString() << std::endl;
            isRoot = isOther_ = false; // # 仅第一步不增加Move/且可为other，后续全增加Move/为next
        }
        return pMove_;
    };
    std::shared_ptr<MoveSpace::Move> pPreMove{};
    std::vector<std::shared_ptr<MoveSpace::Move>> othMoves{ rootMove_ };
    for (; wtleft != wtiend; ++wtleft) {
        //std::wcout << *wtleft << L"\n---------------------------------------------\n" << std::endl;
        for (std::wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
             wtright != wtiend; ++wtright) {
            //std::wcout << *wtright << L"\n---------------------------------------------\n" << std::endl;
            pPreMove = __readMove(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(pPreMove);
        isOther = true;
    }
}

void Instance::__writeMove_PGN_ICCSZH(std::ostream& os, RecFormat fmt) const
{
    std::wstringstream wss{};
    auto __writeRemark = [&](std::wstring remark) {
        if (!remark.empty())
            wss << (L"\n{" + remark + L"}\n");
    };
    std::function<void(const MoveSpace::Move&, bool)> __writeMove = [&](const MoveSpace::Move& move, bool isOther) {
        std::wstring boutNum{ std::to_wstring((move.nextNo() + 1) / 2) };
        bool isEven{ move.nextNo() % 2 == 0 };
        wss << (isOther ? L"(" + boutNum + L". " + (isEven ? L"... " : L"") : (isEven ? L" " : boutNum + L". "))
            << (fmt == RecFormat::PGN_ZH ? move.zh() : move.iccs()) << L' ';
        __writeRemark(move.remark());
        if (move.other()) {
            __writeMove(*move.other(), true);
            wss << L") ";
        }
        if (move.next())
            __writeMove(*move.next(), false);
    };

    __writeRemark(remark_);
    if (rootMove_->fseat())
        __writeMove(*rootMove_, false);
    os << Tools::ws2s(wss.str());
}

void Instance::__readMove_PGN_CC(std::istream& is)
{
    const std::wstring imoveStr{ __getMoveStr(is) };
    auto pos0 = imoveStr.find(L"\n("), pos1 = imoveStr.find(L"\n【");
    std::wstring moveStr{ imoveStr.substr(0, std::min(pos0, pos1)) }, remStr{ imoveStr.substr(std::min(pos0, imoveStr.size()), pos1) };
    std::wregex lingrg{ LR"(\n)" }, mvStrrg{ LR"(.{5})" }, moverg{ LR"(([^…　]{4}[…　]))" },
        remrg{ LR"(\s*(\(\d+,\d+\)): \{([\s\S]*?)\})" };
    std::vector<std::vector<std::wstring>> movlines{};
    for (std::wsregex_token_iterator lineStrit{ moveStr.begin(), moveStr.end(), lingrg, -1 }, end{}; lineStrit != end; ++++lineStrit) {
        std::vector<std::wstring> lines{};
        for (std::wsregex_token_iterator msit{ (*lineStrit).first, (*lineStrit).second, mvStrrg, 0 }; msit != end; ++msit)
            lines.push_back(*msit);
        movlines.push_back(lines);
    }
    std::map<std::wstring, std::wstring> rems{};
    for (std::wsregex_iterator rp{ remStr.begin(), remStr.end(), remrg }; rp != std::wsregex_iterator{}; ++rp)
        rems[(*rp)[1]] = (*rp)[2];

    std::function<void(MoveSpace::Move&, int, int)> __readMove = [&](MoveSpace::Move& move, int row, int col) {
        std::wstring zhStr{ movlines[row][col] };
        if (regex_match(zhStr, moverg)) {
            move.setZh(zhStr.substr(0, 4));
            move.setRemark(rems[L'(' + std::to_wstring(row) + L',' + std::to_wstring(col) + L')']);
            if (zhStr.back() == L'…')
                __readMove(*move.addOther(), row, col + 1);
            if (int(movlines.size()) - 1 > row && movlines[row + 1][col][0] != L'　')
                __readMove(*move.addNext(), row + 1, col);
        } else if (movlines[row][col][0] == L'…') {
            while (movlines[row][++col][0] == L'…')
                ;
            __readMove(move, row, col);
        }
    };

    remark_ = rems[L"(0,0)"];
    if (!movlines.empty())
        __readMove(*rootMove_, 1, 0);
}

void Instance::__writeMove_PGN_CC(std::ostream& os) const
{
    std::wstringstream remStrs{};
    std::wstring lstr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, lstr);
    std::function<void(const MoveSpace::Move&)> __setMoveZH = [&](const MoveSpace::Move& move) {
        int firstcol{ move.CC_ColNo() * 5 }, row{ move.nextNo() * 2 };
        //assert(move.zh().size() == 4);
        lineStr.at(row).replace(firstcol, 4, move.zh());
        if (!move.remark().empty())
            remStrs << L"(" << move.nextNo() << L"," << move.CC_ColNo() << L"): {" << move.remark() << L"}\n";
        if (move.next()) {
            lineStr.at(row + 1).at(firstcol + 2) = L'↓';
            __setMoveZH(*move.next());
        }
        if (move.other()) {
            int fcol{ firstcol + 4 }, num{ move.other()->CC_ColNo() * 5 - fcol };
            lineStr.at(row).replace(fcol, num, std::wstring(num, L'…'));
            __setMoveZH(*move.other());
        }
    };

    if (!remark_.empty())
        remStrs << L"(0,0): {" << remark_ << L"}\n";
    lineStr.front().replace(0, 3, L"　开始");
    lineStr.at(1).at(2) = L'↓';
    if (rootMove_->fseat())
        __setMoveZH(*rootMove_);
    std::wstringstream wss{};
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << moveInfo();
    os << Tools::ws2s(wss.str());
}

void Instance::readBIN(std::istream& is)
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
    std::function<void(MoveSpace::Move&)> __readMove = [&](MoveSpace::Move& move) {
        char tag{};
        is.get(frowcol).get(trowcol).get(tag);
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        move.setRemark(__readWstring());
        if (tag & 0x80)
            __readMove(*move.addNext());
        if (tag & 0x40)
            __readMove(*move.addOther());
    };

    is.read(len, sizeof(int));
    int size{ *(int*)len };
    std::wstring key{}, value{};
    for (int i = 0; i < size; ++i) { // 以size为分割
        key = __readWstring();
        value = __readWstring();
        info_[key] = value;
    }
    remark_ = __readWstring();
    char rtag{};
    is.get(rtag);
    if (rtag)
        __readMove(*rootMove_);
}

void Instance::writeBIN(std::ostream& os) const
{
    auto __writeWstring = [&](const std::wstring& wstr) {
        std::string str{ Tools::ws2s(wstr) };
        int len = str.size();
        os.write((char*)&len, sizeof(int)).write(str.c_str(), len);
    };
    std::function<void(const MoveSpace::Move&)> __writeMove = [&](const MoveSpace::Move& move) {
        os.put(move.frowcol()).put(move.trowcol()).put((move.next() ? 0x80 : 0x00) | (move.other() ? 0x40 : 0x00));
        __writeWstring(move.remark());
        if (move.next())
            __writeMove(*move.next());
        if (move.other())
            __writeMove(*move.other());
    };

    int len = info_.size();
    os.write((char*)&len, sizeof(int));
    std::for_each(info_.begin(), info_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        __writeWstring(kv.first);
        __writeWstring(kv.second);
    });
    __writeWstring(remark_); // 至少会写入0
    os.put(bool(rootMove_->fseat()));
    if (rootMove_->fseat())
        __writeMove(*rootMove_);
}

void Instance::readJSON(std::istream& is)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, is, &root, &errs))
        return;

    Json::Value infoItem{ root["info"] };
    for (auto& key : infoItem.getMemberNames())
        info_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    std::function<void(MoveSpace::Move&, Json::Value&)> __readMove = [&](MoveSpace::Move& move, Json::Value& item) {
        int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        if (item.isMember("r"))
            move.setRemark(Tools::s2ws(item["r"].asString()));
        if (item.isMember("n")) //# 有左子树
            __readMove(*move.addNext(), item["n"]);
        if (item.isMember("o")) // # 有右子树
            __readMove(*move.addOther(), item["o"]);
    };
    remark_ = Tools::s2ws(root["remark"].asString());
    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __readMove(*rootMove_, rootItem);
}

void Instance::writeJSON(std::ostream& os) const
{
    Json::Value root{}, infoItem{};
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::for_each(info_.begin(), info_.end(), [&](const std::pair<std::wstring, std::wstring>& kv) {
        infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
    });
    root["info"] = infoItem;
    std::function<Json::Value(const MoveSpace::Move&)> __writeItem = [&](const MoveSpace::Move& move) {
        Json::Value item{};
        item["f"] = move.frowcol();
        item["t"] = move.trowcol();
        if (!move.remark().empty())
            item["r"] = Tools::ws2s(move.remark());
        if (move.next())
            item["n"] = __writeItem(*move.next());
        if (move.other())
            item["o"] = __writeItem(*move.other());
        return std::move(item);
    };
    root["remark"] = Tools::ws2s(remark_);
    root["moves"] = __writeItem(*rootMove_);
    writer->write(root, &os);
}

void Instance::setFEN(const std::wstring& pieceChars, PieceColor color)
{
    info_[L"FEN"] = getFEN(pieceChars) + L" " + (color == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
}

const std::wstring Instance::pieceChars() const
{
    std::wstring rfen{ info_.at(L"FEN") }, fen{ rfen.substr(0, rfen.find(L' ')) };
    return getPieceChars(fen);
}

// （rootMove）调用, 设置树节点的seat or zh'  // C++primer P512
void Instance::setMoves(RecFormat fmt)
{
    std::function<void(MoveSpace::Move&)> __set = [&](MoveSpace::Move& move) {
        //std::wcout << move.toString() << std::endl;

        if (fmt == RecFormat::PGN_ICCS || fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC) {
            auto moveSeats = fmt == RecFormat::PGN_ICCS ? board_->getMoveSeatFromIccs(move.iccs())
                                                        : board_->getMoveSeatFromZh(move.zh());
            move.setFrowcol(moveSeats.first->rowcol());
            move.setTrowcol(moveSeats.second->rowcol());
            move.setSeats(moveSeats);
        } else //RecFormat::XQF RecFormat::BIN RecFormat::JSON
            move.setSeats(board_->getSeat(move.frowcol()), board_->getSeat(move.trowcol()));
        if (fmt != RecFormat::PGN_ZH && fmt != RecFormat::PGN_CC)
            ;//move.setZh(board_->getZh(move.fseat(), move.tseat()));
        if (fmt != RecFormat::PGN_ICCS)
            move.setIccs(board_->getIccs(move.fseat(), move.tseat()));

        //std::wcout << L"   ?: " << move.toString() << std::endl;
        assert(move.frowcol() >= 0 && move.frowcol() <= 98);
        assert(move.trowcol() >= 0 && move.trowcol() <= 98);
        //assert(move.zh().size() == 4);
        assert(move.iccs().size() == 4);
        assert(move.fseat()->piece());

        ++movCount;
        maxCol = std::max(maxCol, move.otherNo());
        maxRow = std::max(maxRow, move.nextNo());
        move.setCC_ColNo(maxCol); // # 本着在视图中的列数
        if (!move.remark().empty()) {
            ++remCount;
            remLenMax = std::max(remLenMax, static_cast<int>(move.remark().size()));
        }

        move.done();
        //std::wcout << L"done: " << move.toString() << L'\n' << board_->toString() << std::endl;
        if (move.next())
            __set(*move.next());

        move.undo();
        //std::wcout << L"undo: " << move.toString() << L'\n' << board_->toString() << std::endl;

        if (move.other()) {
            ++maxCol;
            __set(*move.other());
        }
    };

    if (rootMove_->frowcol() >= 0 || !rootMove_->iccs().empty() || !rootMove_->zh().empty())
        __set(*rootMove_); // 驱动函数
}

const std::wstring
Instance::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
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

                        Instance ci{};
                        //std::cout << infilename << std::endl;
                        ci.read(infilename);
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
    std::vector<RecFormat> fmts{ RecFormat::XQF, RecFormat::PGN_ICCS, RecFormat::PGN_ZH, RecFormat::PGN_CC,
        RecFormat::BIN, RecFormat::JSON };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex > 0 && tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}

const std::wstring test()
{
    InstanceSpace::Instance ins{};
    ins.read("01.xqf");
    ins.write("01.pgn_iccs");

    ins = InstanceSpace::Instance{};
    ins.read("01.pgn_iccs");
    ins.write("01.pgn_zh");

    ins = InstanceSpace::Instance{};
    ins.read("01.pgn_zh");
    ins.write("01.pgn_cc");

    ins = InstanceSpace::Instance{};
    ins.read("01.pgn_cc");
    ins.write("01.bin");

    ins = InstanceSpace::Instance{};
    ins.read("01.bin");
    ins.write("01.json");

    ins = InstanceSpace::Instance{};
    ins.read("01.json");
    std::wstringstream wss{};
    wss << ins.toString();

    return wss.str();
}
}