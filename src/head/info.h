#ifndef INFO_H
#define INFO_H

#include "board_base.h"
#include "../json/json.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Info {

public:
    Info();
    Info(istream& is, vector<int>& Keys, vector<int>& F32Keys);
    Info(const wstring& strPgn);
    Info(istream& is);
    Info(Json::Value& infoItem);

    void setRecFormat(RecFormat fmt);
    RecFormat getRecFormat();
    void setFEN(wstring& pieceChars);
    wstring getPieChars();
    
    wstring toString(RecFormat fmt = RecFormat::ZH);
    void toBin(ostream& os);
    void toJson(Json::Value& root);

    map<wstring, wstring> info;

private:
};

#endif