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
    if (fseat())
        wss << fseat()->toString() << L'>' << tseat()->toString() << L'-' << (eatPie() ? eatPie()->name() : L' ')
            << iccs() << L' ' << zh() << L' ' << nextNo() << L' ' << otherNo() << L' ' << CC_ColNo() << L' ' << remark();
    return wss.str();
}

void RootMove::read(std::istream& is, RecFormat fmt, const BoardSpace::Board& board, const InfoSpace::Key& key)
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

void RootMove::write(std::ostream& os, RecFormat fmt) const
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
    //std::wcout << "setMoves!" << std::endl;
}

const std::wstring RootMove::moveInfo() const
{
    std::wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

void RootMove::readXQF(std::istream& is, const InfoSpace::Key& key)
{
    char data[4]{};
    std::function<unsigned char(unsigned char, unsigned char)> __sub = [](unsigned char a, unsigned char b) {
        return a - b;
    }; // 保持为<256
    auto __readbytes = [&](char* bytes, int size) {
        int pos = is.tellg();
        is.read(bytes, size);
        if (key.Version_XQF > 10) // '字节解密'
            for (int i = 0; i != size; ++i)
                bytes[i] = __sub(bytes[i], key.F32Keys[(pos + i) % 32]);
    };
    auto __readremarksize = [&]() {
        __readbytes(data, 4);
        return *(int*)(unsigned char*)data - key.KeyRMKSize;
    };
    std::function<void(Move&)> __read = [&](Move& move) {
        __readbytes(data, 4);
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        int fcolrow = __sub(data[0], 0X18 + key.KeyXYf), tcolrow = __sub(data[1], 0X20 + key.KeyXYt);
        //if (fcolrow <= 89 && tcolrow <= 89) { // col<=8, row<=9
        move.setFrowcol((fcolrow % 10) * 10 + fcolrow / 10);
        move.setTrowcol((tcolrow % 10) * 10 + tcolrow / 10);
        //}
        //std::wcout << move.toString() << std::endl;

        char ChildTag = data[2];
        int RemarkSize = 0;
        if (key.Version_XQF <= 0x0A) {
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
            move.setRemark(Tools::s2ws(rem));

            //std::wcout << move.remark() << std::endl;
        }

        if (ChildTag & 0x80) //# 有左子树
            __read(*move.addNext());
        if (ChildTag & 0x40) // # 有右子树
            __read(*move.addOther());
    };

    is.seekg(1024);
    __read(*this);
}

void RootMove::writeXQF(std::ostream& os) const {}

const std::wstring RootMove::getMoveStr(std::istream& is) const
{
    std::stringstream ss{};
    std::string line{};
    is >> std::noskipws;
    while (is >> line) // 以空行为分割，接info read之后
        ss << line;
    return Tools::s2ws(ss.str());
}

void RootMove::readPGN_ICCSZH(std::istream& is, RecFormat fmt)
{
    const std::wstring moveStr{ getMoveStr(is) };
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

void RootMove::writePGN_ICCSZH(std::ostream& os, RecFormat fmt) const
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
    os << Tools::ws2s(wss.str());
}

void RootMove::readPGN_CC(std::istream& is)
{
    const std::wstring moveStr{ getMoveStr(is) };
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

void RootMove::writePGN_CC(std::ostream& os) const
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
    lineStr.front().replace(0, 3, L"　开始");
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remStrs.str(); // << moveInfo();
    os << Tools::ws2s(wss.str());
}

void RootMove::readBIN(std::istream& is)
{
    std::function<void(Move&)> __read = [&](Move& move) {
        char frowcol{}, trowcol{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        is.get(frowcol).get(trowcol).get(tag);
        move.setFrowcol(frowcol);
        move.setTrowcol(trowcol);
        //move.setSeats(board.getSeat(frowcol), board.getSeat(trowcol));
        hasNext = tag & 0x80;
        hasOther = tag & 0x40;
        hasRemark = tag & 0x08;
        if (hasRemark) {
            char len[sizeof(int)]{};
            is.read(len, sizeof(int));
            int length{ *(int*)len };

            char rem[length + 1]{};
            is.read(rem, length);
            move.setRemark(Tools::s2ws(rem));
        }
        if (hasNext)
            __read(*move.addNext());
        if (hasOther)
            __read(*move.addOther());
    };

    __read(*this);
}

void RootMove::writeBIN(std::ostream& os) const
{
    std::function<void(const Move&)> __write = [&](const Move& move) {
        std::string remark{ Tools::ws2s(move.remark()) };
        os.put(static_cast<char>(move.fseat()->rowcolValue()))
            .put(static_cast<char>(move.tseat()->rowcolValue()))
            .put(static_cast<char>((move.next() ? 0x80 : 0x00) | (move.other() ? 0x40 : 0x00) | (remark.empty() ? 0x00 : 0x08)));
        if (!remark.empty()) {
            int len = remark.size();
            os.write((char*)&len, sizeof(int)).write(remark.c_str(), len);
        }
        if (move.next())
            __write(*move.next());
        if (move.other())
            __write(*move.other());
    };

    __write(*this);
}

void RootMove::readJSON(std::istream& is)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, is, &root, &errs))
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

void RootMove::writeJSON(std::ostream& os) const
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
    writer->write(root, &os);
}
}