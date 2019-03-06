#ifndef MOVE_H
#define MOVE_H

#include <vector>
#include <memory>
using namespace std;

class Piece;

// 着法节点类
class Move {

public:
    Move();

    const int fseat() const { return fromseat; }
    const int tseat() const { return toseat; }
    void setSeat(const int fseat, const int tseat)
    {
        fromseat = fseat;
        toseat = tseat;
    }
    void setSeat(pair<int, int> seats) { setSeat(seats.first, seats.second); }

    const shared_ptr<Piece>& eatPiece() const { return eatPie_ptr; }
    const shared_ptr<Move>& prev() const { return prev_ptr; }
    const shared_ptr<Move>& next() const { return next_ptr; }
    const shared_ptr<Move>& other() const { return other_ptr; }
    void setEatPiece(shared_ptr<Piece> pie) { eatPie_ptr = pie; }
    void setPrev(shared_ptr<Move> prev) { prev_ptr = prev; }
    void setNext(shared_ptr<Move> next);
    void setOther(shared_ptr<Move> other);

    const vector<shared_ptr<Move>> getPrevMoves() const;
    const wstring toString() const;

    const wstring zhStr() const { return zh; }
    void setZh(const wstring& zhStr) { zh = zhStr; }
    const wstring iccsStr() const { return iccs; }
    void setIccs(const wstring& iccsStr) { iccs = iccsStr; }
    const wstring remarkStr() const { return remark; }
    void setRemark(const wstring& remarkStr) { remark = remarkStr; }
    const int getStepNo() const { return stepNo; }
    void setStepNo(const int curStepNo) { stepNo = curStepNo; }
    const int getOthCol() const { return othCol; }
    void setOthCol(const int curOthCol) { othCol = curOthCol; }
    const int getMaxCol() const { return maxCol; }
    void setMaxCol(const int curMaxCol) { maxCol = curMaxCol; }

private:
    int fromseat;
    int toseat;
    wstring remark{}; // 注释
    wstring iccs{}; // 着法数字字母描述
    wstring zh{}; // 着法中文描述

    shared_ptr<Piece> eatPie_ptr{};
    shared_ptr<Move> prev_ptr{};
    shared_ptr<Move> next_ptr{};
    shared_ptr<Move> other_ptr{};

    int stepNo{ 0 }; // 着法深度
    int othCol{ 0 }; // 变着广度
    int maxCol{ 0 }; // 图中列位置（需结合board确定）
};

#endif