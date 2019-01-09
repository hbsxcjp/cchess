
#include "move.h"

void Move::setNext(Move *next_) {
    next_->stepNo = stepNo + 1;
    next_->othCol = othCol; // 变着层数
    next_->prev = this;

    nt = next_;
}

void Move::setOther(Move *other) {
    other->stepNo = stepNo;     // 与premove的步数相同
    other->othCol = othCol + 1; // 变着层数
    other->prev = prev;

    ot = other;
}

void Move::setSeat__ICCS(Board *board) {
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
    wstring res{};
    return res;
}

//根据中文纵线着法描述取得源、目标位置: (fseat, tseat)
void Move::setSeat__ZhStr(Board *board) {}

// 根据源、目标位置: (fseat, tseat)取得中文纵线着法描述
void Move::setZhStr(Board *board) {}

void Move::fromJSON(wstring moveJSON) {}
// （rootMove）调用
void Move::fromICCSZh(wstring moveStr, Board *board) {}
void Move::fromCC(wstring moveStr, Board *board) {}

wstring Move::toJSON() {
    wstring res{};
    return res;
} // JSON

wstring Move::toString() {
    wstring res{};
    return res;
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void Move::__initSet(function<void()> setFunc, Board *board) {}
