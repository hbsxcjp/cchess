#ifndef INFO_H
#define INFO_H

#include <map>
using std::map;

#include <string>
using std::wstring;

class Info {

public:
    Info()
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
            { L"Version", L"" } } {};

    wstring toString()
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
    };

private:
    map<wstring, wstring> info;
};

#endif