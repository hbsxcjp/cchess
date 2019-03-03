#include "chessInstance.h"
#include "../json/json.h"
#include "board.h"
#include "board_base.h"
#include "move.h"
#include "piece.h"
#include "tools.h"
#include <cmath>
#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <sstream>
using namespace std;
using namespace Json;
using namespace Tools;
using namespace Board_base;

// ChessInstance
ChessInstance::ChessInstance()
    : pboard(make_shared<Board>())
    , prootMove(make_shared<Move>())
    , pcurrentMove(prootMove)
    , firstColor(PieceColor::RED)
    , info{ { L"Author", L"" },
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

ChessInstance::ChessInstance(const string& filename)
    : ChessInstance()
{
    RecFormat fmt{ getRecFormat(getExt(filename)) };
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

    wstring rfen{ info[L"FEN"] }, fen{ rfen.substr(0, rfen.find(L' ')) };
    pboard = make_shared<Board>(__fenToPieChars(fen));
    __initSet(fmt);
}

void ChessInstance::write(const string& fname, const RecFormat fmt)
{
    string filename{ fname + getExtName(fmt) };
    switch (fmt) {
    case RecFormat::BIN:
        info[L"Format"] = L"BIN";
        writeBIN(filename);
        break;
    case RecFormat::JSON:
        info[L"Format"] = L"JSON";
        writeJSON(filename);
        break;
    default:
        switch (fmt) {
        case RecFormat::ICCS:
            info[L"Format"] = L"ICCS";
            break;
        case RecFormat::ZH:
            info[L"Format"] = L"ZH";
            break;
        case RecFormat::CC:
            info[L"Format"] = L"CC";
            break;
        default:
            break;
        }
        writePGN(filename, fmt);
        break;
    }
}

inline const PieceColor ChessInstance::currentColor() const
{
    return pcurrentMove->stepNo % 2 == 0
        ? firstColor
        : (firstColor == PieceColor::RED ? PieceColor::BLACK
                                         : PieceColor::RED);
}

inline const bool ChessInstance::isStart() const { return bool(pcurrentMove->prev()); }

inline const bool ChessInstance::isLast() const { return bool(pcurrentMove->next()); }

// 基本走法
void ChessInstance::forward()
{
    if (!isLast()) {
        pcurrentMove = pcurrentMove->next();
        pboard->go(*pcurrentMove);
    }
}

void ChessInstance::backward()
{
    if (!isStart()) {
        pboard->back(*pcurrentMove);
        pcurrentMove = pcurrentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void ChessInstance::forwardOther()
{
    if (pcurrentMove->other()) {
        auto toMove = pcurrentMove->other();
        pboard->back(*pcurrentMove);
        pboard->go(*toMove);
        pcurrentMove = toMove;
    }
}

// 复合走法
const vector<shared_ptr<Move>> ChessInstance::getPrevMoves(shared_ptr<Move> pmove) const
{
    vector<shared_ptr<Move>> pmv{ pmove };
    while (!pmove->prev()) {
        pmove = pmove->prev();
        pmv.push_back(pmove);
    }
    std::reverse(pmv.begin(), pmv.end());
    return pmv;
}

void ChessInstance::backwardTo(shared_ptr<Move> pmove)
{
    while (!isStart() && pmove != pcurrentMove) {
        backward();
        pmove = pmove->prev();
    }
}

void ChessInstance::to(shared_ptr<Move> pmove)
{
    if (pmove == pcurrentMove)
        return;
    toFirst();
    for (auto& pm : getPrevMoves(pmove))
        pboard->go(*pm);
    pcurrentMove = pmove;
}

void ChessInstance::toFirst()
{
    while (!isStart())
        backward();
}

void ChessInstance::toLast()
{
    while (!isLast())
        forward();
}

void ChessInstance::go(const int inc)
{
    //function<void(const ChessInstance*)> fward = inc > 0 ? &ChessInstance::forward : &ChessInstance::backward;//未成功!
    auto fward = mem_fn(inc > 0 ? &ChessInstance::forward : &ChessInstance::backward);
    for (int i = abs(inc); i != 0; --i)
        fward(this);
}

void ChessInstance::cutNext() { pcurrentMove->setNext(nullptr); }

void ChessInstance::cutOther()
{
    if (pcurrentMove->other())
        pcurrentMove->setOther(pcurrentMove->other()->other());
}

void ChessInstance::readXQF(const string& filename)
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
    info[L"Version_xqf"] = to_wstring(version);
    info[L"Result"] = (map<char, wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult[0]];
    info[L"PlayType"] = (map<char, wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]];
    info[L"TitleA"] = s2ws(TitleA);
    info[L"Event"] = s2ws(Event);
    info[L"Date"] = s2ws(Date);
    info[L"Site"] = s2ws(Site);
    info[L"Red"] = s2ws(Red);
    info[L"Black"] = s2ws(Black);
    info[L"Opening"] = s2ws(Opening);
    info[L"RMKWriter"] = s2ws(RMKWriter);
    info[L"Author"] = s2ws(Author);

    unsigned char head_KeyXY{ (unsigned char)(headKeyXY[0]) }, head_KeyXYf{ (unsigned char)(headKeyXYf[0]) },
        head_KeyXYt{ (unsigned char)(headKeyXYt[0]) }, head_KeysSum{ (unsigned char)(headKeysSum[0]) };
    unsigned char* head_QiziXY{ (unsigned char*)headQiziXY };
    if (Signature[0] != 0x58 || Signature[1] != 0x51)
        wcout << s2ws(Signature) << L" 文件标记不对。Signature != (0x58, 0x51)\n";
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
    info[L"FEN"] = __pieceCharsToFEN(pieceChars);

    function<void(Move&)> __read = [&](Move& move) {
        auto __byteToSeat = [&](int a, int b) {
            int xy = __subbyte(a, b);
            return getSeat(xy % 10, xy / 10);
        };
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
        move.setSeat(__byteToSeat(data[0], 0X18 + KeyXYf), __byteToSeat(data[1], 0X20 + KeyXYt));
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

    __read(*prootMove);
}

void ChessInstance::readPGN(const string& filename, const RecFormat fmt)
{
    wstring pgnTxt{ readTxt(filename) };
    auto pos = pgnTxt.find(L"》");
    wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != wsregex_iterator{}; ++p)
        info[(*p)[1]] = (*p)[2];
    if (fmt == RecFormat::CC)
        __fromCC(moveStr);
    else
        __fromICCSZH(moveStr, fmt);
}

void ChessInstance::__fromICCSZH(const wstring& moveStr, const RecFormat fmt)
{
    wstring preStr{ LR"((?:\d+\.)?\s*\b([)" };
    wstring mvStr{ fmt == RecFormat::ZH ? LR"(帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平)"
                                        : LR"(abcdefghi\d)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    wstring lastStr{ LR"(]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    wregex moveReg{ preStr + mvStr + lastStr };

    auto setMoves = [&](shared_ptr<Move> pmove, const wstring mvstr, bool isOther) { //# 非递归
        for (wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != wsregex_iterator{}; ++p) {
            auto newMove = make_shared<Move>();
            if (fmt == RecFormat::ZH)
                newMove->zh = (*p)[1];
            else
                newMove->ICCS = (*p)[1];
            wstring rem{ (*p)[2] };
            if (rem.size() > 0)
                newMove->remark = rem;
            if (isOther) { // # 第一步为变着
                pmove->setOther(newMove);
                isOther = false;
            } else
                pmove->setNext(newMove);
            pmove = newMove;
        }
        return pmove;
    };

    shared_ptr<Move> pmove;
    vector<shared_ptr<Move>> othMoves{ prootMove };
    wregex rempat{ LR"(\{([\s\S]*?)\}\s*1\.\s+)" }, spleft{ LR"(\(\d+\.\B)" }, spright{ LR"(\s+\)\B)" }; //\B:符号与空白之间为非边界
    wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    wsmatch wsm;
    if (regex_search((*wtleft).first, (*wtleft).second, wsm, rempat))
        prootMove->remark = wsm.str(1);
    bool isOther{ false }; // 首次非变着
    for (; wtleft != end; ++wtleft) {
        //wcout << *wtleft << L"\n---------------------------------------------\n" << endl;
        wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            //wcout << *wtright << L"\n---------------------------------------------\n" << endl;
            pmove = setMoves(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(pmove);
        isOther = true;
    }
}

void ChessInstance::__fromCC(const wstring& fullMoveStr)
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

    __setRem(*prootMove, 0, 0);
    if (int(movv.size()) > 0)
        __read(*prootMove, 1, 0, false);
}

const wstring ChessInstance::getICCS(const int fseat, const int tseat) const
{
    wstringstream wss{};
    wstring ColChars{ L"abcdefghi" };
    wss << ColChars[getCol(fseat)] << getRow(fseat) << ColChars[getCol(tseat)] << getRow(tseat);
    return wss.str();
}

//(fseat, tseat)->中文纵线着法
const wstring ChessInstance::getZh(const int fseat, const int tseat) const
{
    auto __getNumChar = [&](const PieceColor color, const int col) { return getNumChars(color)[col]; };
    auto __find_index = [](const vector<int>& seats, const int seat) {
        int index{ 0 };
        for (auto st : seats)
            if (seat == st)
                break;
            else
                ++index;
        return index;
    };

    wstringstream wss{};
    int fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    shared_ptr<Piece> fromPiece{ pboard->getPiece(fseat) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    bool isBottomSide{ pboard->isBottomSide(color) };
    vector<int> seats{ pboard->getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };
    auto __getChar = [&](const int col) {
        return __getNumChar(color, isBottomSide ? MaxCol - col : col);
    };

    if (length > 1 && isStronge(name)) {
        if (isPawn(name)) {
            seats = sortPawnSeats(isBottomSide,
                pboard->getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[__find_index(seats, fseat)] << name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << __getChar(fromCol);

    int toRow{ getRow(tseat) };
    //wcout << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退')) << endl;

    wss << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退'))
        << (isLine(name) && toRow != fromRow
                   ? __getNumChar(color, toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1)
                   : __getChar(getCol(tseat)));
    return wss.str();
}

const pair<int, int> ChessInstance::getSeat__ICCS(const wstring& ICCS) const
{
    string iccs{ ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97), getSeat(iccs[3] - 48, iccs[2] - 97));
}

//中文纵线着法->(fseat, tseat)
const pair<int, int> ChessInstance::getSeat__Zh(const wstring& zhStr) const
{
    int index, fseat, tseat;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ getNumChars(PieceColor::RED).find(zhStr.back()) != wstring::npos
            ? PieceColor::RED
            : PieceColor::BLACK };
    bool isBottomSide = pboard->isBottomSide(color);
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](const wchar_t ch) {
        return static_cast<int>(getNumChars(color).find(ch)) + 1;
    };
    auto __getCol = [&](const int num) { return isBottomSide ? ColNum - num : num - 1; };
    auto __getIndex = [](const wchar_t ch) {
        static map<wchar_t, int> ChNum_Indexs{ { L'一', 0 }, { L'二', 1 }, { L'三', 2 },
            { L'四', 3 }, { L'五', 4 }, { L'前', 0 }, { L'中', 1 }, { L'后', 1 },
            { L'进', 1 }, { L'退', -1 }, { L'平', 0 } };
        return ChNum_Indexs[ch];
    };

    if (isPiece(name)) {
        seats = pboard->getSideNameColSeats(color, name, __getCol(__getNum(zhStr[1])));

        if (seats.size() < 1)
            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << pboard->toString() << endl;

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && isAdvBishop(name) && (zhStr[2] == L'退') == isBottomSide)
            ? seats.size() - 1
            : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = __getIndex(zhStr[0]);
        name = zhStr[1];
        seats = pboard->getSideNameSeats(color, name);

        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << pboard->toString();

        if (isPawn(name)) {
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
    int movDir{ __getIndex(zhStr[2]) * (isBottomSide ? 1 : -1) },
        num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    if (isLine(name)) {
        //#'获取直线走子toseat'
        int row = getRow(fseat);
        tseat = (movDir == 0) ? getSeat(row, toCol) : getSeat(row + movDir * num, getCol(fseat));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step{ abs(toCol - getCol(fseat)) }; //  # 相距1或2列
        int inc{ (isAdvBishop(name)) ? step : (step == 1 ? 2 : 1) };
        tseat = getSeat(getRow(fseat) + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

void ChessInstance::readBIN(const string& filename)
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
        info[s2ws(key)] = s2ws(value);
    }
    function<void(Move&)> __read = [&](Move& move) {
        char fseat{}, tseat{}, hasNext{}, hasOther{}, hasRemark{}, tag{};
        //ifs.get(fseat).get(tseat).get(hasNext).get(hasOther);

        ifs.get(fseat).get(tseat).get(tag);
        move.setSeat(fseat, tseat);

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

    __read(*prootMove);
}

void ChessInstance::readJSON(const string& filename)
{
    ifstream ifs(filename);
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
        return;

    Json::Value infoItem{ root["info"] };
    for (auto& key : infoItem.getMemberNames())
        info[s2ws(key)] = s2ws(infoItem[key].asString());
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

    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __read(*prootMove, rootItem);
}

const wstring ChessInstance::__pieceCharsToFEN(const wstring& pieceChars) const
{
    //'下划线字符串对应数字字符'
    vector<pair<wstring, wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    wstring::size_type pos;
    wstring ws{};
    for (int i = 81; i >= 0; i -= 9)
        ws += pieceChars.substr(i, 9) + L"/";
    ws.erase(ws.size() - 1, 1);
    for (auto linenum : line_nums)
        while ((pos = ws.find(linenum.first)) != wstring::npos)
            ws.replace(pos, linenum.first.size(), linenum.second);
    return ws;
}

const wstring ChessInstance::__fenToPieChars(const wstring fen) const
{
    //'数字字符对应下划线字符串'
    vector<pair<wchar_t, wstring>> num_lines{
        { L'9', L"_________" }, { L'8', L"________" }, { L'7', L"_______" },
        { L'6', L"______" }, { L'5', L"_____" }, { L'4', L"____" },
        { L'3', L"___" }, { L'2', L"__" }, { L'1', L"_" }
    };
    wstring chars{};
    wregex sp{ LR"(/)" };
    for (wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != wsregex_token_iterator{}; ++wti)
        chars.insert(0, *wti);
    wstring::size_type pos;
    for (auto& numline : num_lines)
        while ((pos = chars.find(numline.first)) != wstring::npos)
            chars.replace(pos, 1, numline.second);
    return chars;
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void ChessInstance::__initSet(const RecFormat fmt)
{
    auto __setRem = [&](const Move& move) {
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
            /*
            wstring zh{ getZh(move.fseat(), move.tseat()) };
            // wcout << move.toString_zh() << L'\n';
            if (move.zh != zh) {
                wcout << L"move.zh: " << move.zh << L'\n'
                      << L"getZh( ): " << zh << L'\n'
                      << move.toString() << L'\n' << pboard->toString() << endl;
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
                      << move.toString() << L'\n' << pboard->toString() << endl;
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
        pboard->go(move);
        //wcout << move.toString() << L"\n" << pboard->toString() << endl;

        if (move.next())
            __set(*move.next());
        pboard->back(move);
        if (move.other()) {
            maxCol += 1;
            __set(*move.other());
        }
    };

    __setRem(*prootMove);
    if (prootMove->next())
        __set(*prootMove->next()); // 驱动函数
}

inline const wstring ChessInstance::getNumChars(const PieceColor color)
{
    return color == PieceColor::RED ? L"一二三四五六七八九" : L"１２３４５６７８９";
}

const string ChessInstance::getExtName(const RecFormat fmt)
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

const RecFormat ChessInstance::getRecFormat(const string& ext)
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

// 相关特征棋子: 类内声明，类外定义
const bool ChessInstance::isKing(const wchar_t name) { return wstring(L"帅将").find(name) != wstring::npos; }
const bool ChessInstance::isPawn(const wchar_t name) { return wstring(L"兵卒").find(name) != wstring::npos; }
const bool ChessInstance::isAdvBishop(const wchar_t name) { return wstring(L"仕相士象").find(name) != wstring::npos; }
const bool ChessInstance::isStronge(const wchar_t name) { return wstring(L"马车炮兵卒").find(name) != wstring::npos; }
const bool ChessInstance::isLine(const wchar_t name) { return wstring(L"帅车炮兵将卒").find(name) != wstring::npos; }
const bool ChessInstance::isPiece(const wchar_t name) { return wstring(L"帅仕相马车炮兵将士象卒").find(name) != wstring::npos; }

void ChessInstance::writeBIN(const string& filename) const
{
    ofstream ofs(filename, ios_base::binary);
    ofs.put(char(info.size()));
    for (auto& kv : info) {
        string keys{ ws2s(kv.first) }, values{ ws2s(kv.second) };
        char klen{ char(keys.size()) }, vlen{ char(values.size()) };
        ofs.put(klen).write(keys.c_str(), klen).put(vlen).write(values.c_str(), vlen);
    }
    function<void(const Move&)> __write = [&](const Move& move) {
        string remark{ ws2s(move.remark) };
        int len{ int(remark.size()) };

        ofs.put(char(move.fseat())).put(char(move.tseat()));
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

    __write(*prootMove);
}

void ChessInstance::writeJSON(const string& filename) const
{
    ofstream ofs(filename);
    Json::Value root;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value infoItem;
    for (auto& k_v : info)
        infoItem[ws2s(k_v.first)] = ws2s(k_v.second);
    root["info"] = infoItem;

    function<void(const Move&, Json::Value&)> __write = [&](const Move& move, Json::Value& item) {
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
    Json::Value rootItem{};
    __write(*prootMove, rootItem);
    root["moves"] = rootItem;

    writer->write(root, &ofs);
}

void ChessInstance::changeSide(const ChangeType ct) // 未测试
{
    auto curmove = pcurrentMove;
    toFirst();
    vector<pair<int, shared_ptr<Piece>>> seatPieces{};
    if (ct == ChangeType::EXCHANGE) {
        firstColor = firstColor == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
        for (auto& piecep : pboard->getLivePies())
            seatPieces.push_back(make_pair((*piecep).seat(), pboard->getOthPie(piecep)));
    } else {
        function<void(Move&)> __seat = [&](Move& move) {
            if (ct == ChangeType::ROTATE)
                move.setSeat(rotateSeat(move.fseat()), rotateSeat(move.tseat()));
            else
                move.setSeat(symmetrySeat(move.fseat()), symmetrySeat(move.tseat()));
            if (move.next())
                __seat(*move.next());
            if (move.other())
                __seat(*move.other());
        };
        if (prootMove->next())
            __seat(*prootMove->next()); // 驱动调用递归函数
        for (auto& piecep : pboard->getLivePies()) {
            auto seat = (*piecep).seat();
            seatPieces.push_back(make_pair(ct == ChangeType::ROTATE ? rotateSeat(seat) : symmetrySeat(seat), piecep));
        }
    }
    pboard->setSeatPieces(seatPieces);
    if (ct != ChangeType::ROTATE)
        __initSet(RecFormat::BIN); //借用？
    to(curmove);
}

void ChessInstance::writePGN(const string& filename, const RecFormat fmt) const
{
    wstringstream wss{};
    for (const auto m : info)
        wss << L'[' << m.first << L" \"" << m.second << L"\"]\n";
    wss << L"》";
    writeTxt(filename, wss.str() + (fmt == RecFormat::CC ? toString_CC() : toString_ICCSZH(fmt)));
}

const wstring ChessInstance::toString_ICCSZH(const RecFormat fmt) const
{
    wstringstream wss{};
    function<void(const Move&)> __remark = [&](const Move& move) {
        if (move.remark.size() > 0)
            wss << L"\n{" << move.remark << L"}\n";
    };

    __remark(*prootMove);
    function<void(const Move&, bool)> __moveStr = [&](const Move& move, bool isOther) {
        int boutNum{ (move.stepNo + 1) / 2 };
        bool isEven{ move.stepNo % 2 == 0 };
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
    if (prootMove->next())
        __moveStr(*prootMove->next(), false);
    return wss.str();
}

const wstring ChessInstance::toString_CC() const
{
    wstringstream remstrs{};
    wstring lstr((maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((maxRow + 1) * 2, lstr);
    function<void(const Move&)> __setChar = [&](const Move& move) {
        int firstcol = move.maxCol * 5;
        for (int i = 0; i < 4; ++i)
            lineStr[move.stepNo * 2][firstcol + i] = move.zh[i];
        if (move.remark.size() > 0)
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

    __setChar(*prootMove);
    wstringstream wss{};
    lineStr[0][0] = L'开';
    lineStr[0][1] = L'始';
    for (auto line : lineStr)
        wss << line << L'\n';
    wss << remstrs.str();
    wss << L"【着法深度：" << maxRow << L", 变着广度：" << othCol
        << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"】\n";
    return wss.str();
}

void ChessInstance::transDir(const string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string extensions{ ".xqf.pgn1.pgn2.pgn3.bin.json" };
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + getExtName(fmt) };
    function<void(string, string)> __trans = [&](const string& dirfrom, string dirto) {
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
                    string ext_old{ getExt(fname) };
                    if (extensions.find(ext_old) != string::npos) {
                        fcount += 1;
                        //cout << filename << endl;

                        ChessInstance ci(filename);
                        ci.write(fileto, fmt);
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
    cout << dirfrom + " =>" << getExtName(fmt) << ": 转换" << fcount << "个文件, "
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
    vector<RecFormat> fmts{ RecFormat::XQF, RecFormat::ICCS, RecFormat::ZH, RecFormat::CC,
        RecFormat::BIN, RecFormat::JSON };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}
