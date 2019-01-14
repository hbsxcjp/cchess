#include "board.h"
#include "move.h"

#include <iostream>

#include <algorithm>
#include <cctype>

using namespace Board_base;

vector<int> Board::getSideNameSeats(PieceColor color, wchar_t name)
{
    return __getSeats(pieces.getNamePies(color, name));
}

vector<int> Board::getSideNameColSeats(PieceColor color, wchar_t name, int col)
{
    return __getSeats(pieces.getNameColPies(color, name, col));
}

vector<int> Board::__getSeats(vector<Piece*> pies)
{
    vector<int> res{};
    for_each(pies.begin(), pies.end(), [&](Piece* p) { res.push_back(p->seat()); });
    return res;
}

//判断是否将军
bool Board::isKilled(PieceColor color)
{
    PieceColor othColor = color == PieceColor::black ? PieceColor::red : PieceColor::black;
    int kingSeat{ pieces.getKingPie(color)->seat() },
        othKingSeat{ pieces.getKingPie(othColor)->seat() };
    if (isSameCol(kingSeat, othKingSeat)) {
        vector<int> ss{ getSameColSeats(kingSeat, othKingSeat) };
        if (std::all_of(ss.begin(), ss.end(),
                [this](int s) { return isBlank(s); }))
            return true;
    }
    for (auto pie : pieces.getLiveStrongePies(othColor)) {
        auto ss = pie->getFilterMoveSeats(this);
        if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
            return true;
    }
    return false;
}

//判断是否被将死
bool Board::isDied(PieceColor color)
{
    for (auto pie : pieces.getLivePies(color))
        if (pie->getCanMoveSeats(this).size() > 0)
            return false;
    return true;
}

Piece* Board::go(Move* move)
{
    return move->setEatPiece(move_go(move->fseat(), move->tseat()));
}

void Board::back(Move* move)
{
    move_back(move->fseat(), move->tseat(), move->eatPiece());
}

Piece* Board::move_go(int fseat, int tseat)
{
    Piece* eatPiece = pieSeats[tseat];
    eatPiece->setSeat(nullSeat);
    __setPiece(getPiece(fseat), tseat);
    __setPiece(Pieces::nullPiePtr, fseat);
    return eatPiece;
}

void Board::move_back(int fseat, int tseat, Piece* eatPiece)
{
    __setPiece(getPiece(tseat), fseat);
    __setPiece(eatPiece, tseat);
}

void Board::__setPiece(Piece* pie, int tseat)
{
    pieSeats[tseat] = pie;
    pie->setSeat(tseat);
}

wstring Board::getFEN()
{
    wstring res{};
    return res;
}
/*
getFen(chessInstance) {
    let currentMove = this.currentMove;
    chessInstance.moves.toFirst(chessInstance.board);
    let fen = `${this.__fen()} ${chessInstance.moves.firstColor
=== base.BLACK ? 'b' : 'r'} - - 0 0`;
chessInstance.moves.goTo(currentMove, chessInstance.board);
    //assert this.info['FEN'] === fen,
'\n原始:{}\n生成:{}'.format(this.info['FEN'], fen) return fen;
}
*/

void Board::setFEN(wstring FEN)
{
    //'数字字符对应下划线字符串'
    map<wchar_t, wstring> num_lines{
        { L'9', L"_________" }, { L'8', L"________" }, { L'7', L"_______" },
        { L'6', L"______" }, { L'5', L"_____" }, { L'4', L"____" },
        { L'3', L"___" }, { L'2', L"__" }, { L'1', L"_" }
    };
    wstring chars_temp{}, chars{};
    FEN = L'/' + FEN;
    while (FEN.size()) {
        auto p = FEN.rfind(L'/');
        chars_temp.append(FEN, p + 1, 9);
        FEN.erase(p);
    }
    for (auto ch : chars_temp)
        chars += std::isdigit(ch) ? num_lines[ch] : wstring{ ch };
    __setPieces(chars);
}

void Board::__setPieces(wstring chars)
{
    __clearPieces();
    auto getPie = [&](wchar_t ch) {
        if (ch == Piece::nullChar)
            return pieces.nullPiePtr;
        for (auto p : pieces.getPies())
            if (p->wchar() == ch && p->seat() == nullSeat)
                return p;
        return pieces.nullPiePtr;
    };
    for (int s = 0; s < 90; ++s)
        __setPiece(getPie(chars[s]), s);
    bottomColor = pieces.getKingPie(PieceColor::red)->seat() < 45
        ? PieceColor::red
        : PieceColor::black;
}

