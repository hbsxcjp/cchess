#include "info.h"

#include <functional>
#include <iomanip>
#include <regex>
#include <sstream>

using namespace std;
using namespace Board_base;


Info::Info()
    : info{ { L"Author", L"" },
        { L"Black", L"" },
        { L"BlackTeam", L"" },
        { L"Date", L"" },
        { L"ECCO", L"" },
        { L"Event", L"" },
        { L"FEN", FENPro },
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

Info::Info(const wstring& pgnTxt)
    : Info()
{
    wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\])" };
    for (wsregex_iterator p(pgnTxt.begin(), pgnTxt.end(), pat); p != wsregex_iterator{}; ++p)
        info[(*p)[1]] = (*p)[2];
}

Info::Info(istream& is, vector<int>& Keys, vector<int>& F32Keys)
{
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

    is.read(Signature, 2).read(Version_xqf, 1).read(headKeyMask, 1).read(ProductId, 4); // = 8 bytes
    is.read(headKeyOrA, 1).read(headKeyOrB, 1).read(headKeyOrC, 1).read(headKeyOrD, 1).read(headKeysSum, 1).read(headKeyXY, 1).read(headKeyXYf, 1).read(headKeyXYt, 1); // = 16 bytes
    is.read(headQiziXY, 32); // = 48 bytes
    is.read(PlayStepNo, 2).read(headWhoPlay, 1).read(headPlayResult, 1).read(PlayNodes, 4).read(PTreePos, 4).read(Reserved1, 4); // = 64 bytes
    is.read(headCodeA_H, 16).read(TitleA, 64).read(TitleB, 64); // 80 + 128 = 208 bytes
    is.read(Event, 64).read(Date, 16).read(Site, 16).read(Red, 16).read(Black, 16); // = 336 bytes
    is.read(Opening, 64).read(Redtime, 16).read(Blktime, 16).read(Reservedh, 32); // = 464 bytes
    is.read(RMKWriter, 16).read(Author, 16); // = 496 bytes
    is.ignore(528); // = 1024 bytes

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
    //wcout << L"XY_XYf_XYt_Sum_Size: " << 0 + head_KeyXY << L'/'
    //    << 0 + head_KeyXYf << L'/' << 0 + head_KeyXYt << L'/'
    //  << 0 + head_KeysSum << L'/' << KeyRMKSize << endl;

    char KeyBytes[4];
    KeyBytes[0] = (headKeysSum[0] & headKeyMask[0]) | headKeyOrA[0];
    KeyBytes[1] = (headKeyXY[0] & headKeyMask[0]) | headKeyOrB[0];
    KeyBytes[2] = (headKeyXYf[0] & headKeyMask[0]) | headKeyOrC[0];
    KeyBytes[3] = (headKeyXYt[0] & headKeyMask[0]) | headKeyOrD[0];
    string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    //vector<char> F32Keys;
    for (int i = 0; i != 32; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)
    Keys = vector<int>{ version, KeyXYf, KeyXYt, KeyRMKSize };
    wstring pieceChars(90, L'_');
    for (int i = 0; i != 32; ++i) {
        int xy = head_QiziXY[i];
        if (xy < 90) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    setFEN(pieceChars);
}

Info::Info(istream& is)
    : Info()
{
    char size{}, klen{}, vlen{};
    is.get(size);
    for (int i = 0; i != size; ++i) {
        is.get(klen);
        char key[klen + 1]{};
        is.read(key, klen);
        is.get(vlen);
        char value[vlen + 1]{};
        is.read(value, vlen);
        info[s2ws(key)] = s2ws(value);
    }
}

//Info::Info(wistream& wis)    : Info(){}

Info::Info(Json::Value& infoItem)
    : Info()
{
    for (auto& key : infoItem.getMemberNames())
        info[s2ws(key)] = s2ws(infoItem[key].asString());
}

void Info::setRecFormat(RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::XQF:
        info[L"Format"] = L"XQF";
        break;
    case RecFormat::ICCS:
        info[L"Format"] = L"ICCS";
        break;
    case RecFormat::ZH:
        info[L"Format"] = L"ZH";
        break;
    case RecFormat::CC:
        info[L"Format"] = L"CC";
        break;
    case RecFormat::BIN:
        info[L"Format"] = L"BIN";
        break;
    default:
        info[L"Format"] = L"JSON";
        break;
    }
}

RecFormat Info::getRecFormat()
{
    wstring wsfmt{ info[L"Format"] };
    if (wsfmt == L"XQF")
        return RecFormat::XQF;
    else if (wsfmt == L"ICCS")
        return RecFormat::ICCS;
    else if (wsfmt == L"ZH")
        return RecFormat::ZH;
    else if (wsfmt == L"CC")
        return RecFormat::CC;
    else if (wsfmt == L"BIN")
        return RecFormat::BIN;
    else
        return RecFormat::JSON;
}

void Info::setFEN(wstring& pieceChars)
{
    //'下划线字符串对应数字字符'
    vector<pair<wstring, wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    wstring::size_type pos;
    wstring res{};
    for (int i = 81; i >= 0; i -= 9)
        res += pieceChars.substr(i, 9) + L"/";
    res.erase(res.size() - 1, 1);
    for (auto l_n : line_nums)
        while ((pos = res.find(l_n.first)) != wstring::npos)
            res.replace(pos, l_n.first.size(), l_n.second);
    info[L"FEN"] = res;
}

wstring Info::getPieChars()
{
    //'数字字符对应下划线字符串'
    vector<pair<wchar_t, wstring>> num_lines{
        { L'9', L"_________" }, { L'8', L"________" }, { L'7', L"_______" },
        { L'6', L"______" }, { L'5', L"_____" }, { L'4', L"____" },
        { L'3', L"___" }, { L'2', L"__" }, { L'1', L"_" }
    };
    wstring chars{}, fen{ info[L"FEN"] };
    fen = fen.substr(0, fen.find(L' '));
    wregex sp{ LR"(/)" };
    for (wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != wsregex_token_iterator{}; ++wti)
        chars.insert(0, *wti);
    wstring::size_type pos;
    for (auto nl : num_lines)
        while ((pos = chars.find(nl.first)) != wstring::npos)
            chars.replace(pos, 1, nl.second);
    return chars;
}

void Info::toBin(ostream& os)
{
    setRecFormat(RecFormat::BIN);
    os.put(char(info.size()));
    for (auto& kv : info) {
        string keys{ ws2s(kv.first) }, values{ ws2s(kv.second) };
        char klen{ char(keys.size()) }, vlen{ char(values.size()) };
        os.put(klen).write(keys.c_str(), klen).put(vlen).write(values.c_str(), vlen);
    }
}

void Info::toJson(Json::Value& root)
{
    Json::Value infoItem;
    setRecFormat(RecFormat::JSON);
    for (auto& k_v : info)
        infoItem[ws2s(k_v.first)] = ws2s(k_v.second);
    root["info"] = infoItem;
}

wstring Info::toString(RecFormat fmt)
{
    wstringstream wss{};
    setRecFormat(fmt);
    for (const auto m : info)
        wss << L'[' << m.first << L" \"" << m.second << L"\"]\n";
    return wss.str();
}
