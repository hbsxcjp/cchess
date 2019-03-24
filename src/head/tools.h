#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <string>
#include <map>


namespace Tools {

std::string trim(std::string& str);
std::wstring wtrim(std::wstring& str);
std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& ws);

const std::string getExt(const std::string& filename);
std::wstring readTxt(const std::string& fileName);
void writeTxt(const std::string& fileName, const std::wstring& ws);
void getFiles(const std::string& path, std::vector<std::string>& files);
int copyFile(const char* sourceFile, const char* newFile);

// 测试函数
const std::wstring test();

} //

#endif