#include "board_impl.h"

#include <iostream>
using std::boolalpha;
using std::wcout;

#include <iomanip>
using std::setw;


#include <chrono>
using namespace std::chrono;

// Board(wstring pgn) {}

vector<int> Board::getSameColSeats(int seat, int othseat) {
    vector<int> seats{};
    if (!isSameCol(seat, othseat))
        return seats;

    int step = seat < othseat ? 9 : -9;
    // 定义比较函数
    auto compare = [step](int i, int j) -> bool {
        return step > 0 ? i < j : i > j;
    };

    for (int i = seat + step; compare(i, othseat); i += step) {
        seats.push_back(i);
    }
    return seats;
}

vector<int> Board::getKingMoveSeats(int seat) {
    int S{seat - 9}, W{seat - 1}, E{seat + 1}, N{seat + 9};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{E, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, E, N});
        else
            return (vector<int>{S, E});
    } else if (col == 4) {
        if (row == 0 || row == 7)
            return (vector<int>{W, E, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, W, E, N});
        else
            return (vector<int>{S, W, E});
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{W, N});
        else if (row == 1 || row == 8)
            return (vector<int>{S, W, N});
        else
            return (vector<int>{S, W});
    }
}

vector<int> Board::getAdvisorMoveSeats(int seat) {
    int WS{seat - 9 - 1}, ES{seat - 9 + 1}, WN{seat + 9 - 1}, EN{seat + 9 + 1};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == 3) {
        if (row == 0 || row == 7)
            return (vector<int>{EN});
        else
            return (vector<int>{ES});
    } else if (col == 4) {
        return (vector<int>{WS, ES, WN, EN});
    } else {
        if (row == 0 || row == 7)
            return (vector<int>{WN});
        else
            return (vector<int>{WS});
    }
}

// 获取移动、象心行列值
vector<pair<int, int>> Board::getBishopMove_CenSeats(int seat) {
    auto cen = [seat](int s) { return (seat + s) / 2; };

    int EN{seat + 2 * 9 + 2}, ES{seat - 2 * 9 + 2}, WS{seat - 2 * 9 - 2},
        WN{seat + 2 * 9 - 2};
    int row{getRow(seat)}, col{getCol(seat)};
    if (col == MaxCol)
        return (vector<pair<int, int>>{{WS, cen(WS)}, {WN, cen(WN)}});
    else if (col == MinCol)
        return (vector<pair<int, int>>{{EN, cen(EN)}, {ES, cen(ES)}});
    else if (row == 0 || row == 5)
        return (vector<pair<int, int>>{{WN, cen(WN)}, {EN, cen(EN)}});
    else if (row == 4 || row == 9)
        return (vector<pair<int, int>>{{WS, cen(WS)}, {ES, cen(ES)}});
    else
        return (vector<pair<int, int>>{
            {WS, cen(WS)}, {WN, cen(WN)}, {ES, cen(ES)}, {EN, cen(EN)}});
}

