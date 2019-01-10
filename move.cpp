#include "move.h"
#include "board.h"

#include <algorithm>
using std::sort;

void Move::setNext(Move *next_) {
    next_->stepNo = stepNo + 1; // 步数
    next_->othCol = othCol;     // 变着层数
    next_->prev = this;

    nt = next_;
}

void Move::setOther(Move *other) {
    other->stepNo = stepNo;     // 与premove的步数相同
    other->othCol = othCol + 1; // 变着层数
    other->prev = prev;

    ot = other;
}

void Move::setSeat__ICCS() {
    fs = getSeat(static_cast<int>(ColChars.find(da[1])),
                 static_cast<int>(da[0]));
    ts = getSeat(static_cast<int>(ColChars.find(da[3])),
                 static_cast<int>(da[2]));
}

void Move::setICCS() {
    da = L"" + static_cast<wchar_t>(getCol(fs)) + ColChars[getRow(fs)] +
         static_cast<wchar_t>(getCol(ts)) + ColChars[getRow(ts)];
}

wstring Move::getStr(InstanceFormat fmt) {
    switch (fmt) {
    case InstanceFormat::ICCS:
        return da;
    case InstanceFormat::zh:
        return zh;
    default: // 留待以后添加其他的描述格式
        return zh;
    }
}

//根据中文纵线着法描述取得源、目标位置: (fseat, tseat)
void Move::setSeat__ZhStr(Board *board) {
    int index;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{Side_ChNums[PieceColor::red].find(zh[zh.size() - 1]) !=
                             wstring::npos
                         ? PieceColor::red
                         : PieceColor::black};
    bool isBottomSide = board->isBottomSide(color);
    wchar_t name{zh[0]};
    auto __getNum = [&](wchar_t ch) {
        return static_cast<int>(Side_ChNums[color].find(ch)) + 1;
    };
    auto __getCol = [&](int num) { return isBottomSide ? 9 - num : num - 1; };
    if (Pieces::allNames.find(name) != wstring::npos) {
        int col{__getCol(__getNum(zh[1]))};
        for (auto p : board->pieces.getNameColPies(color, name, col))
            seats.push_back(p->seat());
        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 &&
                 Pieces::advbisNames.find(name) != wstring::npos &&
                 (zh[2] == L'退') == isBottomSide)
                    ? seats.size() - 1
                    : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = ChNum_Indexs[zh[0]];
        name = zh[1];
        for (auto p : board->pieces.getNamePies(color, name))
            seats.push_back(p->seat());
        if (Pieces::pawnNames.find(name) != wstring::npos) {
            seats = __sortPawnSeats(isBottomSide, seats);
            //#获取多兵的列
            if (seats.size() == 3 && zh[0] == L'后')
                index += 1;
        } else {
            if (seats.size() < 2) {
                // console.log(`棋子列表少于2个 => ${zhStr} color:${color}name:
                // ${name}\n${this}`);
            }
            if (isBottomSide) //# 修正index
                index = seats.size() - index - 1;
        }
    }
    sort(seats.begin(), seats.end(), [&](int a, int b) { return a < b; });
    // if (seats.length === 0) console.log(`没有找到棋子 =>
    // ${zhStr}color:${color} name: ${name}\n${board}`);
    fs = seats[index];
    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{Direction_Nums[zh[2]] * (isBottomSide ? 1 : -1)},
        num{__getNum(zh[3])}, toCol{__getCol(num)};
    if (Pieces::lineNames.find(name) != wstring::npos) {
        //#'获取直线走子toseat'
        int row = getRow(fs);
        ts = (movDir == 0) ? getSeat(row, toCol)
                           : (getSeat(row + movDir * num, getCol(fs)));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step = toCol - getCol(fs); //  # 相距1或2列
        if (step < 0)
            step *= -1;
        int inc = Pieces::advbisNames.find(name) != wstring::npos
                      ? step
                      : (step == 1 ? 2 : 1);
        ts = getSeat(getRow(fs) + movDir * inc, toCol);
        // console.log(this.tseat);
    }
    // 断言已通过
    // this.setZhStr(board);
    // if (zhStr != this.zhStr)
    //    console.log(board.toString(), zhStr, '=>', this.fseat,
    //    this.tseat,'=>', this.zhStr);
}

// 根据源、目标位置: (fseat, tseat)取得中文纵线着法描述
void Move::setZhStr(Board *board) {}

wstring Move::toString() {
    wstring res{};
    return res;
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void Move::__initSet(function<void()> setFunc, Board *board) {}

// '多兵排序'
vector<int> Move::__sortPawnSeats(bool isBottomSide, vector<int> seats) {
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
    if (isBottomSide) {
        vector<int> restmp(5);
        auto postmp = copy_backward(res.begin(), pos, restmp.begin());
        return (vector<int>{restmp.begin(), postmp});
    } else
        return (vector<int>{res.begin(), pos});
}

void Moves::fromJSON(wstring moveJSON) {}
// （rootMove）调用
void Moves::fromICCSZh(wstring moveStr, Board *board) {}
void Moves::fromCC(wstring moveStr, Board *board) {}

wstring Moves::toJSON() {
    wstring res{};
    return res;
} // JSON

wstring Moves::toString() {
    wstring res{};
    return res;
}