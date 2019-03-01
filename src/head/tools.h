#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <string>
#include <map>
using namespace std;


namespace Tools {

string trim(string& str);
wstring wtrim(wstring& str);
wstring s2ws(const string& s);
string ws2s(const wstring& ws);

const string getExt(const string filename);
wstring readTxt(const string fileName);
void writeTxt(const string fileName, const wstring ws);
void getFiles(const string path, vector<string>& files);
int copyFile(const char* sourceFile, const char* newFile);

// 测试函数
const wstring test();

} //

#endif