// 获取移动、马腿行列值
vector<pair<int, int>> Board::getKnightMove_LegSeats(int seat) {
    auto leg =
        [seat](int to) {
            int x = to - seat;
            if (x > 11)
                return seat + 9;
            else if (x < -11)
                return seat - 9;
            else if (x == 11 || x == -7)
                return seat + 1;
            else
                return seat - 1;
        };

    int EN{seat + 11},
         ES{seat - 7}, SE{seat - 17}, SW{seat - 19}, WS{seat - 11},
         WN{seat + 7}, NW{seat + 17}, NE{seat + 19};
    int row{getRow(seat)}, col{getCol(seat)};
    switch (col) {
    case MaxCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{{WS, leg(WS)}, {SW, leg(SW)}});

        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {WN, leg(WN)}});

        case MinRow:
            return (vector<pair<int, int>>{{WN, leg(WN)}, {NW, leg(NW)}});

        case MinRow + 1:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {WS, leg(WS)}});

        default:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {WS, leg(WS)}, {SW, leg(SW)}});
        }
    case MaxCol - 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}});

        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}, {WN, leg(WN)}});

        case MinRow:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {NE, leg(NE)}});

        case MinRow + 1:
            return (vector<pair<int, int>>{
                {WN, leg(WN)}, {NW, leg(NW)}, {NE, leg(NE)}, {WS, leg(WS)}});

        default:
            return (vector<pair<int, int>>{{WN, leg(WN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WS, leg(WS)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    case MinCol:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{{ES, leg(ES)}, {SE, leg(SE)}});

        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SE, leg(SE)}, {EN, leg(EN)}});

        case MinRow:
            return (vector<pair<int, int>>{{EN, leg(EN)}, {NE, leg(NE)}});

        case MinRow + 1:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NE, leg(NE)}, {ES, leg(ES)}});

        default:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NE, leg(NE)}, {ES, leg(ES)}, {SE, leg(SE)}});
        }
    case MinCol + 1:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SW, leg(SW)}, {SE, leg(SE)}});

        case MaxRow - 1:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {SW, leg(SW)}, {SE, leg(SE)}, {EN, leg(EN)}});

        case MinRow:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {NE, leg(NE)}});

        case MinRow + 1:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {NE, leg(NE)}, {ES, leg(ES)}});

        default:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {ES, leg(ES)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    default:
        switch (row) {
        case MaxRow:
            return (vector<pair<int, int>>{
                {ES, leg(ES)}, {WS, leg(WS)}, {SW, leg(SW)}, {SE, leg(SE)}});

        case MaxRow - 1:
            return (vector<pair<int, int>>{{ES, leg(ES)},
                                           {WS, leg(WS)},
                                           {WN, leg(WN)},
                                           {SW, leg(SW)},
                                           {SE, leg(SE)},
                                           {EN, leg(EN)}});

        case MinRow:
            return (vector<pair<int, int>>{
                {EN, leg(EN)}, {NW, leg(NW)}, {WN, leg(WN)}, {NE, leg(NE)}});

        case MinRow + 1:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WN, leg(WN)},
                                           {WS, leg(WS)},
                                           {ES, leg(ES)}});

        default:
            return (vector<pair<int, int>>{{EN, leg(EN)},
                                           {ES, leg(ES)},
                                           {NW, leg(NW)},
                                           {NE, leg(NE)},
                                           {WN, leg(WN)},
                                           {WS, leg(WS)},
                                           {SE, leg(SE)},
                                           {SW, leg(SW)}});
        }
    }
}


vector<Piece *> Board::getLivePieces() {
    vector<Piece *> res{};
    for (auto p: pieces.getPiePtrs()) 
        if (p->seat != Piece::nullSeat)
            res.push_back(p);
    return res;
}

vector<Piece *> Board::getLivePieces(PieceColor color) {
    vector<Piece *> res{};
    for (auto p: getLivePieces()) 
        if (p->color() == color)
            res.push_back(p);
    return res;
}

vector<int> Board::getNameSeats(PieceColor color, wchar_t name) {
    vector<int> res{};
    for (auto p : getLivePieces(color))
        if (p->chName() == name)
            res.push_back(p->seat);
    return res;
}

vector<int> Board::getNameColSeats(PieceColor color, wchar_t name, int col) {
    vector<int> res{};
    for (auto s : getNameSeats(color, name))
        if (getCol(s) == col)
            res.push_back(s);
    return res;
}

vector<Piece *> Board::getEatedPieces() {
    vector<Piece *> res{};
    for (auto p: pieces.getPiePtrs()) 
        if (p->seat == Piece::nullSeat)
            res.push_back(p);
    return res;
}

