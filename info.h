#ifndef INFO_H
#define INFO_H

#include <map>
using std::map;

#include <string>
using std::wstring;

class Info {

public:
    Info();
    Info(map<wstring, wstring> minfo);

    wstring getFEN();

    wstring toString();
    wstring test();

    map<wstring, wstring> info;

private:
};

#endif