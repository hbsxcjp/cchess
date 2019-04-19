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
    while (prev_move = this_move->prev()) {
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
    if (fseat())
        wss << fseat()->toString() << L'>' << tseat()->toString() << L'-' << (eatPie() ? eatPie()->name() : L' ')
            << iccs() << L' ' << zh() << L' ' << nextNo() << L' ' << otherNo() << L' ' << CC_ColNo() << L' ' << remark();
    return wss.str();
}

void MoveManager::read(std::istream& is, RecFormat fmt, const BoardSpace::Board& board, const InfoSpace::Key& key)
{
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
    setMoves(fmt, board);
}

void MoveManager::write(std::ostream& os, RecFormat fmt) const
{
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
void MoveManager::setMoves(RecFormat fmt, const BoardSpace::Board& board)
{
    std::function<void(Move&)> __set = [&](Move& move) {
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

    if (rootMove_)
        __set(*rootMove_); // 驱动函数
}

void MoveManager::changeSide(const BoardSpace::Board* board, std::_Mem_fn<const int (BoardSpace::Board::*)(int rowcol) const> changeRowcol)
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

const std::wstring MoveManager::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

const std::wstring MoveManager::toString() const
{
    std::ostringstream ss{};
    writePGN_CC(ss);
    return Tools::s2ws(ss.str());
}

void MoveManager::readXQF(std::istream& is, const InfoSpace::Key& key)
{
    char data[4]{};
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
        __readBytes(data, 4);
        return *(int*)data - key.KeyRMKSize;
    };
    std::function<std::wstring(char&)> __getRemark = [&](char& tag) {
        int RemarkSize = 0;
        if (key.Version_XQF <= 0x0A) {
            //char b = 0;
            //if (tag & 0xF0)
            //    b = b | 0x80;
            //if (tag & 0x0F)
            //    b = b | 0x40;
            tag = ((tag & 0xF0) ? 0x80 : 0) | ((tag & 0x0F) ? 0x40 : 0);
            RemarkSize = __getRemarksize();
        } else {
            tag = tag & 0xE0;
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
        __readBytes(data, 4);
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(data[0], 0X18 + key.KeyXYf), tcolrow = __sub(data[1], 0X20 + key.KeyXYt);

        assert(fcolrow <= 89 && tcolrow <= 89);

        move.setFrowcol((fcolrow % 10) * 10 + fcolrow / 10);
        move.setTrowcol((tcolrow % 10) * 10 + tcolrow / 10);
        move.setRemark(__getRemark(data[2]));
        //std::wcout << move.toString() << std::endl;

        if (data[2] & 0x80) //# 有左子树
            __readMove(*move.addNext());
        if (data[2] & 0x40) // # 有右子树
            __readMove(*move.addOther());
    };

    is.seekg(1024);
    __readBytes(data, 4);
    remark_ = __getRemark(data[2]);
    if (data[2] & 0x80) //# 有左子树
        __readMove(*rootMove_);
}

void MoveManager::writeXQF(std::ostream& os) const {}

const std::wstring MoveManager::__getMoveStr(std::istream& is) const
{
    std::stringstream ss{};
    std::string line{};
    is >> std::noskipws;
    while (std::getline(is, line)) // 以空行为分割，接info read之后
        ss << line << '\n';
    return Tools::s2ws(ss.str());
}

void MoveManager::readPGN_ICCSZH(std::istream& is, RecFormat fmt)
{
    const std::wstring moveStr{ __getMoveStr(is) };
    std::wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    std::wstring mvStr{ fmt == RecFormat::PGN_ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                                 : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    std::wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    std::wregex moveReg{ preStr + mvStr + lastStr };

    auto __readMove = [&](std::shared_ptr<Move> move, const std::wstring mvstr, bool isOther) { //# 非递归
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
    std::vector<std::shared_ptr<Move>> othMoves{ rootMove_ };
    std::wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    std::wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    std::wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        remark_ = wsm.str(1);
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {
        //std::wcout << *wtleft << L"\n---------------------------------------------\n" << std::endl;
        std::wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            //std::wcout << *wtright << L"\n---------------------------------------------\n" << std::endl;
            move = __readMove(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(move);
        isOther = true;
    }
}

void MoveManager::writePGN_ICCSZH(std::ostream& os, RecFormat fmt) const
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
    if (rootMove_)
        __writeMove(*rootMove_, false);
    os << Tools::ws2s(wss.str());
}

void MoveManager::readPGN_CC(std::istream& is)
{
    const std::wstring moveStr{ __getMoveStr(is) };
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

    auto __getRemark = [&](int row, int col) {
        return remm[std::to_wstring(row) + L',' + std::to_wstring(col)];
    };
    std::function<void(Move&, int, int, bool)> __readMove = [&](Move& move, int row, int col, bool isOther) {
        std::wstring zh{ movv[row][col] };
        if (regex_match(zh, movefat)) {
            auto newMove = isOther ? move.addOther() : move.addNext();
            newMove->setZh(zh.substr(0, 4));
            newMove->setRemark(__getRemark(row, col));
            if (zh.back() == L'…')
                __readMove(*newMove, row, col + 1, true);
            if (int(movv.size()) - 1 > row)
                __readMove(*newMove, row + 1, col, false);
        } else if (isOther) {
            while (movv[row][col][0] == L'…')
                ++col;
            __readMove(move, row, col, true);
        }
    };

    remark_ = __getRemark(0, 0);
    if (!movv.empty())
        __readMove(*rootMove_, 1, 0, false);
}

void MoveManager::writePGN_CC(std::ostream& os) const
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

    __setMoveZH(*rootMove_);
    std::wstringstream wss{};
    lineStr.front().replace(0, 3, L"　开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str() << moveInfo();
    os << Tools::ws2s(wss.str());
}

void MoveManager::readBIN(std::istream& is)
{
    auto __getRemark = [&]() {
        char len[sizeof(int)]{};
        is.read(len, sizeof(int));
        int length{ *(int*)len };
        char rem[length + 1]{};
        is.read(rem, length);
        return Tools::s2ws(rem);
    };
    std::function<void(Move&)> __readMove = [&](Move& move) {
        char frowcol{}, trowcol{}, tag{};
        is.get(frowcol).get(trowcol).get(tag);
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        if (tag & 0x08)
            move.setRemark(__getRemark());

        if (tag & 0x80)
            __readMove(*move.addNext());
        if (tag & 0x40)
            __readMove(*move.addOther());
    };

    remark_ = __getRemark();
    __readMove(*rootMove_);
}

void MoveManager::writeBIN(std::ostream& os) const
{
    auto __writeRemark = [&](std::wstring remark) {
        int len = remark.size();
        os.write((char*)&len, sizeof(int)).write(Tools::ws2s(remark).c_str(), len);
    };
    std::function<void(const Move&)> __writeMove = [&](const Move& move) {
        std::wstring remark{ move.remark() };
        os.put(static_cast<char>(move.fseat()->rowcol()))
            .put(static_cast<char>(move.tseat()->rowcol()))
            .put(static_cast<char>((move.next() ? 0x80 : 0x00) | (move.other() ? 0x40 : 0x00) | (remark.empty() ? 0x00 : 0x08)));
        if (!remark.empty()) {
            __writeRemark(remark);
        }
        if (move.next())
            __writeMove(*move.next());
        if (move.other())
            __writeMove(*move.other());
    };

    __writeRemark(remark_); // 至少会写入0
    __writeMove(*rootMove_);
}

void MoveManager::readJSON(std::istream& is)
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
        //move.setSeats(board.getSeat(frowcol), board.getSeat(trowcol));
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

void MoveManager::writeJSON(std::ostream& os) const
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