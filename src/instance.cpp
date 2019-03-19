#include "Instance.h"
#include "../json/json.h"
#include "board.h"
#include "move.h"
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
using namespace std;
using namespace Json;

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
    , board_{ make_shared<Board>() }
    , rootMove_{ make_shared<Move>() }
    , currentMove_{ rootMove_ }
    , firstColor_{ PieceColor::RED }
{
}

Instance::Instance(const string& filename)
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
    setBoard();
    setMoves(fmt);
}

void Instance::write(const string& fname, const RecFormat fmt)
{
    string filename{ fname + getExtName(fmt) };
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

const PieceColor Instance::currentColor() const
{
    return currentMove_->getStepNo() % 2 == 0 ? firstColor_ : Piece::getOthColor(firstColor_);
}

const bool Instance::isStart() const { return currentMove_->prev() == nullptr; }

const bool Instance::isLast() const { return currentMove_->next() == nullptr; }

// 基本走法
void Instance::forward()
{
    if (!isLast()) {
        currentMove_ = currentMove_->next();
        currentMove_->done();
        //board_->go(*currentMove_);
    }
}

void Instance::backward()
{
    if (!isStart()) {
        currentMove_->undo();
        //board_->back(*currentMove_);
        currentMove_ = currentMove_->prev();
    }
}

//'移动到当前节点的另一变着'
void Instance::forwardOther()
{
    if (currentMove_->other()) {
        currentMove_->undo();
        currentMove_ = currentMove_->other();
        currentMove_->done();
        //auto toMove = currentMove_->other();
        //board_->back(*currentMove_);
        //board_->go(*toMove);
        //currentMove_ = toMove;
    }
}

// 复合走法
void Instance::backwardTo(shared_ptr<Move> move)
{
    while (!isStart() && move != currentMove_) {
        backward();
        move = move->prev();
    }
}

void Instance::to(shared_ptr<Move> move)
{
    if (move == currentMove_)
        return;
    toFirst();
    for (auto& pmv : move->getPrevMoves())
        pmv->done();
    //board_->go(*pmv);
    currentMove_ = move;
}

void Instance::toFirst()
{
    while (!isStart())
        backward();
}

void Instance::toLast()
{
    while (!isLast())
        forward();
}

void Instance::go(const int inc)
{
    function<void(Instance*)> fbward = inc > 0 ? &Instance::forward : &Instance::backward;
    //auto fbward = mem_fn(inc > 0 ? &Instance::forward : &Instance::backward);
    for (int i = abs(inc); i != 0; --i)
        fbward(this);
}

void Instance::cutNext() { currentMove_->setNext(nullptr); }

void Instance::cutOther()
{
    if (currentMove_->other())
        currentMove_->setOther(currentMove_->other()->other());
}

void Instance::changeSide(const ChangeType ct) // 未测试
{
    auto curmove = currentMove_;
    toFirst();
    setFEN(board_->changeSide(ct));

    if (ct == ChangeType::EXCHANGE)
        firstColor_ = Piece::getOthColor(firstColor_);
    else {
        function<void(Move&)> __setSeat = [&](Move& move) {
            move.setSeats(board_->getOthSeat(move.fseat(), ct), board_->getOthSeat(move.tseat(), ct));
            if (move.next())
                __setSeat(*move.next());
            if (move.other())
                __setSeat(*move.other());
        };
        if (rootMove_->next())
            __setSeat(*rootMove_->next()); // 驱动调用递归函数
    }

    if (ct != ChangeType::ROTATE)
        setMoves(RecFormat::BIN); //借用RecFormat::BIN
    to(curmove);
}

void Instance::readXQF(const string& filename)
{
    ifstream ifs(filename, ios_base::binary);
    auto __subbyte = [](const int a, const int b) { return (256 + a - b) % 256; };
    function<unsigned char(unsigned char, unsigned char)> __calkey = [](unsigned char bKey, unsigned char cKey) {
        return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey % 256; // 保持为<256
    };
    wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
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
    info_[L"Version_xqf"] = to_wstring(version);
    info_[L"Result"] = (map<char, wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult[0]];
    info_[L"PlayType"] = (map<char, wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]];
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
        wcout << Tools::s2ws(Signature) << L" 文件标记不对。Signature != (0x58, 0x51)\n";
    if ((head_KeysSum + head_KeyXY + head_KeyXYf + head_KeyXYt) % 256 != 0)
        wcout << head_KeysSum << head_KeyXY << head_KeyXYf << head_KeyXYt << L" 检查密码校验和不对，不等于0。\n";
    if (version > 18)
        wcout << version << L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

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
            vector<unsigned char> Qixy(begin(headQiziXY), end(headQiziXY)); // head_QiziXY 不是数组，不能用
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
    string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    vector<int> F32Keys(32, 0);
    for (int i = 0; i != 32; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)
    wstring pieceChars(90, L'_');
    for (int i = 0; i != 32; ++i) {
        int xy = head_QiziXY[i];
        if (xy < 90) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    setFEN(pieceChars);
    //wcout << info_[L"FEN"] << endl;

    function<void(Move&)> __read = [&](Move& move) {
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
        int frowcol{ fcolrow % 10 * 10 + fcolrow / 10 }, trowcol{ tcolrow % 10 * 10 + tcolrow / 10 }; // 行列转换

        //wcout << frowcol << L' ' << trowcol << endl;
        //const shared_ptr<Seat>&fseat{ board_->getSeat(frowcol % 10, frowcol / 10) }, &tseat{ board_->getSeat(trowcol % 10, trowcol / 10) };
        move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        //wcout << move.toString() << endl;
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
            move.setRemark(Tools::s2ws(rem));

            //wcout << move.remark() << endl;
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

    __read(*rootMove_);
}

void Instance::readPGN(const string& filename, const RecFormat fmt)
{
    wstring pgnTxt{ Tools::readTxt(filename) };
    auto pos = pgnTxt.find(L"》");
    wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != wsregex_iterator{}; ++p)
        info_[(*p)[1]] = (*p)[2];
    if (fmt == RecFormat::CC)
        __readCC(moveStr);
    else
        __readICCSZH(moveStr, fmt);
}

void Instance::__readICCSZH(const wstring& moveStr, const RecFormat fmt)
{
    wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    wstring mvStr{ fmt == RecFormat::ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                        : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](shared_ptr<Move> move, const wstring mvstr, bool isOther) { //# 非递归
        for (wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != wsregex_iterator{}; ++p) {
            auto newMove = make_shared<Move>();
            if (fmt == RecFormat::ZH)
                newMove->setZh((*p)[1]);
            else
                newMove->setIccs((*p)[1]);
            wstring rem{ (*p)[2] };
            if (rem.size() > 0)
                newMove->setRemark(rem);
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
    vector<shared_ptr<Move>> othMoves{ rootMove_ };
    wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        rootMove_->setRemark(wsm.str(1));
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {
        //wcout << *wtleft << L"\n---------------------------------------------\n" << endl;
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
}

void Instance::__readCC(const wstring& fullMoveStr)
{
    auto pos = fullMoveStr.find(L"\n(");
    wstring moveStr{ fullMoveStr.substr(0, pos) }, remStr{ pos < fullMoveStr.size() ? fullMoveStr.substr(pos) : L"" };
    wregex spfat{ LR"(\n)" }, mstrfat{ LR"(.{5})" },
        movefat{ LR"(([^…　]{4}[…　]))" }, remfat{ LR"(\(\s*(\d+,\d+)\): \{([\s\S]*?)\})" };
    int row{ 0 };
    vector<vector<wstring>> movv{};
    wsregex_token_iterator msp{ moveStr.begin(), moveStr.end(), spfat, -1 }, end{};
    for (; msp != end; ++msp)
        if (row++ % 2 == 0) {
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
            move.setRemark(remm[key]);
    };
    function<void(Move&, int, int, bool)> __read = [&](Move& move, int row, int col, bool isOther) {
        wstring zh{ movv[row][col] };
        if (regex_match(zh, movefat)) {
            auto newMove = make_shared<Move>();
            newMove->setZh(zh.substr(0, 4));
            __setRem(*newMove, row, col);
            if (isOther)
                move.setOther(newMove);
            else
                move.setNext(newMove);
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

void Instance::readBIN(const string& filename)
{
    ifstream ifs(filename, ios_base::binary);
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
    function<void(Move&)> __read = [&](Move& move) {
        char frowcol{}, trowcol{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        //char fseat{}, tseat{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        //ifs.get(fseat).get(tseat).get(hasNext).get(hasOther);

        ifs.get(frowcol).get(trowcol).get(tag);
        //ifs.get(fseat).get(tseat).get(tag);

        //const shared_ptr<Seat>&fseat{ board_->getSeat(frowcol / 10, frowcol % 10) }, &tseat{ board_->getSeat(trowcol / 10, trowcol % 10) };
        move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        //move.setSeat(fseat, tseat);

        hasNext = tag & 0x80;
        hasOther = tag & 0x40;
        hasRemark = tag & 0x08;

        if (hasRemark) {
            //if (length > 0) {
            char len[sizeof(int)]{};
            ifs.read(len, sizeof(int));
            int length{ *(int*)len }; // 如果不采用位存储模式，此三行要移至if语句外！

            char rem[length + 1]{};
            ifs.read(rem, length);
            move.setRemark(Tools::s2ws(rem));
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

    __read(*rootMove_);
}

void Instance::readJSON(const string& filename)
{
    ifstream ifs(filename);
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    Json::Value infoItem{ root["info_"] };
    for (auto& key : infoItem.getMemberNames())
        info_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    function<void(Move&, Json::Value&)> __read = [&](Move& move, Json::Value& item) {
        int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };

        //const shared_ptr<Seat>&fseat{ board_->getSeat(frowcol / 10, frowcol % 10) }, &tseat{ board_->getSeat(trowcol / 10, trowcol % 10) };
        //int fseat{ item["f"].asInt() }, tseat{ item["t"].asInt() };
        move.setSeats(board_->getSeat(frowcol), board_->getSeat(trowcol));
        //move.setSeats(fseat, tseat);

        if (item.isMember("r"))
            move.setRemark(Tools::s2ws(item["r"].asString()));
        if (item.isMember("n")) { //# 有左子树
            move.setNext(make_shared<Move>());
            __read(*move.next(), item["n"]);
        }
        if (item.isMember("o")) { // # 有右子树
            move.setOther(make_shared<Move>());
            __read(*move.other(), item["o"]);
        }
    };

    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __read(*rootMove_, rootItem);
}

void Instance::writeBIN(const string& filename) const
{
    ofstream ofs(filename, ios_base::binary);
    ofs.put(char(info_.size()));
    for (auto& kv : info_) {
        string keys{ Tools::ws2s(kv.first) }, values{ Tools::ws2s(kv.second) };
        char klen{ char(keys.size()) }, vlen{ char(values.size()) };
        ofs.put(klen).write(keys.c_str(), klen).put(vlen).write(values.c_str(), vlen);
    }
    function<void(const Move&)> __write = [&](const Move& move) {
        string remark{ Tools::ws2s(move.remark()) };
        int len{ int(remark.size()) };

        int frowcol{ move.fseat()->rc() }, trowcol{ move.tseat()->rc() };
        ofs.put(char(frowcol)).put(char(trowcol));
        //ofs.put(char(move.fseat())).put(char(move.tseat()));

        //ofs.put(char(move.next() ? true : false)).put(char(move.other() ? true : false));
        ofs.put(char(move.next() ? 0x80 : 0x00) | char(move.other() ? 0x40 : 0x00) | char(len > 0 ? 0x08 : 0x00));

        if (len > 0) {
            ofs.write((char*)&len, sizeof(int)); // 如果不采用位存储模式，则移至if语句外，每move都要保存！
            //if (len > 0)
            ofs.write(remark.c_str(), len);
        }
        if (move.next())
            __write(*move.next());
        if (move.other())
            __write(*move.other());
    };

    __write(*rootMove_);
}

void Instance::writeJSON(const string& filename) const
{
    ofstream ofs(filename);
    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value infoItem;
    for (auto& k_v : info_)
        infoItem[Tools::ws2s(k_v.first)] = Tools::ws2s(k_v.second);
    root["info_"] = infoItem;

    function<void(const Move&, Json::Value&)> __write = [&](const Move& move, Json::Value& item) {
        item["f"] = move.fseat()->rc();
        item["t"] = move.tseat()->rc();
        //item["f"] = move.fseat();
        //item["t"] = move.tseat();
        if (move.remark().size() > 0)
            item["r"] = Tools::ws2s(move.remark());
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

    Json::Value rootItem{};
    __write(*rootMove_, rootItem);
    root["moves"] = rootItem;

    writer->write(root, &ofs);
}

void Instance::writePGN(const string& filename, const RecFormat fmt) const
{
    wstringstream wss{};
    for (const auto& m : info_)
        wss << L'[' << m.first << L" \"" << m.second << L"\"]\n";
    wss << L"》";
    Tools::writeTxt(filename, wss.str() + (fmt == RecFormat::CC ? toString_CC() : toString_ICCSZH(fmt)));
}

const wstring Instance::toString_ICCSZH(const RecFormat fmt) const
{
    wstringstream wss{};
    function<void(const Move&)> __remark = [&](const Move& move) {
        if (move.remark().size() > 0)
            wss << L"\n{" << move.remark() << L"}\n";
    };

    __remark(*rootMove_);
    function<void(const Move&, bool)> __moveStr = [&](const Move& move, bool isOther) {
        int boutNum{ (move.getStepNo() + 1) / 2 };
        bool isEven{ move.getStepNo() % 2 == 0 };
        if (isOther)
            wss << L"(" << boutNum << L". " << (isEven ? L"... " : L"");
        else
            wss << (isEven ? L" " : to_wstring(boutNum) + L". ");
        /*
        if (isEven)
            wss << L" ";
        else
            wss << boutNum << L". ";
            */
        wss << (fmt == RecFormat::ZH ? move.zh() : move.iccs()) << L' ';
        __remark(move);
        if (move.other()) {
            __moveStr(*move.other(), true);
            wss << L") ";
        }
        if (move.next())
            __moveStr(*move.next(), false);
    };

    // 驱动调用函数
    if (rootMove_->next())
        __moveStr(*rootMove_->next(), false);
    return wss.str();
}

const wstring Instance::toString_CC() const
{
    wstringstream remstrs{};
    wstring lstr((getMaxCol() + 1) * 5, L'　');
    vector<wstring> lineStr((getMaxRow() + 1) * 2, lstr);
    function<void(const Move&)> __setChar = [&](const Move& move) {
        int firstcol{ move.getCC_Col() * 5 }, row{ move.getStepNo() * 2 };
        for (int i = 0; i < 4; ++i)
            lineStr[row][firstcol + i] = move.zh()[i];
        if (move.remark().size() > 0)
            remstrs << L"(" << move.getStepNo() << L"," << move.getCC_Col() << L"): {" << move.remark() << L"}\n";
        if (move.next()) {
            lineStr[row + 1][firstcol + 2] = L'↓';
            __setChar(*move.next());
        }
        if (move.other()) {
            for (int c = firstcol + 4, e = move.other()->getCC_Col() * 5; c < e; ++c)
                lineStr[row][c] = L'…';
            __setChar(*move.other());
        }
    };

    __setChar(*rootMove_);
    wstringstream wss{};
    lineStr[0][0] = L'开';
    lineStr[0][1] = L'始';
    for (auto& line : lineStr)
        wss << line << L'\n';
    wss << remstrs.str() << __moveInfo();
    return wss.str();
}

const wstring Instance::__moveInfo() const
{
    wstringstream wss{};
    wss << L"【着法深度：" << maxRow << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

void Instance::setFEN(const wstring& pieceChars)
{
    info_[L"FEN"] = board_->getFEN(pieceChars) + L" " + (firstColor_ == PieceColor::RED ? L"r" : L"b") + L" - - 0 1";
}

void Instance::setBoard()
{
    wstring rfen{ info_[L"FEN"] };
    board_->putPieces(rfen.substr(0, rfen.find(L' ')));
}

// （rootMove）调用, 设置树节点的seat or zh'  // C++primer P512
void Instance::setMoves(const RecFormat fmt)
{
    function<void(Move&)> __setRemData = [&](const Move& move) {
        if (move.remark().size() > 0) {
            ++remCount;
            remLenMax = max(remLenMax, static_cast<int>(move.remark().size()));
        }
    };

    function<void(Move&)> __set = [&](Move& move) {
        if (fmt == RecFormat::ICCS || fmt == RecFormat::ZH || fmt == RecFormat::CC)
            move.setSeats(board_->getMoveSeats(move, fmt));
        if (fmt != RecFormat::ZH && fmt != RecFormat::CC)
            move.setZh(board_->getZh(move));
        if (fmt != RecFormat::ICCS) //RecFormat::XQF RecFormat::BIN RecFormat::JSON
            move.setIccs(board_->getIccs(move));

        ++movCount;
        maxCol = max(maxCol, move.getOthCol());
        maxRow = max(maxRow, move.getStepNo());
        move.setCC_Col(maxCol); // # 本着在视图中的列数
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

    __setRemData(*rootMove_);
    if (rootMove_->next())
        __set(*rootMove_->next()); // 驱动函数
}

const string Instance::getExtName(const RecFormat fmt)
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

const RecFormat Instance::getRecFormat(const string& ext)
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

const wstring Instance::toString() const
{
    wstringstream wss{};
    wss << board_->toString() << toString_CC();
    return wss.str();
}

const wstring Instance::test() const
{
    wstringstream wss{};
    wss << board_->test(); // << toString();
    return wss.str();
}