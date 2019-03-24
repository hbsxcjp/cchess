#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include "seat.h"
#include <memory>
#include <string>
#include <vector>

class Piece;
class Seat;

// 着法节点类
class Move : public std::enable_shared_from_this<Move> {

public:
    Move();

    const std::shared_ptr<Move>& setSeats(const std::shared_ptr<SeatSpace::Seat>& fseat, const std::shared_ptr<SeatSpace::Seat>& tseat);
    const std::shared_ptr<Move>& setSeats(const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>& seats);
    const std::shared_ptr<Move>& addNext(const std::shared_ptr<Move>& next = std::make_shared<Move>());
    const std::shared_ptr<Move>& addOther(const std::shared_ptr<Move>& other = std::make_shared<Move>());
    void setPrev(const std::shared_ptr<Move>& prev) { prev_ = std::weak_ptr<Move>(prev); }
    std::vector<std::shared_ptr<Move>> getPrevMoves();
    const std::shared_ptr<Move>& done();
    const std::shared_ptr<Move>& undo();

    const std::shared_ptr<SeatSpace::Seat>& fseat() const { return fseat_; }
    const std::shared_ptr<SeatSpace::Seat>& tseat() const { return tseat_; }
    const std::shared_ptr<PieceSpace::Piece>& eatPie() const { return eatPie_; }
    const std::shared_ptr<Move>& next() const { return next_; }
    const std::shared_ptr<Move>& other() const { return other_; }
    const std::shared_ptr<Move>& prev() const { return std::move(prev_.lock()); }

    const std::wstring zh() const { return zh_; }
    void setZh(const std::wstring& zh) { zh_ = zh; }
    const std::wstring iccs() const { return iccs_; }
    void setIccs(const std::wstring& iccs) { iccs_ = iccs; }
    const std::wstring remark() const { return remark_; }
    void setRemark(const std::wstring& remark) { remark_ = remark; }
    const int getStepNo() const { return stepNo_; }
    void setStepNo(const int stepNo) { stepNo_ = stepNo; }
    const int getOthCol() const { return othCol_; }
    void setOthCol(const int othCol) { othCol_ = othCol; }
    const int getCC_Col() const { return CC_Col_; }
    void setCC_Col(const int CC_Col) { CC_Col_ = CC_Col; }

    const std::wstring toString() const;

private:
    std::shared_ptr<SeatSpace::Seat> fseat_{};
    std::shared_ptr<SeatSpace::Seat> tseat_{};
    std::shared_ptr<PieceSpace::Piece> eatPie_;
    std::shared_ptr<Move> next_{};
    std::shared_ptr<Move> other_{};
    std::weak_ptr<Move> prev_{};

    std::wstring remark_{}; // 注释
    std::wstring iccs_{}; // 着法数字字母描述
    std::wstring zh_{}; // 着法中文描述
    int stepNo_{ 0 }; // 着法深度
    int othCol_{ 0 }; // 变着广度
    int CC_Col_{ 0 }; // 图中列位置（需在Instance::setMoves确定）
};

#endif