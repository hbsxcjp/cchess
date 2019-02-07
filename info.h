#ifndef INFO_H
#define INFO_H

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <string>
using std::wstring;
#include <iostream>
using std::istream;

class Info {

public:
    Info();
    Info(const wstring& strPgn);
    Info(istream& ifs,  vector<int>& Keys, vector<int>& F32Keys);
    
    void toFEN(wstring& pieceChars);
    wstring getPieChars();
    wstring toString();
    wstring test();

    map<wstring, wstring> info;

private:
};

#endif