/*
// 车炮可走的四个方向位置
static getRookCannonMoveSeat_Lines(seat) {
    let seat_lines = [[], [], [], []];
    let row = Board.getRow(seat); //this指类，而不是实例
    let leftlimit = row * 9 - 1;
    let rightlimit = (row + 1) * 9;
    for (let i = seat - 1; i > leftlimit; i--) {
        seat_lines[0].push(i);
    }
    for (let i = seat + 1; i < rightlimit; i++) {
        seat_lines[1].push(i);
    }
    for (let i = seat - 9; i > -1; i -= 9) {
        seat_lines[2].push(i);
    }
    for (let i = seat + 9; i < 90; i += 9) {
        seat_lines[3].push(i);
    }
    return seat_lines;
}
*/
/*
vector<int> getPawnMoveSeats(int seat) {
    int E{seat + 1},
        S{seat - 9},
        W{seat - 1},
        N{seat + 9};
    int row{getRow(seat)}, col{getCol(seat)};
    switch (col) {
        case base.maxColNo:
            mvseats = [S, W, N];
            break;
        case base.MinColNo:
            mvseats = [E, S, N];
            break;
        default:
            mvseats = [E, S, W, N];
    }
    let row = Board.getRow(seat);
    if (row === 9 || row === 0) {
        seats = new Set([E, W]);
    }
    else {
        if (this.getSide(this.getColor(seat)) === 'bottom') {
            seats = new Set([E, W, N]);
        }
        else {
            seats = new Set([E, W, S]);
        }
    }
    return mvseats.filter(s => seats.has(s));
}


// '多兵排序'
static sortPawnSeats(isBottomSide, pawnSeats) {
    let temp = [],
        result = [];
    // 按列建立字典，按列排序
    pawnSeats.forEach(seat => {
        let col = Board.getCol(seat);
        if (temp[col]) {
            temp[col].push(seat);
        } else {
            temp[col] = [seat];
        }
    });
    // 筛除只有一个位置的列, 整合成一个数组
    temp.forEach(seats => {
        if (seats.length > 1) {
            result = result.concat(seats);
        }
    });
    // 根据棋盘顶底位置,是否反序
    return isBottomSide ? result.reverse() : result;
}

toString() {
    function __getname(piece) {
        let rcpName = { '车': '車', '马': '馬', '炮': '砲' };
        let name = piece.name;
        return (piece.color === base.BLACK && name in rcpName) ?
rcpName[name] : name;
    }

    let lineStr = base.BlankBoard.trim().split('\n').map(line =>
[...line.trim()]); for (let piece of this.getLivePieces()) { let
seat = this.getSeat(piece); lineStr[(9 - Board.getRow(seat)) *
2][Board.getCol(seat) * 2] = __getname(piece);
    }
    return `\n${lineStr.map(line =>
line.join('')).join('\n')}\n`;
}

isBottomSide(color) {
    return this.bottomPieColor === color;
}

isBlank(seat) {
    return Boolean(this.pies[seat]);
}

getSeat(piece) {
    return this.pies.indexOf(piece);
}

getPiece(seat) {
    return this.pies[seat];
}

getColor(seat) {
    return this.pies[seat].color;
}

getSide(color) {
    return this.isBottomSide(color) ? 'bottom' : 'top';
}

getKingSeat(color) {
    let piece;
    for (let seat of Board.kingSeats()[this.getSide(color)]) {
        piece = this.getPiece(seat);
        if (Boolean(piece) && piece.isKing) {
            return seat;
        }
    }
    console.log('出错了，在棋盘上没有找到将、帅！');
    console.log(this.toString());
}

getLivePieces() {
    return this.pies.filter(p => Boolean(p));
}

getLiveSidePieces(color) {
    return this.getLivePieces().filter(p => p.color === color);
}

getSideNameSeats(color, name) {
    return this.getLiveSidePieces(color).filter(
        p => p.name === name).map(p => this.getSeat(p));
}

getSideNameColSeats(color, name, col) {
    return this.getSideNameSeats(color, name).filter(s =>
Board.getCol(s)
=== col);
}

getEatedPieces() {
    let livePieces = new Set(this.getLivePieces());
    return this.pies.filter(p => !livePieces.has(p));
}

isKilled(color) {
    let otherColor = color === base.BLACK ? base.RED :
base.BLACK; let kingSeat = this.getKingSeat(color); let otherSeat =
this.getKingSeat(otherColor); if (Board.isSameCol(kingSeat, otherSeat)) {
// 将帅是否对面 if (every(getSameColSeats(kingSeat, otherSeat).map(s =>
this.isBlank(seat)))) { return true;
        }
        for (let piece of this.getLiveSidePieces(otherColor)) {
            if (piece.isStronge &&
(piece.getMoveSeats(this).indexOf(kingSeat) >= 0)) { return
true;
            }
        }
    }
    return false;
}

isDied(color) {
    for (let piece of this.getLiveSidePieces(color)) {
        if (this.canMoveSeats(this.getSeat(piece))) {
            return False;
        }
    }
    return True
}

__go(move) {
    let fseat = move.fseat,
        tseat = move.tseat;
    move.eatPiece = this.pies[tseat];
    this.pies[tseat] = this.pies[fseat];
    this.pies[fseat] = null;
    return move.eatPiece;
}

__back(move) {
    let fseat = move.fseat,
        tseat = move.tseat;
    this.pies[fseat] = this.pies[tseat];
    this.pies[tseat] = move.eatPiece;
}

// '获取棋子可走的位置, 不能被将军'
canMoveSeats(fseat) {
    let result = [];
    let piece = this.getPiece(fseat);
    let color = piece.color;
    let moveData;
    for (let tseat of piece.getMoveSeats(this)) {
        moveData = { fseat: fseat, tseat: tseat };
        moveData.eatPiece = this.__go(moveData);
        if (!this.isKilled(color)) {
            result.push(tseat);
        }
        this.__back(moveData);
    }
    return result
}

__fen(pieceChars = null) {
    function __linetonums() {
        //'下划线字符串对应数字字符元组 列表'
        let result = [];
        for (let i = 9; i > 0; i--) {
            result.push(['_'.repeat(i), String(i)]);
        }
        return result;
    }

    if (!pieceChars) {
        pieceChars = this.pies.map(p => piece.char);
    }
    let charls = [];
    for (let rowno = 0; rowno < base.NumRows; rowno++) {
        charls.push(pieceChars.slice(rowno * 9, (rowno + 1) *
9));
    }
    let fen = charls.reverse().map(chars =>
chars.join('')).join('/'); for (let [_str, nstr] of __linetonums()) {
fen = fen.replace(_str, nstr);
    }
    return fen;
}

getFen(chessInstance) {
    let currentMove = this.currentMove;
    chessInstance.moves.toFirst(chessInstance.board);
    let fen = `${this.__fen()} ${chessInstance.moves.firstColor
=== base.BLACK ? 'b' : 'r'} - - 0 0`;
chessInstance.moves.goTo(currentMove, chessInstance.board);
    //assert this.info['FEN'] === fen,
'\n原始:{}\n生成:{}'.format(this.info['FEN'], fen) return fen;
}

setSeatPieces(seatPieces) {
    this.pies = (new Array(90)).fill(null);
    for (let [seat, piece] of seatPieces) {
        this.pies[seat] = piece;
    }
    this.bottomPieColor = this.getKingSeat(base.RED) < 45 ?
base.RED : base.BLACK;
}

setFen(chessInstance) {
    function __numtolines() {
        //'数字字符: 下划线字符串'
        let numtolines = [];
        for (let i = 0; i < 10; i++)
            numtolines.push([String(i), '_'.repeat(i)]);
        return numtolines;
    }

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

            */

