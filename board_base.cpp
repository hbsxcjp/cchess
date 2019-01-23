#include "board_base.h"
//#include "piece.h"

#include <string>
using std::string;
using std::wstring;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <regex>
using std::wregex;
using std::wsregex_iterator;

#include <utility>
using std::pair;

#include <fstream>
using std::wifstream;
using std::wofstream;
#include <sstream>
using std::wstringstream;

#include <algorithm>
using std::find;
using std::reverse;

#include <iomanip>
using std::boolalpha;
using std::setw;

#include <chrono>
using namespace std::chrono;

vector<int> Board_base::getSameColSeats(int seat, int othseat)
{
    vector<int> seats{};
    if (!isSameCol(seat, othseat))
        return seats;

    int step = seat < othseat ? 9 : -9;
    // 定义比较函数
    auto compare = [step](int i, int j) -> bool {
        return step > 0 ? i < j : i > j;
    };

    for (int i = seat + step; compare(i, othseat); i += step) {
        seats.push_back(i);
    }
    return seats;
}

vector<int> Board_base::getKingMoveSeats(int seat)
{
    int S{ seat - 9 }, W{ seat - 1 }, E{ seat + 1 }, N{ seat + 9 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{ E, N });
        else if (row == 1 || row == 8)
            return (vector<int>{ S, E, N });
        else
            return (vector<int>{ S, E });
    } else if (col == 4) {
        if (row == 0 || row == 7)
            return (vector<int>{ W, E, N });
        else if (row == 1 || row == 8)
            return (vector<int>{ S, W, E, N });
        else
            return (vector<int>{ S, W, E });
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{ W, N });
        else if (row == 1 || row == 8)
            return (vector<int>{ S, W, N });
        else
            return (vector<int>{ S, W });
    }
}

vector<int> Board_base::getAdvisorMoveSeats(int seat)
{
    int WS{ seat - 9 - 1 }, ES{ seat - 9 + 1 }, WN{ seat + 9 - 1 }, EN{ seat + 9 + 1 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{ EN });
        else
            return (vector<int>{ ES });
    } else if (col == 4) {
        return (vector<int>{ WS, ES, WN, EN });
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{ WN });
        else
            return (vector<int>{ WS });
    }
}

