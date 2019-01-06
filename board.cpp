//#include "board_base.h"
//#include "piece.h"
#include "board.h"

#include <algorithm>

using namespace Board_base;

Piece *Board::move_go(int fseat, int tseat) {
    auto eatPiece = __movePiece(getPiece(fseat), tseat);
    __movePiece(Pieces::nullPiePtr, fseat);
    return eatPiece;
}

void Board::move_back(int fseat, int tseat, Piece *eatPiece) {
    __movePiece(getPiece(tseat), fseat);
    __movePiece(eatPiece, tseat);
}

//判断是否将军
bool Board::isKilled(PieceColor color) {
    PieceColor othColor =
        color == PieceColor::black ? PieceColor::red : PieceColor::black;
    int kingSeat{pieces.getKingPie(color)->seat()},
        othKingSeat{pieces.getKingPie(othColor)->seat()};
    if (isSameCol(kingSeat, othKingSeat)) {
        vector<int> ss{getSameColSeats(kingSeat, othKingSeat)};
        if (std::all_of(ss.begin(), ss.end(),
                        [this](int s) { return isBlank(s); }))
            return true;
    }
    for (auto pie : pieces.getLiveStrongePies(othColor)) {
        auto ss = pie->getCanMoveSeats(this);
        if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
            return true;
    }
    return false;
}

//判断是否被将死
bool Board::isDied(PieceColor color) {
    for (auto pie : pieces.getLivePies(color))
        if (getCanMoveSeats(pie->seat()).size() > 0)
            return false;
    return true;
}

// '获取棋子可走的位置, 不能被将军'
vector<int> Board::getCanMoveSeats(int fseat) {
    vector<int> res{};
    auto pie = getPiece(fseat);
    auto color = pie->color();
    // 移动棋子后，检测是否会被对方将军
    for (auto tseat : pie->getCanMoveSeats(this)) {
        auto eatPiece = move_go(fseat, tseat);
        if (!isKilled(color))
            res.push_back(tseat);
        move_back(fseat, tseat, eatPiece);
    }
    return res;
}

inline void Board::__setPiece(Piece *pie, int tseat) {
    pieSeats[tseat] = pie;
    pie->setSeat(tseat);
}

inline Piece *Board::__movePiece(Piece *pie, int tseat) {
    Piece *eatPiece = pieSeats[tseat];
    eatPiece->setSeat(nullSeat);
    __setPiece(pie, tseat);
    return eatPiece;
}

wstring Board::getFEN() {
    wstring res{};
    return res;
}

void Board::setFEN(wstring FEN) {
    wstring fen{L'/' + FEN}, chars{};
    //'数字字符对应下划线字符串'
    map<wstring, wstring> num_lines{
        {L"9", L"_________"}, {L"8", L"________"}, {L"7", L"_______"},
        {L"6", L"______"},    {L"5", L"_____"},    {L"4", L"____"},
        {L"3", L"___"},       {L"2", L"__"},       {L"1", L"_"}};
    while (fen.size()) {
        auto p = fen.rfind(L'/');
        chars.append(fen, p + 1, 9);
        fen.erase(p);
    }
    for (auto n_l : num_lines)
        chars.replace(chars.find(n_l.first), 1, n_l.second);
    __setPieces(chars);
}

void Board::__setPieces(wstring chars) {
    std::fill(pieSeats.begin(), pieSeats.end(), Pieces::nullPiePtr);
    auto getPie = [&](wchar_t ch) {
        for (auto p : pieces.getPies())
            if (p->wchar() == ch && p->seat() != nullSeat)
                return p;
        return pieces.nullPiePtr;
    };
    for (int s = 0; s < 90; ++s)
        __setPiece(getPie(chars[s]), s);
    bottomColor = pieces.getKingPie(PieceColor::red)->seat() < 45
                      ? PieceColor::red
                      : PieceColor::black;
}

/*

setFen(chessInstance) {

    function __isvalid(charls) {
        '判断棋子列表是否有效'
        if (charls.length != 90)
            return 'len(charls) != 90
棋局的位置个数不等于90，有误！'; let chars = charls.filter(c => c !=
'_'); if (chars.length > 32) return 'chars.length > 32
全部的棋子个数大于32个，有误！'; return false;
    }

    let fen = chessInstance.info.info['FEN'];
    let afens = fen.split(' ');
    fen = afens[0];
    let fenstr = fen.split('/').reverse().join('');
    let charls = base.multRepl(fenstr,
__numtolines()).split(''); let info = __isvalid(charls); if (info)
        console.log(info);
    let seatChars = []; //= charls.entries().map(c => );
    for (let i = 0; i < charls.length; i++) {
        seatChars.push([i, charls[i]]);
    }
    this.setSeatPieces(chessInstance.pieces.seatPieces(seatChars));
    chessInstance.moves.firstColor = afens[1] === 'b' ?
base.BLACK : base.RED; this.currentMove = this.rootMove;
}

*/
void Board::changeSide(ChangeType ct) {} // = ChangeType::exchange

wstring Board::__FEN() {
    wstring res{};
    //'下划线字符串对应数字字符'
    map<wstring, wstring> line_nums{
        {L"_________", L"9"}, {L"________", L"8"}, {L"_______", L"7"},
        {L"______", L"6"},    {L"_____", L"5"},    {L"____", L"4"},
        {L"___", L"3"},       {L"__", L"2"},       {L"_", L"1"}};
    for (int row = MaxRow; row >= MinRow; --row) {
        for (int col = MinCol; col <= MaxCol; ++col)
            res += getPiece(getSeat(row, col))->wchar();
        if (row != MinRow)
            res += L'/';
    }
    for (auto l_n : line_nums)
        res.replace(res.find(l_n.first), l_n.first.size(), l_n.second);
    return res;
}


const wstring Board::toString() {
    auto getName = [](Piece *p) {
        map<wchar_t, wchar_t> rcpName{
            {L'车', L'車'}, {L'马', L'馬'}, {L'炮', L'砲'}};
        wchar_t name = p->chName();
        return (p->color() == PieceColor::black &&
                rcpName.find(name) != rcpName.end())
                   ? rcpName[name]
                   : name;
    };
    /*
    vector<wstring> lineStr;
    wstringstream res{L'\n'}, wss{Board_base::TextBlankBoard};
    for (wstring s; std::getline(wss, s);)
        lineStr.push_back(s);
    for (auto p : getLivePies())
        lineStr[(9 - getRow(p->seat())) * 2][getCol(p->seat()) * 2] =
            getName(p);
    for(auto s: lineStr)
        res << s << L'\n';
    return res.str();
    */
    wstring res{Board_base::TextBlankBoard};
    for (auto p : pieces.getLivePies())
        res[((9 - getRow(p->seat())) * 2 - 1) * 18 + getCol(p->seat()) * 2] =
            getName(p);
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

getSideNameColSeats(color, name, col) {
    return this.getSideNameSeats(color, name).filter(s =>
Board_impl.getCol(s)
=== col);
}

            */

wstring Board::test_board() {
    wstringstream wss{};
    wss << L"test "
           L"board.h\n----------------------------------------------------"
           L"-\n";
    setFEN(L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5");
    
    wss << toString();

    return wss.str();
}