wstring Board::getStaticValue() {
    wstringstream wss{};
    wss << L"NumCols: ";
    wss << L"ColNum " << ColNum << L" RowNum " << RowNum << L" MinCol "
        << MinCol << L" MaxCol " << MaxCol << L'\n';

    wss << L"multiSeats:\n";
    // vector<const vector<int>> multiSeats = ;
    for (auto aseats : {allSeats, bottomKingSeats, topKingSeats,
                        bottomAdvisorSeats, topAdvisorSeats, bottomBishopSeats,
                        topBishopSeats, bottomPawnSeats, topPawnSeats}) {
        for (auto seat : aseats)
            wss << setw(2) << seat << L' ';
        wss << L'\n';
    }

    wss << L"Side_ChNums: ";
    for (auto colstr : Side_ChNums)
        wss << static_cast<int>(colstr.first) << L"-> " << colstr.second
            << L" ";

    wss << L"\nChNum_Indexs: ";
    for (auto chidx : ChNum_Indexs)
        wss << chidx.first << L"-> " << chidx.second << L" ";

    wss << L"\nDirection_Nums: ";
    for (auto dirnum : Direction_Nums)
        wss << dirnum.first << L"-> " << dirnum.second << L" ";

    wss << L"\nFEN: " << FEN << L"\nColChars: " << ColChars
        << L"\nTextBlankBoard: \n"
        << TextBlankBoard << L'\n';
    return wss.str();
}

