#include "piece.h"
#include "board.h"

#include <algorithm>
using std::remove_if;

vector<int> Piece::getCanMoveSeats(Board *board) {
    vector<int> res{getMoveSeats(board)};
    auto p = remove_if(res.begin(), res.end(), [&](int seat) {
        return board->getColor(seat) == color();
    });
    return (vector<int>{res.begin(), p});
    /*
    vector<int> res{};
    for (auto seat : getMoveSeats(board))
        if (board->getColor(seat) != color())
            res.push_back(seat);
    return res;
    */
}

vector<int> Piece::filterMove_obstruct(Board *board,
                                       vector<pair<int, int>> move_obs) {
    vector<int> res{};
    for (auto st_c : move_obs)
        if (board->isBlank(st_c.second))
            res.push_back(st_c.first);
    return res;
}

vector<int> Rook::getMoveSeats(Board *board) {
    vector<int> res{};
    auto seatLines = getRookCannonMoveSeat_Lines(seat());
    for (auto seatLine : seatLines)
        for (auto seat : seatLine) {
            res.push_back(seat);
            if (!board->isBlank(seat))
                break;
        }
    return res;
}

vector<int> Cannon::getMoveSeats(Board *board) {
    vector<int> res{};
    auto seatLines = getRookCannonMoveSeat_Lines(seat());
    for (auto seatLine : seatLines) {
        bool skip = false;
        for (auto seat : seatLine) {
            if (!skip) {
                if (board->isBlank(seat))
                    res.push_back(seat);
                else
                    skip = true;
            } else if (!board->isBlank(seat)) {
                res.push_back(seat);
                break;
            }
        }
    }
    return res;
}

vector<int> Pawn::getMoveSeats(Board *board) {
    return getPawnMoveSeats(board->isBottomSide(color()), seat());
}

wstring Piece::toString() {
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(4) << index() << setw(4) << static_cast<int>(color()) << setw(6)
        << wchar() << setw(6) << seat() << setw(6) << chName() << setw(9)
        << isBlank() << setw(8) << isKing() << setw(8) << isStronge() << L'\n';
    return wss.str();
}

// 相关特征棋子名字串: 类内声明，类外定义
const wstring Pieces::kingNames{L"帅将"};
const wstring Pieces::pawnNames{L"兵卒"};
const wstring Pieces::advbisNames{L"仕相士象"};
const wstring Pieces::strongeNames{L"马车炮兵卒"};
const wstring Pieces::lineNames{L"帅车炮兵将卒"};
const wstring Pieces::allNames{L"帅仕相马车炮兵将士象卒"};

// 类外初始化类内静态const成员
int Piece::curIndex{-1};
NullPie Pieces::nullPiece{NullPie(L'_')}; // 空棋子
Piece *Pieces::nullPiePtr{&nullPiece};    // 空棋子指针

// 一副棋子类
Pieces::Pieces()
    : kings{King(L'K'), King(L'k')}, advisors{Advisor(L'A'), Advisor(L'A'),
                                              Advisor(L'a'), Advisor(L'a')},
      bishops{Bishop(L'B'), Bishop(L'B'), Bishop(L'b'), Bishop(L'b')},
      knights{Knight(L'N'), Knight(L'N'), Knight(L'n'), Knight(L'n')},
      rooks{Rook(L'R'), Rook(L'R'), Rook(L'r'), Rook(L'r')},
      cannons{Cannon(L'C'), Cannon(L'C'), Cannon(L'c'), Cannon(L'c')},
      pawns{Pawn(L'P'), Pawn(L'P'), Pawn(L'P'), Pawn(L'P'), Pawn(L'P'),
            Pawn(L'p'), Pawn(L'p'), Pawn(L'p'), Pawn(L'p'), Pawn(L'p')} {
    for (auto &pie : kings)
        piePtrs.push_back(&pie);
    for (auto &pie : advisors)
        piePtrs.push_back(&pie);
    for (auto &pie : bishops)
        piePtrs.push_back(&pie);
    for (auto &pie : knights)
        piePtrs.push_back(&pie);
    for (auto &pie : rooks)
        piePtrs.push_back(&pie);
    for (auto &pie : cannons)
        piePtrs.push_back(&pie);
    for (auto &pie : pawns)
        piePtrs.push_back(&pie);
    Piece::curIndex = 0;
}

inline Piece *Pieces::getKingPie(PieceColor color) {
    return piePtrs[color == PieceColor::red ? 0 : 1];
}

