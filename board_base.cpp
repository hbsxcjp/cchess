#include "board_base.h"
//#include "piece.h"

#include <string>
using std::string;
using std::wstring;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <utility>
using std::pair;

#include <fstream>
using std::wifstream;
using std::wofstream;
#include <sstream>
using std::wstringstream;

#include <iomanip>
using std::boolalpha;
using std::setw;

#include <chrono>
using namespace std::chrono;

// 函数
int Board_base::getRow(int seat) { return seat / 9; }
int Board_base::getCol(int seat) { return seat % 9; }
int Board_base::getSeat(int row, int col) { return row * 9 + col; }
int Board_base::rotateSeat(int seat) { return 89 - seat; }
int Board_base::symmetrySeat(int seat) {
    return (getRow(seat) + 1) * 9 - seat % 9 - 1;
}
bool Board_base::isSameCol(int seat, int othseat) {
    return getCol(seat) == getCol(othseat);
}

vector<int> Board_base::getSameColSeats(int seat, int othseat) {
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

vector<int> Board_base::getKingMoveSeats(int seat) {
    int S{seat - 9}, W{seat - 1}, E{seat + 1}, N{seat + 9};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{E, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, E, N});
        else
            return (vector<int>{S, E});
    } else if (col == 4) {
        if (row == 0 || row == 7)
            return (vector<int>{W, E, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, W, E, N});
        else
            return (vector<int>{S, W, E});
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{W, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, W, N});
        else
            return (vector<int>{S, W});
    }
}

vector<int> Board_base::getAdvisorMoveSeats(int seat) {
    int WS{seat - 9 - 1}, ES{seat - 9 + 1}, WN{seat + 9 - 1}, EN{seat + 9 + 1};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{EN});
        else
            return (vector<int>{ES});
    } else if (col == 4) {
        return (vector<int>{WS, ES, WN, EN});
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{WN});
        else
            return (vector<int>{WS});
    }
}

// 获取移动、象心行列值
vector<pair<int, int>> Board_base::getBishopMove_CenSeats(int seat) {
    auto cen = [seat](int s) { return (seat + s) / 2; };

    int EN{seat + 2 * 9 + 2}, ES{seat - 2 * 9 + 2}, WS{seat - 2 * 9 - 2},
        WN{seat + 2 * 9 - 2};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == MaxCol)
        return (vector<pair<int, int>>{{WS, cen(WS)}, {WN, cen(WN)}});
    else if (col == MinCol)
        return (vector<pair<int, int>>{{EN, cen(EN)}, {ES, cen(ES)}});
    else if (row == 0 || row == 5)
        return (vector<pair<int, int>>{{WN, cen(WN)}, {EN, cen(EN)}});
    else if (row == 4 || row == 9)
        return (vector<pair<int, int>>{{WS, cen(WS)}, {ES, cen(ES)}});
    else
        return (vector<pair<int, int>>{
            {WS, cen(WS)}, {WN, cen(WN)}, {ES, cen(ES)}, {EN, cen(EN)}});
}

