#include "piece.h"
#include "board_base.h"


const wstring Piece::toString() {
    wstringstream wss{};
    wss << boolalpha;
    wss << setw(5) << index() << setw(4) << static_cast<int>(color()) << setw(5)
        << wchar() << setw(6) << chName() << setw(9) << isBlank() << setw(8)
        << isKing() << setw(8) << isStronge() << L'\n';
    return wss.str();
}


// 棋子可置放的全部位置
vector<int> Piece::getSeats(PieceColor bottomColor) {
    bool isBottom = color() == bottomColor;
    switch (tolower(ch)) {
    case L'k':
        return isBottom ? Board_base::bottomKingSeats : Board_base::topKingSeats;
    case L'a':
        return isBottom ?  Board_base::bottomAdvisorSeats : Board_base::topAdvisorSeats;
    case L'b':
        return isBottom ?  Board_base::bottomBishopSeats : Board_base::topBishopSeats;
    case L'p':
        return isBottom ?  Board_base::bottomPawnSeats : Board_base::topPawnSeats;
    default:
        return Board_base::allSeats;
    }
}

// 棋子可移动到的全部位置 // 筛除本方棋子占用的目标位置
vector<int> Piece::getCanMoveSeats() { // moveSeats, board
                                       // return moveSeats.filter(s =>
                                       // board.getColor(s) != this->_color);
    return (vector<int>{});
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
const int Piece::nullSeat{-1};
const Piece Piece::nullPie{Piece(L'_')};

// 一副棋子类
Pieces::Pieces() {
    wstring pieceChars{L"KAABBNNRRCCPPPPPkaabbnnrrccppppp"};
    for (auto wc : pieceChars)
        pies.push_back(Piece(wc));
}

vector<Piece *> Pieces::getPiePtrs() {
    vector<Piece *> res{};
    for (int i = 0; i != 32; ++i)
        res.push_back(&pies[i]);
    return res;
}

vector<Piece *> Pieces::getLivePieces() {
    vector<Piece *> res{};
    for (auto p : getPiePtrs())
        if (p->seat != Piece::nullSeat)
            res.push_back(p);
    return res;
}

vector<Piece *> Pieces::getLivePieces(PieceColor color) {
    vector<Piece *> res{};
    for (auto p : getLivePieces())
        if (p->color() == color)
            res.push_back(p);
    return res;
}

vector<int> Pieces::getNameSeats(PieceColor color, wchar_t name) {
    vector<int> res{};
    for (auto p : getLivePieces(color))
        if (p->chName() == name)
            res.push_back(p->seat);
    return res;
}
/*
vector<int> Pieces::getNameColSeats(PieceColor color, wchar_t name, int col) {
    vector<int> res{};
    for (auto s : getNameSeats(color, name))
        if (getCol(s) == col)
            res.push_back(s);
    return res;
}
*/
vector<Piece *> Pieces::getEatedPieces() {
    vector<Piece *> res{};
    for (auto p : getPiePtrs())
        if (p->seat == Piece::nullSeat)
            res.push_back(p);
    return res;
}

wstring Pieces::toString() {
    wstring ws{L"index color wchar chName isBlank isKing isStronge\n"};
    for (auto pie : pies)
        ws += pie.toString();
    return ws + (Piece{Piece::nullPie})
                    .toString(); // 关注空棋子的特性! 将const限定符取消
}

// Pieces seatPieces(vector<int, wchar_t> seatChars)    {    }

extern wstring test_piece() {
    Pieces pieces = Pieces();
    wstringstream wss{};
    wss << L"test "
           L"piece.h\n-----------------------------------------------------"
           L"\nPieces::toString:\n";
    wss << pieces.toString();

    wss << L"Piece::getKingPie\\Piece::chName：\n红棋->"
        << pieces.getKingPie(PieceColor::red).chName() << L' ' << L"黑棋->"
        << pieces.getKingPie(PieceColor::black).chName();

    wss << L"\nPieces::getOthSidePiece：\n";
    for (auto piePtr : pieces.getPiePtrs())
        wss << piePtr->chName() << L"->" << pieces.getOthPie(*piePtr).chName()
            << L' ';

    wss << L"\n特征棋子串：\n"
        << Pieces::kingNames << L' ' << Pieces::pawnNames << L' '
        << Pieces::advbisNames << L' ' << Pieces::strongeNames << L' '
        << Pieces::lineNames << L' ' << Pieces::allNames << L"\n\n";

    return wss.str();
}