inline Piece *Pieces::getOthPie(Piece *pie) {
    int index{pie->index()}, othIndex;
    if (index < 2)
        othIndex = index == 1 ? 0 : 1; //帅0将1
    else if (index > 21)
        othIndex = index > 26 ? index - 5 : index + 5; // 兵22-26卒27-31
    else
        othIndex =
            index % 4 < 2 ? index - 2 : index + 2; // 士象马车炮，每种棋4子
    return piePtrs[othIndex];
}

vector<Piece *> Pieces::getLivePies() {
    vector<Piece *> res{};
    for (auto p : piePtrs)
        if (p->seat() != nullSeat)
            res.push_back(p);
    return res;
}

vector<Piece *> Pieces::getLivePies(PieceColor color) {
    vector<Piece *> res{};
    for (auto p : piePtrs)
        if (p->seat() != nullSeat && p->color() == color)
            res.push_back(p);
    return res;
}

vector<Piece *> Pieces::getLiveStrongePies(PieceColor color) {
    vector<Piece *> res{};
    for (auto p : piePtrs)
        if (p->seat() != nullSeat && p->color() == color && p->isStronge())
            res.push_back(p);
    return res;
}

vector<Piece *> Pieces::getEatedPies() {
    vector<Piece *> res{};
    for (auto p : piePtrs)
        if (p->seat() == nullSeat)
            res.push_back(p);
    return res;
}

vector<Piece *> Pieces::getNamePies(PieceColor color, wchar_t name) {
    vector<Piece *> res{};
    for (auto p : getLivePies(color))
        if (p->chName() == name)
            res.push_back(p);
    return res;
}

wstring Pieces::toString() {
    wstring ws{
        L"index color wchar seat chName isBlank isKing isStronge\n"}; // index
    for (auto p : piePtrs)
        ws += p->toString();
    return ws + nullPiePtr->toString(); // 关注空棋子的特性!
}

// Pieces seatPieces(vector<int, wchar_t> seatChars)    {    }

wstring Pieces::test_piece() {
    // Pieces apieces = Pieces();
    wstringstream wss{};
    wss << L"test "
           L"piece.h\n-----------------------------------------------------\n"
        << L"特征棋子串：\n"
        << kingNames << L' ' << pawnNames << L' ' << advbisNames << L' '
        << strongeNames << L' ' << lineNames << L' ' << allNames << L"\n";

    wss << L"Pieces::toString:\n";
    wss << toString() << L"共计：" << piePtrs.size() << L"个\n";

    wss << L"Piece::getKingPie\\Piece::chName：\n红棋->"
        << getKingPie(PieceColor::red)->chName() << L' ' << L"黑棋->"
        << getKingPie(PieceColor::black)->chName();

    wss << L"\nPieces::getOthPie：\n";
    for (auto piePtr : piePtrs)
        wss << piePtr->chName() << L"->" << getOthPie(piePtr)->chName() << L' ';

    for (auto id : (vector<int>{0, 1, 2, 3, 4, 5, 9, 10, 17, 18}))
        piePtrs[id]->setSeat(id);
    wss << L"\nPieces::getLivePies：\n";
    wss << L"getLivePies()：";
    for (auto piePtr : getLivePies())
        wss << piePtr->chName() << L' ';
    wss << L"\ngetLivePies(PieceColor::red)：";
    for (auto piePtr : getLivePies(PieceColor::red))
        wss << piePtr->chName() << L' ';
    wss << L"\ngetLivePies(PieceColor::black)：";
    for (auto piePtr : getLivePies(PieceColor::black))
        wss << piePtr->chName() << L' ';
    wss << L"\ngetEatedPies()：";
    for (auto piePtr : getEatedPies())
        wss << piePtr->chName() << L' ';
    wss << L"\ngetNamePies(PieceColor::red, L'N')：\n";
    for (auto piePtr : getNamePies(PieceColor::red, L'马'))
        wss << piePtr->toString();

    wss << L"\ngetSeats(PieceColor::red)：\n";
    for (auto id : {0, 1, 2, 4, 6, 8, 10, 14, 18, 22, 27}) {
        auto p = piePtrs[id];
        wss << p->chName() << L':';
        for (auto s : p->getSeats(PieceColor::red))
            wss << s << L' ';
        wss << L"\n\n";
    }

    // wss << print_vector_int(getSeats(PieceColor::red));
    return wss.str();
}
