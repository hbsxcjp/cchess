#include "chessInstanceIO.h"
#include "../json/json.h"
#include "board.h"
#include "board_base.h"
#include "chessInstance.h"
#include "move.h"
#include "piece.h"
#include "tools.h"
#include <map>
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

// ChessInstanceIO
void ChessInstanceIO::read(const string& filename, ChessInstance& ci)
{
    RecFormat fmt{ getRecFormat(getExt(filename)) };
    switch (fmt) {
    case RecFormat::XQF:
        readXQF(filename, ci);
        break;
    case RecFormat::BIN:
        readBIN(filename, ci);
        break;
    case RecFormat::JSON:
        readJSON(filename, ci);
        break;
    default:
        readPGN(filename, ci, fmt);
        break;
    }

    ci.setBoard();
    ci.initSet(fmt);
}

void ChessInstanceIO::write(const string& fname, ChessInstance& ci, const RecFormat fmt)
{
    string filename{ fname + getExtName(fmt) };
    map<wstring, wstring>& info = ci.getInfo();
    switch (fmt) {
    case RecFormat::BIN:
        info[L"Format"] = L"BIN";
        writeBIN(filename, ci);
        break;
    case RecFormat::JSON:
        info[L"Format"] = L"JSON";
        writeJSON(filename, ci);
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
        writePGN(filename, ci, fmt);
        break;
    }
}

void ChessInstanceIO::transDir(const string& dirfrom, const RecFormat fmt)
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

                        ChessInstance ci{};
                        read(filename, ci);
                        write(fileto, ci, fmt);
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

void ChessInstanceIO::testTransDir(int fd, int td, int ff, int ft, int tf, int tt)
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

void ChessInstanceIO::readXQF(const string& filename, ChessInstance& ci)
{
    map<wstring, wstring>& info = ci.getInfo();
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
    ci.setFEN(pieceChars);

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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    __read(*prootMove);
}

void ChessInstanceIO::readPGN(const string& filename, ChessInstance& ci, const RecFormat fmt)
{
    map<wstring, wstring>& info = ci.getInfo();
    wstring pgnTxt{ readTxt(filename) };
    auto pos = pgnTxt.find(L"》");
    wstring moveStr{ pos < pgnTxt.size() ? pgnTxt.substr(pos) : L"" };
    wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != wsregex_iterator{}; ++p)
        info[(*p)[1]] = (*p)[2];
    if (fmt == RecFormat::CC)
        __fromCC(moveStr, ci);
    else
        __fromICCSZH(moveStr, ci, fmt);
}

void ChessInstanceIO::__fromICCSZH(const wstring& moveStr, ChessInstance& ci, const RecFormat fmt)
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
    shared_ptr<Move>& prootMove = ci.getRootMove();
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

void ChessInstanceIO::__fromCC(const wstring& fullMoveStr, ChessInstance& ci)
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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    __setRem(*prootMove, 0, 0);
    if (int(movv.size()) > 0)
        __read(*prootMove, 1, 0, false);
}

void ChessInstanceIO::readBIN(const string& filename, ChessInstance& ci)
{
    map<wstring, wstring>& info = ci.getInfo();
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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    __read(*prootMove);
}

void ChessInstanceIO::readJSON(const string& filename, ChessInstance& ci)
{
    map<wstring, wstring>& info = ci.getInfo();
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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __read(*prootMove, rootItem);
}

void ChessInstanceIO::writeBIN(const string& filename, ChessInstance& ci)
{
    map<wstring, wstring>& info = ci.getInfo();
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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    __write(*prootMove);
}

void ChessInstanceIO::writeJSON(const string& filename, ChessInstance& ci)
{
    map<wstring, wstring>& info = ci.getInfo();
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
    shared_ptr<Move>& prootMove = ci.getRootMove();
    __write(*prootMove, rootItem);
    root["moves"] = rootItem;

    writer->write(root, &ofs);
}

void ChessInstanceIO::writePGN(const string& filename, ChessInstance& ci, const RecFormat fmt)
{
    map<wstring, wstring>& info = ci.getInfo();
    wstringstream wss{};
    for (const auto m : info)
        wss << L'[' << m.first << L" \"" << m.second << L"\"]\n";
    wss << L"》";
    writeTxt(filename, wss.str() + (fmt == RecFormat::CC ? toString_CC(ci) : toString_ICCSZH(ci, fmt)));
}

const wstring ChessInstanceIO::toString_ICCSZH(ChessInstance& ci, const RecFormat fmt)
{
    wstringstream wss{};
    function<void(const Move&)> __remark = [&](const Move& move) {
        if (move.remark.size() > 0)
            wss << L"\n{" << move.remark << L"}\n";
    };

    shared_ptr<Move>& prootMove = ci.getRootMove();
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

const wstring ChessInstanceIO::toString_CC(ChessInstance& ci)
{
    wstringstream remstrs{};
    wstring lstr((ci.maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((ci.maxRow + 1) * 2, lstr);
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

    shared_ptr<Move>& prootMove = ci.getRootMove();
    __setChar(*prootMove);
    wstringstream wss{};
    lineStr[0][0] = L'开';
    lineStr[0][1] = L'始';
    for (auto line : lineStr)
        wss << line << L'\n';
    wss << remstrs.str();
    wss << L"【着法深度：" << ci.maxRow << L", 变着广度：" << ci.othCol
        << L", 视图宽度：" << ci.maxCol << L", 着法数量：" << ci.movCount
        << L", 注解数量：" << ci.remCount << L", 注解最长：" << ci.remLenMax << L"】\n";
    return wss.str();
}

const string ChessInstanceIO::getExtName(const RecFormat fmt)
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

const RecFormat ChessInstanceIO::getRecFormat(const string& ext)
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