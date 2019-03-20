#ifndef MOVE_H
#define MOVE_H

#include <memory>
#include <string>
#include <vector>
using namespace std;

class Piece;
class Seat;

// 着法节点类
class Move : public std::enable_shared_from_this<Move> {

public:
    Move();

    const shared_ptr<Move>& setSeats(const shared_ptr<Seat>& fseat, const shared_ptr<Seat>& tseat);
    const shared_ptr<Move>& setSeats(const pair<const shared_ptr<Seat>, const shared_ptr<Seat>>& seats);
    const shared_ptr<Move>& addNext(const shared_ptr<Move>& next = make_shared<Move>());
    const shared_ptr<Move>& addOther(const shared_ptr<Move>& other = make_shared<Move>());
    void setPrev(const shared_ptr<Move>& prev) { prev_ = weak_ptr<Move>(prev); }
    vector<shared_ptr<Move>> getPrevMoves();
    const shared_ptr<Move>& done();
    const shared_ptr<Move>& undo();

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
    const int getCC_Col() const { return CC_Col_; }
    void setCC_Col(const int CC_Col) { CC_Col_ = CC_Col; }

    const wstring toString() const;

private:
    shared_ptr<Seat> fseat_{};
    shared_ptr<Seat> tseat_{};
    shared_ptr<Piece> eatPie_;
    shared_ptr<Move> next_{};
    shared_ptr<Move> other_{};
    weak_ptr<Move> prev_{};

    wstring remark_{}; // 注释
    wstring iccs_{}; // 着法数字字母描述
    wstring zh_{}; // 着法中文描述
    int stepNo_{ 0 }; // 着法深度
    int othCol_{ 0 }; // 变着广度
    int CC_Col_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif