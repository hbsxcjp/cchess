#ifndef INFO_H
#define INFO_H

#include "board_base.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

class Info {

public:
    Info();
    Info(const wstring& strPgn);
    Info(istream& is,  vector<int>& Keys, vector<int>& F32Keys);
    Info(istream& is);    

    void toFEN(wstring& pieceChars);
    wstring getPieChars();
    wstring toString(RecFormat fmt);
    void toBin(ostream& os);

    map<wstring, wstring> info;
private:
};

#endif