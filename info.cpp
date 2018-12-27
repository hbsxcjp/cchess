#include "base.h"

Info::Info(string pgn){}
Info::Info()
    : info{ { L"Author", L"" },
        { L"Black", L"" },
        { L"BlackTeam", L"" },
        { L"Date", L"" },
        { L"ECCO", L"" },
        { L"Event", L"" },
        { L"FEN", FEN },
        { L"Format", L"zh" },
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
        { L"Version", L"" } } {}

wstring Info::toString()
{
    wstring ws{};
    /*
    return Object.keys(this.info)
        .map((k) = > `[$ { k } "${this.info[k]}"]`)
        .join('\n');
        */
    for (const auto m : this->info)
        ws += m.first + L": " + m.second + L"\n";
    return ws;
}

Info* Info::setFromPgn(wstring infoStr)
{ /*
    let result;
    let regexp = /\[(\S +) "(.*)"\] / gm;
    while ((result = regexp.exec(infoStr)) != null) {
        this.info[result[1]] = result[2];
    } //# 读取info内容（在已设置原始key上可能会有增加）
*/
}


