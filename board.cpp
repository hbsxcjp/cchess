#include "board.h"
#include "board_base.h"


/*
// 车炮可走的四个方向位置
static getRookCannonMoveSeat_Lines(seat) {
    let seat_lines = [[], [], [], []];
    let row = Board_impl.getRow(seat); //this指类，而不是实例
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
    let row = Board_impl.getRow(seat);
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
        let col = Board_impl.getCol(seat);
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
seat = this.getSeat(piece); lineStr[(9 - Board_impl.getRow(seat)) *
2][Board_impl.getCol(seat) * 2] = __getname(piece);
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
    for (let seat of Board_impl.kingSeats()[this.getSide(color)]) {
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
Board_impl.getCol(s)
=== col);
}

getEatedPieces() {
    let livePieces = new Set(this.getLivePieces());
    return this.pies.filter(p => !livePieces.has(p));
}

isKilled(color) {
    let otherColor = color === base.BLACK ? base.RED :
base.BLACK; let kingSeat = this.getKingSeat(color); let otherSeat =
this.getKingSeat(otherColor); if (Board_impl.isSameCol(kingSeat, otherSeat)) {
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


//类内声明，类外定义
bool Board::isKilled(PieceColor color)
{
    return true;
}
bool Board::isDied(PieceColor color)
{

    return true;    
}
// '获取棋子可走的位置, 不能被将军'
vector<int> Board::canMoveSeats(int fseat)
{

    return (vector<int>{});
}


wstring test_board() {
    wstringstream wss{};
    wss << L"test "
           L"board_base.h\n-----------------------------------------------------\n";
    wss << Board_base::test_getStaticValue();
    wss << Board_base::test_getSeats();
    wss << Board_base::test_getMoveSeats();

    // wss << test_getRowCols();
    return wss.str();
}