// 获取移动、象心行列值
vector<pair<int, int>> Board_base::getBishopMove_CenSeats(int seat)
{
    auto cen = [seat](int s) { return (seat + s) / 2; };

    int EN{ seat + 2 * 9 + 2 }, ES{ seat - 2 * 9 + 2 }, WS{ seat - 2 * 9 - 2 },
        WN{ seat + 2 * 9 - 2 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    if (col == MaxCol)
        return (vector<pair<int, int>>{ { WS, cen(WS) }, { WN, cen(WN) } });
    else if (col == MinCol)
        return (vector<pair<int, int>>{ { EN, cen(EN) }, { ES, cen(ES) } });
    else if (row == 0 || row == 5)
        return (vector<pair<int, int>>{ { WN, cen(WN) }, { EN, cen(EN) } });
    else if (row == 4 || row == 9)
        return (vector<pair<int, int>>{ { WS, cen(WS) }, { ES, cen(ES) } });
    else
        return (vector<pair<int, int>>{
            { WS, cen(WS) }, { WN, cen(WN) }, { ES, cen(ES) }, { EN, cen(EN) } });
}

// 获取移动、马腿行列值
vector<pair<int, int>> Board_base::getKnightMove_LegSeats(int seat)
{
    auto leg = [seat](int to) {
        switch (to - seat) {
        case 17:
        case 19:
            return seat + 9;
        case -17:
        case -19:
            return seat - 9;
        case 11:
        case -7:
            return seat + 1;
        default:
            return seat - 1;
        }
    };

    int EN{ seat + 11 }, ES{ seat - 7 }, SE{ seat - 17 }, SW{ seat - 19 },
        WS{ seat - 11 }, WN{ seat + 7 }, NW{ seat + 17 }, NE{ seat + 19 };
    int row{ getRow(seat) }, col{ getCol(seat) };
    switch (col) {
    case MaxCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{ { WS, leg(WS) }, { SW, leg(SW) } });
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { WN, leg(WN) } });
        case MinRow:
            return (vector<pair<int, int>>{ { WN, leg(WN) }, { NW, leg(NW) } });
        case MinRow + 1:
            return (vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { WS, leg(WS) } });
        default:
            return (vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { WS, leg(WS) }, { SW, leg(SW) } });
        }
    case MaxCol - 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) }, { WN, leg(WN) } });
        case MinRow:
            return (vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (vector<pair<int, int>>{
                { WN, leg(WN) }, { NW, leg(NW) }, { NE, leg(NE) }, { WS, leg(WS) } });
        default:
            return (vector<pair<int, int>>{ { WN, leg(WN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WS, leg(WS) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    case MinCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{ { ES, leg(ES) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                { ES, leg(ES) }, { SE, leg(SE) }, { EN, leg(EN) } });
        case MinRow:
            return (vector<pair<int, int>>{ { EN, leg(EN) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (vector<pair<int, int>>{
                { EN, leg(EN) }, { NE, leg(NE) }, { ES, leg(ES) } });
        default:
            return (vector<pair<int, int>>{
                { EN, leg(EN) }, { NE, leg(NE) }, { ES, leg(ES) }, { SE, leg(SE) } });
        }
    case MinCol + 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                { ES, leg(ES) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                { ES, leg(ES) }, { SW, leg(SW) }, { SE, leg(SE) }, { EN, leg(EN) } });
        case MinRow:
            return (vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { NE, leg(NE) }, { ES, leg(ES) } });
        default:
            return (vector<pair<int, int>>{ { EN, leg(EN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { ES, leg(ES) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    default:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                { ES, leg(ES) }, { WS, leg(WS) }, { SW, leg(SW) }, { SE, leg(SE) } });
        case MaxRow - 1:
            return (vector<pair<int, int>>{ { ES, leg(ES) },
                { WS, leg(WS) },
                { WN, leg(WN) },
                { SW, leg(SW) },
                { SE, leg(SE) },
                { EN, leg(EN) } });
        case MinRow:
            return (vector<pair<int, int>>{
                { EN, leg(EN) }, { NW, leg(NW) }, { WN, leg(WN) }, { NE, leg(NE) } });
        case MinRow + 1:
            return (vector<pair<int, int>>{ { EN, leg(EN) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WN, leg(WN) },
                { WS, leg(WS) },
                { ES, leg(ES) } });
        default:
            return (vector<pair<int, int>>{ { EN, leg(EN) },
                { ES, leg(ES) },
                { NW, leg(NW) },
                { NE, leg(NE) },
                { WN, leg(WN) },
                { WS, leg(WS) },
                { SE, leg(SE) },
                { SW, leg(SW) } });
        }
    }
}

// 车炮可走的四个方向位置
vector<vector<int>> Board_base::getRookCannonMoveSeat_Lines(int seat)
{
    vector<vector<int>> res{ vector<int>{}, vector<int>{}, vector<int>{},
        vector<int>{} };
    int row{ getRow(seat) }, left{ row * 9 - 1 }, right{ row * 9 + 9 };
    for (int i = seat - 1; i != left; --i)
        res[0].push_back(i);
    for (int i = seat + 1; i != right; ++i)
        res[1].push_back(i);
    for (int i = seat - 9; i > -1; i -= 9)
        res[2].push_back(i);
    for (int i = seat + 9; i < 90; i += 9)
        res[3].push_back(i);
    return res;
}

vector<int> Board_base::getPawnMoveSeats(bool isBottomSide, int seat)
{
    int E{ seat + 1 }, S{ seat - 9 }, W{ seat - 1 }, N{ seat + 9 }, row{ getRow(seat) };
    switch (getCol(seat)) {
    case MaxCol:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{ W, N });
            else
                return (vector<int>{ N });
        } else {
            if (row < 5)
                return (vector<int>{ W, S });
            else
                return (vector<int>{ S });
        }
    case MinCol:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{ E, N });
            else
                return (vector<int>{ N });
        } else {
            if (row < 5)
                return (vector<int>{ E, S });
            else
                return (vector<int>{ S });
        }
    default:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{ E, W, N });
            else
                return (vector<int>{ N });
        } else {
            if (row < 5)
                return (vector<int>{ E, W, S });
            else
                return (vector<int>{ S });
        }
    }
}

// '多兵排序'
vector<int> Board_base::sortPawnSeats(bool isBottomSide, vector<int> seats)
{
    map<int, vector<int>> temp{};
    vector<int> res(5);
    // 按列建立字典，按列排序
    for_each(seats.begin(), seats.end(),
        [&](int s) { temp[getCol(s)].push_back(s); });
    // 筛除只有一个位置的列, 整合成一个数组
    auto pos = res.begin();
    for_each(temp.begin(), temp.end(), [&](pair<int, vector<int>> col_seats) {
        if (col_seats.second.size() > 1){
            std::sort(col_seats.second.begin(), col_seats.second.end()); // 每列排序
            pos = copy(col_seats.second.begin(), col_seats.second.end(), pos); //按列存入
        }
    });
    res = vector<int>{ res.begin(), pos };    
    if (isBottomSide)
        reverse(res.begin(), res.end()); // 根据棋盘顶底位置,是否反序
    return res;
}

