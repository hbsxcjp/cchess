#include "chessInstance.h"
#include "json.h"

#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
//#include <sstream>
//#include <locale>
#include <regex>
//#include <codecvt>
//#include <utility>

using namespace std;
using namespace Board_base;
using namespace Json;

// ChessInstance
ChessInstance::ChessInstance()
{
    rootMove = make_shared<Move>();
    currentMove = rootMove;
    firstColor = PieceColor::red; // 棋局载入时需要设置此属性！
}

ChessInstance::ChessInstance(string filename)
    : ChessInstance()
{
    string ext{ getExt(filename) };
    if (ext == ".pgn") {
        wstring pgnTxt{ readTxt(filename) }, infoTxt{}, movesTxt{};
        auto pos = pgnTxt.find(L"\n1. ");
        infoTxt = pgnTxt.substr(0, pos);
        movesTxt = pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"";
        info = Info(infoTxt);
        board = Board(info);
        __init(movesTxt, info);
    } else if (ext == ".xqf") {
        vector<int> Keys(4, 0);
        vector<int> F32Keys(32, 0);
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs, Keys, F32Keys);
        board = Board(info);
        __init(ifs, Keys, F32Keys);
    } else if (ext == ".bin") {
        ifstream ifs(filename, ios_base::binary);
        info = Info(ifs);
        board = Board(info);
        __init(ifs);
    } else if (ext == ".json") {
        ifstream ifs(filename);
        Json::CharReaderBuilder builder;
        Json::Value root;
        JSONCPP_STRING errs;
        if (parseFromStream(builder, ifs, &root, &errs)) {
            info = Info(root);
            board = Board(info);
            __init(root);
        }
    }
}

void ChessInstance::write(string fname, string ext, RecFormat fmt)
{
    string filename{ fname + ext };
    if (ext == ".pgn") {
        writeTxt(to_string(static_cast<int>(fmt)) + "_" + filename,
            info.toString(fmt) + L"\n" + toString(fmt));
    } else if (ext == ".bin") {
        ofstream ofs(filename, ios_base::binary);
        info.toBin(ofs);
        toBin(ofs);
    } else if (ext == ".json") {
        ofstream ofs(filename);
        Json::Value root;
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        info.toJson(root);
        toJson(root);
        writer->write(root, &ofs);
    }
}

void ChessInstance::__init(istream& is, vector<int>& Keys, vector<int>& F32Keys)
{
    fromXQF(is, Keys, F32Keys);
    __initSet(RecFormat::XQF);
}

void ChessInstance::__init(wstring moveStr, Info& info)
{
    RecFormat fmt{ info.getRecFormat() };
    switch (fmt) {
    case RecFormat::ICCS:
    case RecFormat::ZH:
        fromICCSZH(moveStr, fmt);
        break;
    default:
        fromCC(moveStr);
        break;
    }
    __initSet(fmt);
}

void ChessInstance::__init(istream& is)
{
    fromBIN(is);
    __initSet(RecFormat::BIN);
}

void ChessInstance::__init(Json::Value& root)
{
    fromJson(root["moves"]);
    __initSet(RecFormat::JSON);
}

inline PieceColor ChessInstance::currentColor()
{
    return currentMove->stepNo % 2 == 0
        ? firstColor
        : (firstColor == PieceColor::red ? PieceColor::black
                                         : PieceColor::red);
}

inline vector<shared_ptr<Move>> ChessInstance::getPrevMoves(shared_ptr<Move> move)
{
    vector<shared_ptr<Move>> res{ move };
    while (!move->prev())
        res.push_back(move->prev());
    std::reverse(res.begin(), res.end());
    return res;
}

// 基本走法
void ChessInstance::forward()
{
    if (currentMove->next()) {
        currentMove = currentMove->next();
        board.go(*currentMove);
    }
}