wstring Board::getSeats() {
    wstringstream wss{};
    wss << boolalpha;
    wss << L"allSeats rotateSeat symmetrySeat isSameCol\n";
    for (auto s : allSeats)
        wss << setw(5) << s << setw(10) << rotateSeat(s) << setw(12)
            << symmetrySeat(s) << setw(14) << isSameCol(s, s + 8) << L'\n';
    wss << L"getSameColSeats:\n";

    vector<pair<int, int>> vp{{84, 21}, {86, 23}, {66, 13}};
    for (auto ss : vp) {
        for (auto s : getSameColSeats(ss.first, ss.second))
            wss << setw(3) << s << L' ';
        wss << L'\n';
    }
    return wss.str();
}

wstring Board::getMoveSeats() {
    wstringstream wss{};
    wss << L"getKingMoveSeats:\n";
    vector<vector<int>> testSeats = {bottomKingSeats, topKingSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getKingMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }

    wss << L"getAdvisorMoveSeats:\n";
    testSeats = {bottomAdvisorSeats, topAdvisorSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat : getAdvisorMoveSeats(seat))
                wss << L' ' << setw(3) << toseat;
            wss << L'\n';
        }

    wss << L"getBishopMove_CenSeats:\n";
    testSeats = {bottomBishopSeats, topBishopSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getBishopMove_CenSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }

    wss << L"getKnightMove_LegSeats:\n";
    testSeats = {allSeats};
    for (auto tseats : testSeats)
        for (auto seat : tseats) {
            wss << setw(3) << seat << L"->";
            for (auto toseat_cen : getKnightMove_LegSeats(seat))
                wss << L' ' << setw(3) << toseat_cen.first << L'_' << setw(2)
                    << toseat_cen.second;
            wss << L'\n';
        }


    return wss.str();
}

extern wstring test_getRowCols() {
    // 获取全部行列的seat值
    auto getSeats = []() {
        for (int row = 0; row != 10; ++row)
            for (int col = 0; col != 9; ++col)
                Board::getSeat(row, col);
    };
    // 获取全部seat值的行列
    auto getRowCols = []() {
        for (int seat = 0; seat != 90; ++seat) {
            Board::getRow(seat);
            Board::getCol(seat);
        }
    };

    wstringstream wss{};
    int count{10000};
    auto t0 = steady_clock::now();
    for (int i = 0; i != count; ++i) {
        getSeats();
        getRowCols();
    }
    auto d = steady_clock::now() - t0;
    wss << "getSeats: use time -> " << duration_cast<milliseconds>(d).count()
        << "ms" << L'\n';

    return wss.str();
}

extern wstring test_board() {
    Board board{};
    wstringstream wss{};
    wss << L"test "
           L"board.h\n-----------------------------------------------------\n";
    // wss << board.getStaticValue();
    // wss << board.getSeats();
    wss << board.getMoveSeats();

    // wss << test_getRowCols();
    return wss.str();
}
