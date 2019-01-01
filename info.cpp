#include "info.h"

// 类外定义
wstring Info::toString()
    {
        wstring ws{};
        for (const auto m : info)
            ws += m.first + L": " + m.second + L"\n";
        return ws;
    }

//外部文件定义全局函数
extern wstring test_info() {
    Info ainfo{};
    wstring ws{L"test info.h\n-----------------------------------------------------\nInfo::toString:\n"};
    ws += ainfo.toString() + L'\n';
    return ws;
}