// 获取移动、马腿行列值
vector<pair<int, int>> Board_base::getKnightMove_LegSeats(int seat) {
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

    int EN{seat + 11}, ES{seat - 7}, SE{seat - 17}, SW{seat - 19},
        WS{seat - 11}, WN{seat + 7}, NW{seat + 17}, NE{seat + 19};
    int row{getRow(seat)}, col{getCol(seat)};
    switch (col) {
    case MaxCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{{WS, leg(WS)}, {SW, leg(SW)}});
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {WN, leg(WN)}});
        case MinRow:
            return (vector<pair<int, int>>{{WN, leg(WN)}, {NW, leg(NW)}});
        case MinRow + 1:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {WS, leg(WS)}});
        default:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {WS, leg(WS)}, {SW, leg(SW)}});
        }
    case MaxCol - 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}});
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}, {WN, leg(WN)}});
        case MinRow:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {NE, leg(NE)}});
        case MinRow + 1:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {NE, leg(NE)}, {WS, leg(WS)}});
        default:
            return (vector<pair<int, int>>{{WN, leg(WN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WS, leg(WS)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    case MinCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{{ES, leg(ES)}, {SE, leg(SE)}});
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SE, leg(SE)}, {EN, leg(EN)}});
        case MinRow:
            return (vector<pair<int, int>>{{EN, leg(EN)}, {NE, leg(NE)}});
        case MinRow + 1:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NE, leg(NE)}, {ES, leg(ES)}});
        default:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NE, leg(NE)}, {ES, leg(ES)}, {SE, leg(SE)}});
        }
    case MinCol + 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SW, leg(SW)}, {SE, leg(SE)}});
        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SW, leg(SW)}, {SE, leg(SE)}, {EN, leg(EN)}});
        case MinRow:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {NE, leg(NE)}});
        case MinRow + 1:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {NE, leg(NE)}, {ES, leg(ES)}});
        default:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {ES, leg(ES)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    default:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}});
        case MaxRow - 1:
            return (vector<pair<int, int>>{{ES, leg(ES)},
                                           {WS, leg(WS)},
                                           {WN, leg(WN)},
                                           {SW, leg(SW)},
                                           {SE, leg(SE)},
                                           {EN, leg(EN)}});
        case MinRow:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {WN, leg(WN)}, {NE, leg(NE)}});
        case MinRow + 1:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WN, leg(WN)},
                                           {WS, leg(WS)},
                                           {ES, leg(ES)}});
        default:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {ES, leg(ES)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WN, leg(WN)},
                                           {WS, leg(WS)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    }
}

// 车炮可走的四个方向位置
vector<vector<int>> Board_base::getRookCannonMoveSeat_Lines(int seat) {
    vector<vector<int>> res{vector<int>{}, vector<int>{}, vector<int>{},
                            vector<int>{}};
    int row{getRow(seat)}, left{row * 9 - 1}, right{row * 9 + 9};
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

vector<int> Board_base::getPawnMoveSeats(bool isBottomSide, int seat) {
    int E{seat + 1}, S{seat - 9}, W{seat - 1}, N{seat + 9}, row{getRow(seat)};
    switch (getCol(seat)) {
    case MaxCol:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{W, N});
            else
                return (vector<int>{N});
        } else {
            if (row < 5)
                return (vector<int>{W, S});
            else
                return (vector<int>{S});
        }
    case MinCol:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{E, N});
            else
                return (vector<int>{N});
        } else {
            if (row < 5)
                return (vector<int>{E, S});
            else
                return (vector<int>{S});
        }
    default:
        if (isBottomSide) {
            if (row > 4)
                return (vector<int>{E, W, N});
            else
                return (vector<int>{N});
        } else {
            if (row < 5)
                return (vector<int>{E, W, S});
            else
                return (vector<int>{S});
        }
    }
}

vector<int> Board_base::reverse(vector<int> seats) {
    vector<int> res(seats.size());
    auto pos = copy_backward(seats.begin(), seats.end(), res.begin());
    return (vector<int>{res.begin(), pos});
}

bool Board_base::find_char(wstring ws, wchar_t ch) {
    return ws.find(ch) != wstring::npos;
}

int Board_base::find_index(vector<int> seats, int seat) {
    int index{0};
    for (auto s : seats)
        if (s != seat)
            ++index;
    return index;
}

// '多兵排序'
vector<int> Board_base::sortPawnSeats(bool isBottomSide, vector<int> seats) {
    map<int, vector<int>> temp{};
    vector<int> res(5);
    // 按列建立字典，按列排序
    for_each(seats.begin(), seats.end(),
             [&](int s) { temp[getCol(s)].push_back(s); });
    // 筛除只有一个位置的列, 整合成一个数组
    auto pos = res.begin();
    for_each(temp.begin(), temp.end(), [&](pair<int, vector<int>> col_seats) {
        if (col_seats.second.size() > 1)
            pos = copy(col_seats.second.begin(), col_seats.second.end(), pos);
    });
    // 根据棋盘顶底位置,是否反序
    return isBottomSide ? reverse(vector<int>{res.begin(), pos})
                        : (vector<int>{res.begin(), pos});
}

