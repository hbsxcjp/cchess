#ifndef INFO_H
#define INFO_H

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
    wstring toString();
    void toBin(ostream& os);

    map<wstring, wstring> info;
private:
};

#endif