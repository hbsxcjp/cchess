#ifndef MOVE_H
#define MOVE_H

#include <memory>
#include <string>
#include <vector>
using namespace std;

class Piece;
class Seat;

// 着法节点类
class Move {

public:
    Move() = default;

    void setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat);
    void setNext(shared_ptr<Move> next);
    void setOther(shared_ptr<Move> other);
    void setPrev(shared_ptr<Move> prev) { prev_ = weak_ptr<Move>(prev); }
    vector<shared_ptr<Move>> getPrevMoves();
    void done();
    void undo();

    const shared_ptr<Seat>& fseat() const { return fseat_; }
    const shared_ptr<Seat>& tseat() const { return tseat_; }
    const shared_ptr<Piece>& eatPie() const { return eatPie_; }
    const shared_ptr<Move>& next() const { return next_; }
    const shared_ptr<Move>& other() const { return other_; }
    const shared_ptr<Move>& prev() const { return move(prev_.lock()); }

    const wstring zh() const { return zh_; }
    void setZh(const wstring& zh) { zh_ = zh; }
    const wstring iccs() const { return iccs_; }
    void setIccs(const wstring& iccs) { iccs_ = iccs; }
    const wstring remark() const { return remark_; }
    void setRemark(const wstring& remark) { remark_ = remark; }
    const int getStepNo() const { return stepNo_; }
    void setStepNo(const int stepNo) { stepNo_ = stepNo; }
    const int getOthCol() const { return othCol_; }
    void setOthCol(const int othCol) { othCol_ = othCol; }
    const int getMaxCol() const { return maxCol_; }
    void setMaxCol(const int maxCol) { maxCol_ = maxCol; }

    const wstring toString() const;
private:
    shared_ptr<Seat> fseat_{};
    shared_ptr<Seat> tseat_{};
    shared_ptr<Piece> eatPie_{};
    shared_ptr<Move> next_{};
    shared_ptr<Move> other_{};
    weak_ptr<Move> prev_{};

    wstring remark_{}; // 注释
    wstring iccs_{}; // 着法数字字母描述
    wstring zh_{}; // 着法中文描述
    int stepNo_{ 0 }; // 着法深度
    int othCol_{ 0 }; // 变着广度
    int maxCol_{ 0 }; // 图中列位置（需结合board确定）
};

#endif