wstring Board_base::print_vector_int(vector<int> vi)
{
    wstringstream wss{};
    for (auto i : vi)
        wss << setw(3) << i << L' ';
    wss << L'\n';
    return wss.str();
}

wstring Board_base::readTxt(const char* fileName)
{
    wstringstream wss;
    wchar_t buf[1024];
    wifstream wifs(fileName);
    while (!wifs.eof()) {
        wifs.read(buf, 1024);
        wss.write(buf, wifs.gcount());
    }
    wifs.close();
    return wss.str();
}

void Board_base::writeTxt(const char* fileName, wstring ws)
{
    wofstream wofs(fileName);
    wofs << ws;
    wofs.close();
}

pair<map<wstring, wstring>, wstring> Board_base::getHead_Body(const char* filename)
{
    map<wstring, wstring> minfo;
    wregex pat{ LR"(\[(\w+)\s+\"(.*)\"\]\s+)" };
    wstring ws{ readTxt(filename) };
    for (wsregex_iterator p(ws.begin(), ws.end(), pat); p != wsregex_iterator{};
         ++p)
        minfo[(*p)[1]] = (*p)[2];
    return make_pair(minfo, std::regex_replace(ws, pat, L""));
}

// 测试
wstring Board_base::test()
{
    wstringstream wss{};
    wss << L"test "
           L"board_base.h\n----------------------------------------------------"
           L"-\n";
    wss << boolalpha << L"NumCols: " << L"ColNum " << ColNum << L" RowNum " << RowNum
        << L" MinCol " << MinCol << L" MaxCol " << MaxCol << L'\n';
    wss << L"multiSeats:\n";
    // vector<const vector<int>> multiSeats = ;
    for (auto aseats : { allSeats, bottomKingSeats, topKingSeats,
             bottomAdvisorSeats, topAdvisorSeats, bottomBishopSeats,
             topBishopSeats, bottomPawnSeats, topPawnSeats }) {
        for (auto seat : aseats)
            wss << setw(2) << seat << L' ';
        wss << L'\n';
    }
    wss << L"\nFEN: " << FEN << L"\nColChars: " << ColChars
        << L"\nTextBlankBoard: \n"
        << TextBlankBoard << L'\n';
    wss << L"allSeats rotateSeat symmetrySeat isSameCol\n";
    for (auto s : allSeats)
        wss << setw(5) << s << setw(10) << rotateSeat(s) << setw(12)
            << symmetrySeat(s) << setw(14) << isSameCol(s, s + 8) << L'\n';
    wss << L"getSameColSeats:\n";
    vector<pair<int, int>> vp{ { 84, 21 }, { 86, 23 }, { 66, 13 } };
    for (auto ss : vp) {
        for (auto s : getSameColSeats(ss.first, ss.second))
            wss << setw(3) << s << L' ';
        wss << L'\n';
    }

    wss << L"getKingMoveSeats:\n";
    vector<vector<int>> testSeats = { bottomKingSeats, topKingSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getKingMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }
    wss << L"getAdvisorMoveSeats:\n";
    testSeats = { bottomAdvisorSeats, topAdvisorSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getAdvisorMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }
    wss << L"getBishopMove_CenSeats:\n";
    testSeats = { bottomBishopSeats, topBishopSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getBishopMove_CenSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }
    wss << L"getKnightMove_LegSeats:\n";
    testSeats = { allSeats };
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getKnightMove_LegSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }

    // 获取全部行列的seat值
    auto getSeats = []() {
        for (int row = 0; row != 10; ++row)
            for (int col = 0; col != 9; ++col)
                getSeat(row, col);
    };
    // 获取全部seat值的行列
    auto getRowCols = []() {
        for (int seat = 0; seat != 90; ++seat) {
            getRow(seat);
            getCol(seat);
        }
    };
    int count{ 10000 };
    auto t0 = steady_clock::now();
    for (int i = 0; i != count; ++i) {
        getSeats();
        getRowCols();
    }
    auto d = steady_clock::now() - t0;
    wss << "getSeats: use time -> " << duration_cast<milliseconds>(d).count()
        << "ms" << L'\n';

    return wss.str();
}
