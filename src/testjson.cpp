#include "json.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;
using namespace std::chrono;

int main(int argc, char const* argv[])
{
    setlocale(LC_ALL, "");
    std::ios_base::sync_with_stdio(false);
    //auto time0 = steady_clock::now();

    Json::Value jsonRoot; //定义根节点
    Json::Value jsonItem; //定义一个子对象
    jsonItem["item1"] = "one"; //添加数据
    jsonItem["item2"] = 2;
    jsonRoot.append(jsonItem);
    jsonItem.clear(); //清除jsonItem
    jsonItem["item1.0"] = 1.0;
    jsonItem["item2.0"] = 2.0;
    jsonRoot["item"] = jsonItem;
    cout << jsonRoot.toStyledString() << endl; //输出到控制台

    //auto time_d = steady_clock::now() - time0;
    //cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

    return 0;
}