wstring Board_base::print_vector_int(vector<int> vi) {
    wstringstream wss{};
    for (auto i : vi)
        wss << setw(3) << i << L' ';
    wss << L'\n';
    return wss.str();
}

// 读取文本文件
wstring Board_base::readTxt(string fileName) {
    wstring ws;
    wifstream wifs{fileName, std::ios_base::in};
    wifs >> std::noskipws;
    for (wchar_t ch; wifs >> ch;)
        ws += ch;
    wifs.close();
    return ws;
}

// 写入文本文件
void Board_base::writeTxt(string fileName, wstring ws) {
    wofstream wofs{fileName, std::ios_base::out};
    wofs << ws;
    wofs.close();
}

// 测试
wstring Board_base::test_constValue() {
    wstringstream wss{};
    wss << L"NumCols: ";
    wss << L"ColNum " << ColNum << L" RowNum " << RowNum << L" MinCol "
        << MinCol << L" MaxCol " << MaxCol << L'\n';

    wss << L"multiSeats:\n";
    // vector<const vector<int>> multiSeats = ;
    for (auto aseats : {allSeats, bottomKingSeats, topKingSeats,
                        bottomAdvisorSeats, topAdvisorSeats, bottomBishopSeats,
                        topBishopSeats, bottomPawnSeats, topPawnSeats}) {
        for (auto seat : aseats)
            wss << setw(2) << seat << L' ';
        wss << L'\n';
    }

    wss << L"Side_ChNums: ";
    for (auto colstr : Side_ChNums)
        wss << static_cast<int>(colstr.first) << L"-> " << colstr.second
            << L" ";

    wss << L"\nChNum_Indexs: ";
    for (auto chidx : ChNum_Indexs)
        wss << chidx.first << L"-> " << chidx.second << L" ";

    wss << L"\nDirection_Nums: ";
    for (auto dirnum : Direction_Nums)
        wss << dirnum.first << L"-> " << dirnum.second << L" ";

    wss << L"\nFEN: " << FEN << L"\nColChars: " << ColChars
        << L"\nTextBlankBoard: \n"
        << TextBlankBoard << L'\n';
    return wss.str();
}

// 测试
wstring Board_base::test_getSeats() {
    wstringstream wss{};
    wss << boolalpha;
    wss << L"allSeats rotateSeat symmetrySeat isSameCol\n";
    for (auto s : allSeats)
        wss << setw(5) << s << setw(10) << rotateSeat(s) << setw(12)
            << symmetrySeat(s) << setw(14) << isSameCol(s, s + 8) << L'\n';
    wss << L"getSameColSeats:\n";

    vector<pair<int, int>> vp{{84, 21}, {86, 23}, {66, 13}};
    for (auto ss : vp) {
        for (auto s : getSameColSeats(ss.first, ss.second))
            wss << setw(3) << s << L' ';
        wss << L'\n';
    }
    return wss.str();
}

// 测试
wstring Board_base::test_getMoveSeats() {
    wstringstream wss{};
    wss << L"getKingMoveSeats:\n";
    vector<vector<int>> testSeats = {bottomKingSeats, topKingSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getKingMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }

    wss << L"getAdvisorMoveSeats:\n";
    testSeats = {bottomAdvisorSeats, topAdvisorSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getAdvisorMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }

    wss << L"getBishopMove_CenSeats:\n";
    testSeats = {bottomBishopSeats, topBishopSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getBishopMove_CenSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }

    wss << L"getKnightMove_LegSeats:\n";
    testSeats = {allSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getKnightMove_LegSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }

    return wss.str();
}

// 测试
wstring Board_base::test_getRowCols() {
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

    wstringstream wss{};
    int count{10000};
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

wstring Board_base::test_board_base() {
    wstringstream wss{};
    wss << L"test "
           L"board_base.h\n----------------------------------------------------"
           L"-\n";
    wss << test_constValue();
    wss << test_getSeats();
    wss << test_getMoveSeats();
    wss << test_getRowCols();
    return wss.str();
}
