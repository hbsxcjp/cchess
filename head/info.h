#ifndef INFO_H
#define INFO_H

#include <vector>
#include <map>
#include <string>
#include <iostream>
using namespace std;

class Info {

public:
    Info();
    Info(const wstring& strPgn);
    Info(istream& is,  vector<int>& Keys, vector<int>& F32Keys);
    
    void toBin(ostream& ofs);

    void toFEN(wstring& pieceChars);
    wstring getPieChars();
    wstring toString();
    wstring test();

    map<wstring, wstring> info;

private:
};

#endif