void ChessInstance::backward()
{
    if (currentMove->prev()) {
        board.back(*currentMove);
        currentMove = currentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void ChessInstance::forwardOther()
{
    if (currentMove->other()) {
        shared_ptr<Move> toMove = currentMove->other();
        board.back(*currentMove);
        board.go(*toMove);
        currentMove = toMove;
    }
}

// 复合走法
void ChessInstance::backwardTo(shared_ptr<Move> move)
{
    while (move != currentMove) {
        board.back(*move);
        move = move->prev();
    }
}

void ChessInstance::to(shared_ptr<Move> move)
{
    if (move == currentMove)
        return;
    toFirst();
    for (auto& m : getPrevMoves(move))
        board.go(*m);
    currentMove = move;
}

inline void ChessInstance::toFirst()
{
    while (!isStart())
        backward();
}

inline void ChessInstance::toLast()
{
    while (!isLast())
        forward();
}

inline void ChessInstance::go(int inc = 1)
{
    if (inc > 0)
        for (int i = 0; i != inc; ++i)
            forward();
    else
        for (int i = inc; i != 0; ++i)
            backward();
}

inline void ChessInstance::cutNext() { currentMove->setNext(nullptr); }

inline void ChessInstance::cutOther()
{
    if (currentMove->other())
        currentMove->setOther(currentMove->other()->other());
}

const wstring ChessInstance::getICCS(int fseat, int tseat)
{
    wstringstream wss{};
    wss << ColChars[getCol(fseat)] << getRow(fseat) << ColChars[getCol(tseat)] << getRow(tseat);
    return wss.str();
}

const pair<int, int> ChessInstance::getSeat__ICCS(wstring ICCS)
{
    string iccs{ ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97),
        getSeat(iccs[3] - 48, iccs[2] - 97));
}

//(fseat, tseat)->中文纵线着法
const wstring ChessInstance::getZh(int fseat, int tseat)
{
    wstringstream wss{};
    int fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    Piece* fromPiece{ board.getPiece(fseat) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    bool isBottomSide{ board.isBottomSide(color) };
    vector<int> seats{ board.getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };
    auto __getChar = [&](int col) {
        return getNumChar(color, isBottomSide ? MaxCol - col : col);
    };

    if (length > 1 && find_char(Pieces::strongeNames, name)) {
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide,
                board.getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[find_index(seats, fseat)] << name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << __getChar(fromCol);

    int toRow{ getRow(tseat) };
    //wcout << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退')) << endl;

    wss << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退'))
        << (find_char(Pieces::lineNames, name) && toRow != fromRow
                   ? getNumChar(color, toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1)
                   : __getChar(getCol(tseat)));
    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const pair<int, int> ChessInstance::getSeat__Zh(wstring zhStr)
{
    int index, fseat, tseat;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ find_char(getNumChars(PieceColor::red), zhStr.back())
            ? PieceColor::red
            : PieceColor::black };
    bool isBottomSide = board.isBottomSide(color);
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](wchar_t ch) {
        return static_cast<int>(getNumChars(color).find(ch)) + 1;
    };
    auto __getCol = [&](int num) { return isBottomSide ? ColNum - num : num - 1; };
    if (find_char(Pieces::allNames, name)) {
        seats = board.getSideNameColSeats(color, name, __getCol(__getNum(zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << board.toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && find_char(Pieces::advbisNames, name)
                    && (zhStr[2] == L'退') == isBottomSide)
            ? seats.size() - 1
            : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = getIndex(zhStr[0]);
        name = zhStr[1];
        seats = board.getSideNameSeats(color, name);

        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << board.toString();

        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide, seats);
            //#获取多兵的列
            if (seats.size() == 3 && zhStr[0] == L'后')
                index += 1;
        } else {
            if (isBottomSide) //# 修正index
                index = seats.size() - index - 1;
        }
    }
    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ getIndex(zhStr[2]) * (isBottomSide ? 1 : -1) },
        num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    if (find_char(Pieces::lineNames, name)) {
        //#'获取直线走子toseat'
        int row = getRow(fseat);
        tseat = (movDir == 0) ? getSeat(row, toCol)
                              : (getSeat(row + movDir * num, getCol(fseat)));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step = toCol - getCol(fseat); //  # 相距1或2列
        if (step < 0)
            step *= -1;
        int inc = find_char(Pieces::advbisNames, name)
            ? step
            : (step == 1 ? 2 : 1);
        tseat = getSeat(getRow(fseat) + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

void ChessInstance::fromICCSZH(wstring moveStr, RecFormat fmt)
{
    wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    wstring mvStr{ fmt == RecFormat::ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                        : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](shared_ptr<Move> move, wstring mvstr, bool isOther) { //# 非递归
        vector<pair<wstring, wstring>> mrStrs{};
        for (wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != wsregex_iterator{}; ++p)
            mrStrs.push_back(make_pair((*p)[1], (*p)[2]));
        for (auto mr : mrStrs) {
            auto newMove = make_shared<Move>();
            if (fmt == RecFormat::ZH)
                newMove->zh = mr.first;
            else
                newMove->ICCS = mr.first;

            //wcout << mr.first << endl;

            if (mr.second.size() > 0)
                //wcout << mr.second << endl;
                newMove->remark = mr.second;

            if (isOther) { // # 第一步为变着
                move->setOther(newMove);
                isOther = false;
            } else
                move->setNext(newMove);
            move = newMove;
        }
        return move;
    };

    shared_ptr<Move> move;
    vector<shared_ptr<Move>> othMoves{ rootMove };
    wregex rempat{ LR"(\{([\s\S]*?)\}\s+1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    wsmatch res;
    if (regex_search((*wtleft).first, (*wtleft).second, res, rempat))
        rootMove->remark = res.str(1);
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {

        //wcout << *wtleft << L"\n---------------------------------------------\n"
        //      << endl;

        wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            //wcout << *wtright << L"\n---------------------------------------------\n" << endl;
            move = setMoves(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(move);
        isOther = true;
    }
    //wcout << L"fromICCSZh OK!" << endl;
}

void ChessInstance::fromCC(wstring fullMoveStr)
{
    auto pos = fullMoveStr.find(L"\n(");
    wstring moveStr{ fullMoveStr.substr(0, pos) }, remStr{ pos < fullMoveStr.size() ? fullMoveStr.substr(pos) : L"" };
    wregex spfat{ LR"(\n)" }, mstrfat{ LR"(.{5})" },
        movefat{ LR"(([^…　]{4}[…　]))" }, remfat{ LR"(\(\s*(\d+,\d+)\): \{([\s\S]*?)\})" };
    int row{ 0 };
    vector<vector<wstring>> movv{};
    wsregex_token_iterator msp{ moveStr.begin(), moveStr.end(), spfat, -1 }, end{};
    for (; msp != end; ++msp)
        if (++row % 2 == 0) {
            vector<wstring> linev{};
            for (wsregex_token_iterator mp{ (*msp).first, (*msp).second, mstrfat, 0 }; mp != end; ++mp)
                linev.push_back(*mp);
            movv.push_back(linev);
        }
    map<wstring, wstring> remm{};
    if (remStr.size() > 0)
        for (wsregex_iterator rp{ remStr.begin(), remStr.end(), remfat }; rp != wsregex_iterator{}; ++rp)
            remm[(*rp)[1]] = (*rp)[2];

    auto __setRem = [&](Move& move, int row, int col) {
        wstring key{ to_wstring(row) + L',' + to_wstring(col) };
        if (remm.find(key) != remm.end())
            move.remark = remm[key];
    };
    function<void(Move&, int, int, bool)> __read = [&](Move& move, int row, int col, bool isOther) {
        wstring zhStr{ movv[row][col] };
        if (regex_match(zhStr, movefat)) {
            auto newMove = make_shared<Move>();
            newMove->zh = zhStr.substr(0, 4);
            __setRem(*newMove, row, col);
            if (isOther)
                move.setOther(newMove);
            else
                move.setNext(newMove);
            if (zhStr.back() == L'…')
                __read(*newMove, row, col + 1, true);
            if (int(movv.size()) - 1 > row)
                __read(*newMove, row + 1, col, false);
        } else if (isOther) {
            while (movv[row][col][0] == L'…')
                ++col;
            __read(move, row, col, true);
        }
    };

    __setRem(*rootMove, 0, 0);
    if (int(movv.size()) > 0)
        __read(*rootMove, 1, 0, false);
}

void ChessInstance::fromBIN(istream& is)
{
    function<void(Move&)> __read = [&](Move& move) {
        char fseat{}, tseat{}, hasNext{}, hasOther{};
        is.get(fseat).get(tseat).get(hasNext).get(hasOther);
        move.setSeat(fseat, tseat);

        char len[sizeof(int)]{};
        is.read(len, sizeof(int));
        int length{ *(int*)len };
        if (length > 0) {
            char rem[length + 1]{};
            is.read(rem, length);
            move.remark = s2ws(rem);
        }

        if (hasNext) { //# 有左子树
            move.setNext(make_shared<Move>());
            __read(*move.next());
        }
        if (hasOther) { // # 有右子树
            move.setOther(make_shared<Move>());
            __read(*move.other());
        }
    };

    __read(*rootMove);
}

void ChessInstance::fromJson(Json::Value& rootItem)
{
    function<void(Move&, Json::Value&)> __read = [&](Move& move, Json::Value& item) {
        int fseat{ item["f"].asInt() }, tseat{ item["t"].asInt() };
        move.setSeat(fseat, tseat);
        if (item.isMember("r"))
            move.remark = s2ws(item["r"].asString());

        if (item.isMember("n")) { //# 有左子树
            move.setNext(make_shared<Move>());
            __read(*move.next(), item["n"]);
        }
        if (item.isMember("o")) { // # 有右子树
            move.setOther(make_shared<Move>());
            __read(*move.other(), item["o"]);
        }
    };

    if (!rootItem.isNull())
        __read(*rootMove, rootItem);
}

void ChessInstance::fromXQF(istream& is, vector<int>& Keys, vector<int>& F32Keys)
{
    int version{ Keys[0] }, KeyXYf{ Keys[1] }, KeyXYt{ Keys[2] }, KeyRMKSize{ Keys[3] };
    //wcout << L"ver_XYf_XYt_Size: " << version << L'/'
    //      << KeyXYf << L'/' << KeyXYt << L'/' << KeyRMKSize << endl;

    function<void(Move&)> __read = [&](Move& move) {
        auto __byteToSeat = [&](int a, int b) {
            int xy = __subbyte(a, b);
            return getSeat(xy % 10, xy / 10);
        };
        auto __readbytes = [&](char* byteStr, int size) {
            int pos = is.tellg();
            is.read(byteStr, size);
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
        //auto seats = make_pair(__byteToSeat(data[0], 0X18 + KeyXYf), __byteToSeat(data[1], 0X20 + KeyXYt));
        //move.setSeat(seats);
        move.setSeat(__byteToSeat(data[0], 0X18 + KeyXYf), __byteToSeat(data[1], 0X20 + KeyXYt));
        //wcout << L"fs_ts: " << move.fseat() << L'/' << move.tseat() << endl;

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
            move.remark = s2ws(rem);
            //wcout << L"remark: " << move.remark << L'/' << endl;
        }

        if (ChildTag & 0x80) { //# 有左子树
            move.setNext(make_shared<Move>());
            __read(*move.next());
        }
        if (ChildTag & 0x40) { // # 有右子树
            move.setOther(make_shared<Move>());
            __read(*move.other());
        }
    };

    //is.seekg(1024);
    __read(*rootMove);
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void ChessInstance::__initSet(RecFormat fmt)
{
    auto __setRem = [&](Move& move) {
        int length = move.remark.size();
        if (length > 0) {
            remCount += 1;
            if (length > remLenMax)
                remLenMax = length;
        }
    };

    function<void(Move&)> __set = [&](Move& move) {
        switch (fmt) {
        case RecFormat::ICCS: {
            move.setSeat(getSeat__ICCS(move.ICCS));
            move.zh = getZh(move.fseat(), move.tseat());
            break;
        }
        case RecFormat::ZH:
        case RecFormat::CC: {
            move.setSeat(getSeat__Zh(move.zh));
            move.ICCS = getICCS(move.fseat(), move.tseat());
            //*
            wstring zh{ getZh(move.fseat(), move.tseat()) };
            // wcout << move.toString_zh() << L'\n';
            if (move.zh != zh) {
                wcout << L"move.zh: " << move.zh << L'\n'
                      << L"getZh( ): " << zh << L'\n'
                      << move.toString() << L'\n' << board.toString() << endl;
                return;
            } //*/
            break;
        }
        case RecFormat::XQF:
        case RecFormat::BIN:
        case RecFormat::JSON: {
            move.ICCS = getICCS(move.fseat(), move.tseat());
            move.zh = getZh(move.fseat(), move.tseat());
            /*
            auto seats = getSeat__Zh(move.zh);
            // wcout << move.toString() << L'\n';
            if ((seats.first != move.fseat()) || (seats.second != move.tseat())) {
                wcout << L"move.fs_ts: " << move.fseat() << L' ' << move.tseat() << L'\n'
                      << L"getSeat__Zh( ): " << move.zh << L'\n'
                      << move.toString() << L'\n' << board.toString() << endl;
                return;
            } //*/
            break;
        }
        default:
            break;
        }

        movCount += 1;
        __setRem(move);
        move.maxCol = maxCol; // # 本着在视图中的列数
        if (move.othCol > othCol)
            othCol = move.othCol;
        if (move.stepNo > maxRow)
            maxRow = move.stepNo;
        board.go(move);

        //wcout << move.toString() << L"\n"
        //      << board.toString() << endl;

        if (move.next())
            __set(*move.next());
        board.back(move);
        if (move.other()) {
            maxCol += 1;
            __set(*move.other());
        }
    };

    __setRem(*rootMove);
    if (rootMove->next())
        __set(*rootMove->next()); // 驱动函数
    //toFirst(); // 复原board
}

void ChessInstance::toBin(ostream& os)
{
    function<void(Move&)> __write = [&](Move& move) {
        os.put(char(move.fseat())).put(char(move.tseat()));
        os.put(char(move.next() ? true : false)).put(char(move.other() ? true : false));

        string remark{ ws2s(move.remark) };
        int len{ int(remark.size()) };
        os.write((char*)&len, sizeof(int));
        if (len > 0)
            os.write(remark.c_str(), len);

        if (move.next())
            __write(*move.next());
        if (move.other())
            __write(*move.other());
    };

    __write(*rootMove);
}

void ChessInstance::toJson(Json::Value& root)
{
    function<void(Move&, Json::Value&)> __write = [&](Move& move, Json::Value& item) {
        item["f"] = move.fseat();
        item["t"] = move.tseat();
        if (move.remark.size() > 0)
            item["r"] = ws2s(move.remark);
        if (move.next()) {
            Json::Value newItem{};
            __write(*move.next(), newItem);
            item["n"] = newItem;
        }
        if (move.other()) {
            Json::Value newItem{};
            __write(*move.other(), newItem);
            item["o"] = newItem;
        }
    };

    Json::Value rootItem;
    __write(*rootMove, rootItem);
    root["moves"] = rootItem;
}

wstring ChessInstance::toString(RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::ZH:
    case RecFormat::ICCS:
        return toString_ICCSZH(fmt);
    case RecFormat::CC:
        return toString_CC();
    default:
        return toString_CC();
    }
}

wstring ChessInstance::toString_ICCSZH(RecFormat fmt)
{
    wstringstream wss{};
    function<void(Move&)> __remark = [&](Move& move) {
        if (move.remark.size() > 0)
            wss << L"\n{" << move.remark << L"}\n";
    };
    __remark(*rootMove);

    function<void(Move&, bool)> __moveStr = [&](Move& move, bool isOther) {
        int boutNum = (move.stepNo + 1) / 2;
        bool isEven = move.stepNo % 2 == 0;
        if (isOther)
            wss << L"(" << boutNum << L". " << (isEven ? L"... " : L"");
        else if (isEven)
            wss << L" ";
        else
            wss << boutNum << L". ";
        wss << (fmt == RecFormat::ZH ? move.zh : move.ICCS) << L' ';
        __remark(move);
        if (move.other()) {
            __moveStr(*move.other(), true);
            wss << L") ";
        }
        if (move.next())
            __moveStr(*move.next(), false);
    };

    // 驱动调用函数
    if (rootMove->next())
        __moveStr(*rootMove->next(), false);
    return wss.str();
}

wstring ChessInstance::toString_CC()
{
    wstringstream remstrs{};
    wstring lstr((maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((maxRow + 1) * 2, lstr);
    function<void(Move&)> __setChar = [&](Move& move) {
        int firstcol = move.maxCol * 5;
        for (int i = 0; i < 4; ++i)
            lineStr[move.stepNo * 2][firstcol + i] = move.zh[i];
        if (move.remark.size())
            remstrs << L"(" << move.stepNo << L"," << move.maxCol << L"): {"
                    << move.remark << L"}\n";
        if (move.next()) {
            lineStr[move.stepNo * 2 + 1][firstcol + 2] = L'↓';
            __setChar(*move.next());
        }
        if (move.other()) {
            int linel = move.other()->maxCol * 5;
            for (int i = firstcol + 4; i < linel; ++i)
                lineStr[move.stepNo * 2][i] = L'…';
            __setChar(*move.other());
        }
    };
    __setChar(*rootMove);

    wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 变着广度：" << othCol
        << L", 视图宽度：" << maxCol << L", \n　着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    lineStr[0][0] = L'1';
    lineStr[0][1] = L'.';
    lineStr[0][2] = L' ';
    lineStr[0][3] = L'开';
    lineStr[0][4] = L'始';
    for (auto line : lineStr)
        wss << line << L'\n';
    return wss.str() + remstrs.str();
}

void ChessInstance::transDir(string dirfrom, string ext, RecFormat fmt)
{
    string extensions{ ".xqf.pgn.bin.json" };
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + ext };
    string ext_old{};
    function<void(string, string)> __trans = [&](string dirfrom, string dirto) {
        long hFile = 0; //文件句柄
        struct _finddata_t fileinfo; //文件信息
        if (access(dirto.c_str(), 0) != 0)
            mkdir(dirto.c_str());
        if ((hFile = _findfirst((dirfrom + "/*").c_str(), &fileinfo)) != -1) {
            do {
                string fname{ fileinfo.name };
                if (fileinfo.attrib & _A_SUBDIR) { //如果是目录,迭代之
                    if (fname != "." && fname != "..") {
                        dcount += 1;
                        __trans(dirfrom + "/" + fname, dirto + "/" + fname);
                    }
                } else { //如果是文件,执行转换
                    string filename{ dirfrom + "/" + fname };
                    string fileto{ dirto + "/" + fname.substr(0, fname.rfind('.')) };
                    ext_old = getExt(fname);
                    if (extensions.find(ext_old) != string::npos) {
                        fcount += 1;
                        //cout << filename << endl;

                        ChessInstance ci(filename);
                        ci.write(fileto, ext, fmt);
                        movcount += ci.movCount;
                        remcount += ci.remCount;
                        if (ci.remLenMax > remlenmax)
                            remlenmax = ci.remLenMax;
                    } else
                        copyFile(filename.c_str(), (fileto + ext_old).c_str());
                }
            } while (_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    };

    __trans(dirfrom, dirto);
    cout << dirfrom + " =>" << static_cast<int>(fmt) << ext << ": 转换" << fcount << "个文件, "
         << dcount << "个目录成功！\n   着法数量: "
         << movcount << ", 注释数量: " << remcount << ", 最大注释长度: " << remlenmax << endl;
}

void ChessInstance::testTransDir(int fd, int td, int ff, int ft, int tf, int tt)
{
    vector<string> dirfroms{
        "c:\\棋谱\\示例文件",
        "c:\\棋谱\\象棋杀着大全",
        "c:\\棋谱\\疑难文件",
        "c:\\棋谱\\中国象棋棋谱大全"
    };
    vector<string> exts{ ".xqf", ".pgn", ".bin", ".json" };

    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex) {
                string dirName{ dirfroms[dir] + exts[fIndex] };
                if (tIndex != fIndex) {
                    switch (tIndex) {
                    case 1:
                        transDir(dirName, ".pgn", RecFormat::ICCS);
                        transDir(dirName, ".pgn", RecFormat::ZH);
                        transDir(dirName, ".pgn", RecFormat::CC);
                        break;
                    case 2:
                        transDir(dirName, ".bin", RecFormat::BIN);
                        break;
                    case 3:
                        transDir(dirName, ".json", RecFormat::JSON);
                        break;
                    default:
                        break;
                    }
                }// else if (tIndex == 1) {
                    //transDir(dirName, ".pgn", RecFormat::CC);
                //}
            }
}