void Board::__clearPieces()
{
    std::fill(pieSeats.begin(), pieSeats.end(), Pieces::nullPiePtr);
    auto ps = pieces.getPies();
    for_each(ps.begin(), ps.end(),
        [&](Piece* p) { p->setSeat(nullSeat); });
}

void Board::changeSide(ChangeType ct) {} // = ChangeType::exchange

/*
changeSide(chessInstance, changeType = 'exchange') {
    let __changeSeat = (transfun) => {
        //'根据transfun改置每个move的fseat,tseat'
        function __seat(move) {
            move.fseat = transfun(move.fseat);
            move.tseat = transfun(move.tseat);
            if (move.next)
                __seat(move.next);
            if (move.other)
                __seat(move.other);
        }

        if (this.rootMove.next)
            __seat(this.rootMove.next);
    } //# 驱动调用递归函数

    let currentMove = this.currentMove;
    chessInstance.moves.toFirst();
    let seatPieces;
    if (changeType === 'exchange') {
        //chessInstance.info.info['FEN'] 需要更改
        chessInstance.moves.firstColor =
chessInstance.moves.firstColor
=== base.BLACK ? base.RED : base.BLACK; seatPieces =
this.getLivePieces().map( p => [this.getSeat(piece),
chessInstance.pieces.getOthSidePiece(piece)]); } else { let
rotateSeat = (s)
=> Math.abs(s - 89); let symmetrySeat = (s) => (Math.floor(s /
9) + 1) * 9 - s % 9 - 1; let transfun = changeType === 'rotate' ?
rotateSeat : symmetrySeat;
        __changeSeat(transfun); // 转换move的seat值
        seatPieces = this.getLivePieces().map(p =>
[transfun(this.getSeat(piece)), piece]);
    }
    this.setSeatPieces(seatPieces);
    if (changeType != 'rotate')
        this.moves.setMoveInfo(this);
    if (currentMove !== this.rootMove) {
        this.moves.moveTo(currentMove);
    }
}

}
            */

wstring Board::__FEN()
{
    wstring res{};
    //'下划线字符串对应数字字符'
    vector<pair<wstring, wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    for (int row = MaxRow; row >= MinRow; --row) {
        for (int col = MinCol; col <= MaxCol; ++col)
            res += getPiece(getSeat(row, col))->wchar();
        if (row != MinRow)
            res += L'/';
    }
    wstring::size_type pos;
    for (auto l_n : line_nums)
        while ((pos = res.find(l_n.first)) != wstring::npos)
            res.replace(pos, l_n.first.size(), l_n.second);
    return res;
}

const wstring Board::toString()
{
    auto getName = [](Piece* p) {
        map<wchar_t, wchar_t> rcpName{
            { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' }
        };
        wchar_t name = p->chName();
        return (p->color() == PieceColor::black && rcpName.find(name) != rcpName.end())
            ? rcpName[name]
            : name;
    };
    wstring res{ Board_base::TextBlankBoard };
    for (auto p : pieces.getLivePies())
        res[(9 - getRow(p->seat())) * 2 * 18 + getCol(p->seat()) * 2] = getName(p);
    return res;
}

wstring Board::test_board()
{
    wstringstream wss{};
    wss << L"test "
           L"board.h\n----------------------------------------------------"
           L"-\n";
    for (auto fen :
        { L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5",
            L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" }) {
        wss << L"  fen  :" << fen << L'\n';
        setFEN(fen);
        wss << L"__FEN():" << __FEN() << L'\n';

        wss << L"Board::toString():\n"
            << toString();

        wss << L"Piece::getCanMoveSeats():\n";
        for (auto p : pieces.getLivePies()) {
            wss << p->chName() << L' ' << p->wchar() << L'_' << setw(2)
                << p->seat() << L'：';
            for (auto s : p->getCanMoveSeats(this))
                wss << setw(2) << s << L' ';
            wss << L'\n';
        }

        wss << pieces.toString();
        // wss << pieces.test_piece();

        wss << L"Board::getFEN():" << L'\n';
        wss << L"Board::changeSide():" << L'\n';
    }
    return wss.str();
}
