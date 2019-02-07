#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <windows.h>

using namespace std;

string GBK_2_UTF8(string gbkStr)
{
    string outUtf8 = "";
    int n = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    wchar_t* str1 = new wchar_t[n];
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char* str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    outUtf8 = str2;
    delete[] str1;
    str1 = NULL;
    delete[] str2;
    str2 = NULL;
    return outUtf8;
}

wstring GBK_2_WS(string gbkStr)
{
    wstring outWS = L"";
    int n = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    wchar_t* str1 = new wchar_t[n];
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, str1, n);
    outWS = str1;
    delete[] str1;
    str1 = NULL;
    return outWS;
}

string UTF8_2_GBK(string utf8Str)
{
    string outGBK = "";
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    wchar_t* str1 = new wchar_t[n];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
    char* str2 = new char[n];
    WideCharToMultiByte(CP_ACP, 0, str1, -1, str2, n, NULL, NULL);
    outGBK = str2;
    delete[] str1;
    str1 = NULL;
    delete[] str2;
    str2 = NULL;
    return outGBK;
}

int m2w()
{
    int len;
    char* pmbnull = NULL;
    char* pmb = (char*)malloc(30);
    wchar_t* pwc = L"Hi葡萄美酒夜光杯y";
    wchar_t* pwcs = (wchar_t*)malloc(30);

    printf("转换为多字节字符串\n");
    len = wcstombs(pmb, pwc, 30);
    printf("被转换的字符 %d\n", len);
    printf("第一个多字节字符的十六进制值：%#.4x\n", pmb);
    cout << pmb << endl;

    printf("转换回宽字符字符串\n");
    len = mbstowcs(pwcs, pmb, 30);
    printf("被转换的字符 %d\n", len);
    printf("第一个宽字符的十六进制值：%#.4x\n\n", pwcs);
    wcout << pwcs << endl;

    return (0);
}

#define BUFFER_SIZE 50
int w2m()
{
    size_t ret;
    char* MB = (char*)malloc(BUFFER_SIZE);
    wchar_t* WC = L"http://葡萄美酒夜光杯www.w3cschool.cc";

    /* 转换宽字符字符串 */
    ret = wcstombs(MB, WC, BUFFER_SIZE);

    printf("要转换的字符数 = %u\n", ret);
    printf("多字节字符 = %s\n\n", MB);

    return (0);
}

std::string ws2s(const std::wstring& ws)
{
    //std::string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
    //setlocale(LC_ALL, "");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = 2 * ws.size() + 1;
    char* _Dest = new char[_Dsize]();
    wcstombs(_Dest, _Source, _Dsize);
    std::string result = _Dest;
    delete[] _Dest;
    //setlocale(LC_ALL, curLocale.c_str());
    return result;
}

std::wstring s2ws(const std::string& s)
{
    //setlocale(LC_ALL, "chs");
    const char* _Source = s.c_str();
    size_t _Dsize = s.size() + 1;
    wchar_t* _Dest = new wchar_t[_Dsize];
    mbstowcs(_Dest, _Source, _Dsize);
    std::wstring result = _Dest;
    delete[] _Dest;
    //setlocale(LC_ALL, "C");
    return result;
}

int main(void)
{
    setlocale(LC_ALL, ""); //"chs"
    std::ios_base::sync_with_stdio(false);

    char* src_str = "e葡萄rtte美酒夜光edd杯d";
    cout << "origin string: " << src_str << endl;
    
    wstring ws0 = s2ws(src_str);
    wcout << "utf8 to ws: " << ws0 << endl;
    cout << "ws to utf8: " << ws2s(ws0) << endl;
    
    string str_gbk = UTF8_2_GBK(src_str);
    wstring ws1 = s2ws(str_gbk);
    wcout << "gbk to ws: " << ws1 << endl;
    cout << "ws to gbk: " << ws2s(ws1) << endl;
/*
    string str_gbk = UTF8_2_GBK(src_str);
    cout << "utf8 to gbk: " << str_gbk << endl;

    // windows default is gbk
    // this program default is utf8
    string str_utf8 = GBK_2_UTF8(str_gbk);
    cout << "gbk to utf8: " << str_utf8 << endl;

    wstring ws = GBK_2_WS(str_gbk);
    wcout << "gbk to ws: " << ws << endl;
*/
    //m2w();
    //w2m();

    return 0;
}
