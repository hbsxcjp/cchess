#include "info.h"
#include "board_base.h"

#include <sstream>
using std::wstringstream;

using namespace Board_base;

Info::Info()
    : info{ { L"Author", L"" },
        { L"Black", L"" },
        { L"BlackTeam", L"" },
        { L"Date", L"" },
        { L"ECCO", L"" },
        { L"Event", L"" },
        { L"FEN", L"" },
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
        { L"Version", L"" } }
{
}

Info::Info(map<wstring, wstring> minfo)
    : info{ minfo }
{
}

wstring Info::getFEN()
{
    wstring fen{ info[L"FEN"] };
    if (fen.size() == 0)
        return FEN;
    return wstring{ fen, 0, fen.find(L' ') };
}

// 类外定义
wstring Info::toString()
{
    wstring ws{};
    for (const auto m : info)
        ws += m.first + L": " + m.second + L"\n";
    return ws;
}

wstring Info::test()
{
    Info ainfo{};
    wstringstream wss;
    wss << L"test info.h\n"
        << L"-----------------------------------------------------\n"
        << L"Info::toString:\n";
    wss << ainfo.toString() << L'\n';
    return wss.str();
}
