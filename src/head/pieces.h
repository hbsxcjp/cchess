#ifndef PIECES_H
#define PIECES_H

#include <memory>
#include <string>
#include <vector>
using namespace std;

// 棋子类
class Piece;
class Board;
enum class PieceColor;

// 一副棋子类
class Pieces {
public:
    Pieces();

    const shared_ptr<Piece> getKingPie(const PieceColor color) const;
    const shared_ptr<Piece> getOthPie(const shared_ptr<Piece> pie) const;
    const shared_ptr<Piece> getFreePie(const wchar_t ch) const;
    const vector<shared_ptr<Piece>> getPies() const { return piePtrs; }
    const vector<shared_ptr<Piece>> getLivePies() const;
    const vector<shared_ptr<Piece>> getLivePies(const PieceColor color) const;
    const vector<shared_ptr<Piece>> getLiveStrongePies(const PieceColor color) const;
    const vector<shared_ptr<Piece>> getNamePies(const PieceColor color, const wchar_t name) const;
    const vector<shared_ptr<Piece>> getNameColPies(const PieceColor color, const wchar_t name, int col) const;
    const vector<shared_ptr<Piece>> getEatedPies() const;
    const vector<shared_ptr<Piece>> getEatedPies(const PieceColor color) const;

    void clearSeat();
    // 相关特征棋子
    static const shared_ptr<Piece> nullPiePtr;

    const wstring toString() const;
    const wstring test() const;

private:
    vector<shared_ptr<Piece>> piePtrs;
};

#endif