#include "move.h"
#include "../json/json.h"
#include "board.h"
#include "info.h"
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
    frowcol_ = fseat->rowcol();
    tseat_ = tseat;
    trowcol_ = tseat->rowcol();
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
    while (prev_move = this_move->prev()) {
        moves.push_back(prev_move);
        this_move = prev_move;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

const std::shared_ptr<Move>& Move::done()
{
    eatPie_ = fseat_->movTo(tseat_);
    return next_;
}

const std::shared_ptr<Move>& Move::undo()
{
    tseat_->movTo(fseat_, eatPie_);
    return std::move(prev());
}

const std::wstring Move::toString() const
{
    std::wstringstream wss{};
    wss << std::setw(2) << std::to_wstring(frowcol_) << L'>' << std::setw(2) << std::to_wstring(trowcol_)
        << L'-' << std::setw(4) << iccs_ << L':' << std::setw(4) << zh_ << L'{' << remark_ << L'}';
    return wss.str();
}

void MoveOwner::read(std::istream& is, RecFormat fmt, const BoardSpace::Board& board, const InfoSpace::Key& key)
{
    rootMove_ = std::make_shared<Move>();
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(is, key);
        break;
    case RecFormat::PGN_ICCS:
        readPGN_ICCSZH(is, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        readPGN_ICCSZH(is, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        readPGN_CC(is);
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
    hasMove_ = !rootMove_->iccs().empty() || !rootMove_->zh().empty() || rootMove_->frowcol() != -1;
    if (hasMove_)
        setMoves(fmt, board);
}

void MoveOwner::write(std::ostream& os, RecFormat fmt) const
{
    if (!hasMove_)
        return;
    switch (fmt) {
    case RecFormat::XQF:
        writeXQF(os);
        break;
    case RecFormat::PGN_ICCS:
        writePGN_ICCSZH(os, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        writePGN_ICCSZH(os, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        writePGN_CC(os);
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
}

// （rootMove）调用, 设置树节点的seat or zh'  // C++primer P512
void MoveOwner::setMoves(RecFormat fmt, const BoardSpace::Board& board)
{
    std::function<void(Move&)> __set = [&](Move& move) {
        //std::wcout << move.toString() << std::endl;

        if (fmt == RecFormat::PGN_ICCS)
            move.setSeats(board.getMoveSeatFromIccs(move.iccs()));
        else if (fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC)
            move.setSeats(board.getMoveSeatFromZh(move.zh()));
        else
            move.setSeats(board.getSeat(move.frowcol()), board.getSeat(move.trowcol()));

#ifndef NDEBUG
        if (!move.fseat()->piece())
            std::wcout << board.toString() << "\n"
                       << move.frowcol() << " " << move.trowcol() << "\n"
                       << move.toString() << std::endl;
#endif
        assert(move.fseat()->piece());

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
        if (!move.remark().empty()) {
            ++remCount;
            remLenMax = std::max(remLenMax, static_cast<int>(move.remark().size()));
        }

        move.done();
        if (move.next())
            __set(*move.next());
        move.undo();
        if (move.other()) {
            ++maxCol;
            __set(*move.other());
        }
    };

    __set(*rootMove_); // 驱动函数
}

void MoveOwner::changeSide(const BoardSpace::Board* board, std::_Mem_fn<const int (BoardSpace::Board::*)(int rowcol) const> changeRowcol)
{
    std::function<void(MoveSpace::Move&)> __setRowcol = [&](MoveSpace::Move& move) {
        move.setFrowcol(changeRowcol(board, move.fseat()->rowcol()));
        move.setTrowcol(changeRowcol(board, move.tseat()->rowcol()));
        if (move.next())
            __setRowcol(*move.next());
        if (move.other())
            __setRowcol(*move.other());
    };
    if (rootMove_)
        __setRowcol(*rootMove_); // 驱动调用递归函数
    setMoves(RecFormat::BIN, *board); //借用RecFormat::BIN
}

const std::wstring MoveOwner::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

const std::wstring MoveOwner::toString() const
{
    std::ostringstream ss{};
    writePGN_CC(ss);
    return Tools::s2ws(ss.str());
}

void MoveOwner::readXQF(std::istream& is, const InfoSpace::Key& key)
{
    char data[4]{}, &frc{ data[0] }, &trc{ data[1] }, &tag{ data[2] }, clen[4]{};
    int* len{ (int*)clen };
    std::function<unsigned char(unsigned char, unsigned char)> __sub = [](unsigned char a, unsigned char b) {
        return a - b;
    }; // 保持为<256
    auto __readBytes = [&](char* bytes, int size) {
        int pos = is.tellg();
        is.read(bytes, size);
        if (key.Version_XQF > 10) // '字节解密'
            for (int i = 0; i != size; ++i)
                bytes[i] = __sub(bytes[i], key.F32Keys[(pos + i) % 32]);
    };
    auto __getRemarksize = [&]() {
        __readBytes(clen, 4);
        return *len - key.KeyRMKSize;
    };
    std::function<std::wstring()> __readDataAndGetRemark = [&]() {
        __readBytes(data, 4);
        int RemarkSize = 0;
        if (key.Version_XQF <= 10) {
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
    std::function<void(Move&)> __readMove = [&](Move& move) {
        move.setRemark(__readDataAndGetRemark());
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(frc, 0X18 + key.KeyXYf), tcolrow = __sub(trc, 0X20 + key.KeyXYt);
        assert(fcolrow <= 89 && tcolrow <= 89);
        move.setFrowcol((fcolrow % 10) * 10 + fcolrow / 10);
        move.setTrowcol((tcolrow % 10) * 10 + tcolrow / 10);
        char ntag{ tag };
        if (ntag & 0x80) //# 有左子树
            __readMove(*move.addNext());
        if (ntag & 0x40) // # 有右子树
            __readMove(*move.addOther());
    };

    is.seekg(1024);
    remark_ = __readDataAndGetRemark();
    if (tag & 0x80) //# 有左子树
        __readMove(*rootMove_);
}

void MoveOwner::writeXQF(std::ostream& os) const {}

const std::wstring MoveOwner::__getMoveStr(std::istream& is) const
{
    std::stringstream ss{};
    std::string line{};
    is >> std::noskipws;
    while (std::getline(is, line)) // 以空行为分割，接info read之后
        ss << line << '\n';
    return Tools::s2ws(ss.str());
}

void MoveOwner::readPGN_ICCSZH(std::istream& is, RecFormat fmt)
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
    auto __readMove = [&](std::shared_ptr<Move> pMove_, const std::wstring mvstr, bool isOther_) { //# 非递归
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
    std::shared_ptr<Move> pPreMove{};
    std::vector<std::shared_ptr<Move>> othMoves{ rootMove_ };
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

void MoveOwner::writePGN_ICCSZH(std::ostream& os, RecFormat fmt) const
{
    std::wstringstream wss{};
    auto __writeRemark = [&](std::wstring remark) {
        if (!remark.empty())
            wss << (L"\n{" + remark + L"}\n");
    };
    std::function<void(const Move&, bool)> __writeMove = [&](const Move& move, bool isOther) {
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
    __writeMove(*rootMove_, false);
    os << Tools::ws2s(wss.str());
}

void MoveOwner::readPGN_CC(std::istream& is)
{
    const std::wstring imoveStr{ __getMoveStr(is) };
    auto pos = imoveStr.find(L"\n(");
    std::wstring moveStr{ imoveStr.substr(0, pos) }, remStr{ imoveStr.substr(pos) };
    std::wregex lingrg{ LR"(\n)" }, mvStrrg{ LR"(.{5})" }, moverg{ LR"(([^…　]{4}[…　]))" },
        remrg{ LR"(\(\s*(\d+,\d+)\): \{([\s\S]*?)\})" };
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

    std::function<void(Move&, int, int)> __readMove = [&](Move& move, int row, int col) {
        std::wstring zhStr{ movlines[row][col] };
        if (regex_match(zhStr, moverg)) {
            move.setZh(zhStr.substr(0, 4));
            move.setRemark(rems[std::to_wstring(row) + L',' + std::to_wstring(col)]);
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

    remark_ = rems[L"0,0"];
    if (!movlines.empty())
        __readMove(*rootMove_, 1, 0);
}

void MoveOwner::writePGN_CC(std::ostream& os) const
{
    std::wstringstream remStrs{};
    std::wstring lstr((getMaxCol() + 1) * 5, L'　');
    std::vector<std::wstring> lineStr((getMaxRow() + 1) * 2, lstr);
    std::function<void(const Move&)> __setMoveZH = [&](const Move& move) {
        int firstcol{ move.CC_ColNo() * 5 }, row{ move.nextNo() * 2 };
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
    __setMoveZH(*rootMove_);
    std::wstringstream wss{};
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << moveInfo();
    os << Tools::ws2s(wss.str());
}

void MoveOwner::readBIN(std::istream& is)
{
    char len[sizeof(int)]{};
    int* length{ (int*)len };
    std::function<std::wstring()> __getRemark = [&]() {
        is.read(len, sizeof(int));
        if (*length > 0) {
            char rem[*length + 1]{};
            is.read(rem, *length);
            return Tools::s2ws(rem);
        } else
            return std::wstring{};
    };
    std::function<void(Move&)> __readMove = [&](Move& move) {
        char frowcol{}, trowcol{}, tag{};
        is.get(frowcol).get(trowcol).get(tag);

        std::cout << int(frowcol) << int(trowcol) << std::endl;

        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        if (tag & 0x08)
            move.setRemark(__getRemark());
        std::wcout << move.toString() << std::endl;

        if (tag & 0x80)
            __readMove(*move.addNext());
        if (tag & 0x40)
            __readMove(*move.addOther());
    };

    remark_ = __getRemark();
    char tag{};
    is.get(tag);
    if (tag)
        __readMove(*rootMove_);
}

void MoveOwner::writeBIN(std::ostream& os) const
{
    auto __writeRemark = [&](const std::wstring& remark) {
        std::string sremark{ Tools::ws2s(remark) };
        int len = sremark.size();
        os.write((char*)&len, sizeof(int));
        if (len > 0)
            os.write(sremark.c_str(), len);
    };
    std::function<void(const Move&)> __writeMove = [&](const Move& move) {
        const std::wstring& remark{ move.remark() };
        os.put(move.frowcol()).put(move.trowcol()).put((move.next() ? 0x80 : 0x00) | (move.other() ? 0x40 : 0x00) | (remark.empty() ? 0x00 : 0x08));
        if (!remark.empty()) {
            __writeRemark(remark);
        }
        std::wcout << move.toString() << std::endl;

        if (move.next())
            __writeMove(*move.next());
        if (move.other())
            __writeMove(*move.other());
    };

    __writeRemark(remark_); // 至少会写入0
    if (rootMove_->fseat()) {
        os.put(1);
        __writeMove(*rootMove_);
    }
}

void MoveOwner::readJSON(std::istream& is)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, is, &root, &errs))
        return;

    std::function<void(Move&, Json::Value&)> __readMove = [&](Move& move, Json::Value& item) {
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

void MoveOwner::writeJSON(std::ostream& os) const
{
    std::function<Json::Value(const Move&)> __writeItem = [&](const Move& move) {
        Json::Value item{};
        item["f"] = move.fseat()->rowcol();
        item["t"] = move.tseat()->rowcol();
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
    root["remark"] = Tools::ws2s(remark_);
    root["moves"] = __writeItem(*rootMove_);
    writer->write(root, &os);
}
}