#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

class Board {
public:
    Board(wstring pgn){};
    Board(){};

    /*
    constructor() {
        this.pies = new Array(90);
        this.bottomSide = base.RED;
    }

    static allSeats() {
        return base.range(0, 90);
    }

    static kingSeats() {
        return {
            'bottom': [21, 22, 23, 12, 13, 14, 3, 4, 5],
            'top': [84, 85, 86, 75, 76, 77, 66, 67, 68]
        };
    }

    static advisorSeats() {
        return {
            'bottom': [21, 23, 13, 3, 5],
            'top': [84, 86, 76, 66, 68]
        };
    }

    static bishopSeats() {
        return {
            'bottom': [2, 6, 18, 22, 26, 38, 42],
            'top': [47, 51, 63, 67, 71, 83, 87]
        };
    }

    static pawnSeats() {
        return {
            'top': base.range(0, 45).concat([45, 47, 49, 51, 53, 54, 56, 58, 60, 62]),
            'bottom': base.range(45, 90).concat([27, 29, 31, 33, 35, 36, 38, 40, 42, 44])
        };
    }

    static getRow(seat) {
        return Math.floor(seat / 9);
    }

    static getCol(seat) {
        return seat % 9;
    }

    static getSeat(row, col) {
        return row * 9 + col;
    }

    static rotateSeat(seat) {
        return 89 - seat;
    }

    static symmetrySeat(seat) {
        return (Board.getRow(seat) + 1) * 9 - seat % 9 - 1;
    }

    static isSameCol(seat, othseat) {
        return Board.getCol(seat) === Board.getCol(othseat);
    }

    static getSameColSeats(seat, othseat) {
        let seats = new Array();
        let step = seat < othseat ? 9 : -9;

        function compare(i, j) {
            return step > 0 ? i < j : i > j;
        }; // 定义比较函数
        for (let i = seat + step; compare(i, othseat); i += step) {
            seats.push(i);
        }
        return seats;
    }

    static getKingMoveSeats(seat) {
        let E = seat + 1,
            S = seat - 9,
            W = seat - 1,
            N = seat + 9;
        let row = Board.getRow(seat);
        let col = Board.getCol(seat);
        let mvseats, seats;
        if (col === 4) {
            mvseats = [E, S, W, N];
        }
        else if (col === 3) {
            mvseats = [E, S, N];
        }
        else {
            mvseats = [W, S, N];
        }
        if (row === 0 || row === 7) {
            seats = new Set([E, W, N]);
        }
        else if (row === 2 || row === 9) {
            seats = new Set([S, W, N]);
        }
        else {
            seats = new Set([E, S, W, N]);
        }
        return mvseats.filter(s => seats.has(s));
    }

    static getAdvisorMoveSeats(seat) {
        let EN = seat + 9 + 1,
            ES = seat - 9 + 1,
            WS = seat - 9 - 1,
            WN = seat + 9 - 1;
        let row = Board.getRow(seat);
        let col = Board.getCol(seat);
        if (col === 4) {
            return [EN, ES, WS, WN];
        }
        else if (col === 3) {
            if (row === 0 || row === 7) {
                return [EN];
            }
            else {
                return [ES];
            }
        }
        else {
            if (row === 0 || row === 7) {
                return [WN];
            }
            else {
                return [WS];
            }
        }
    }

    // 获取移动、象心行列值
    static getBishopMove_CenSeats(seat) {
        let EN = seat + 2 * 9 + 2,
            ES = seat - 2 * 9 + 2,
            WS = seat - 2 * 9 - 2,
            WN = seat + 2 * 9 - 2;
        let row = Board.getRow(seat);
        let col = Board.getCol(seat);
        let mvseats;
        if (col === base.maxColNo) {
            mvseats = [WS, WN];
        }
        else if (col === base.MinColNo) {
            mvseats = [ES, EN];
        }
        else if (row === 0 || row === 5) {
            mvseats = [EN, WN];
        }
        else if (row === 4 || row === 9) {
            mvseats = [ES, WS];
        }
        else {
            mvseats = [EN, ES, WN, WS];
        }
        return mvseats.map(s => [s, (seat + s) / 2]);
    }

    // 获取移动、马腿行列值
    static getKnightMove_LegSeats(seat) {
        function _leg(first, to) {
            let x = to - first;
            if (x > 9 + 2) {
                return first + 9;
            }
            else if (x < -9 - 2) {
                return first - 9;
            }
            else if (x === 9 + 2 || x === -9 + 2) {
                return first + 1;
            }
            else {
                return first - 1;
            }
        }

        let EN = seat + 9 + 2,
            ES = seat - 9 + 2,
            SE = seat - 2 * 9 + 1,
            SW = seat - 2 * 9 - 1,
            WS = seat - 9 - 2,
            WN = seat + 9 - 2,
            NW = seat + 2 * 9 - 1,
            NE = seat + 2 * 9 + 1;
        let mvseats, seats;
        switch (Board.getCol(seat)) {
            case base.maxColNo:
                mvseats = [SW, WS, WN, NW];
                break;
            case base.maxColNo - 1:
                mvseats = [SE, SW, WS, WN, NW, NE];
                break;
            case base.MinColNo:
                mvseats = [EN, ES, SE, NE];
                break;
            case base.MinColNo + 1:
                mvseats = [EN, ES, SE, SW, NW, NE];
                break;
            default:
                mvseats = [EN, ES, SE, SW, WS, WN, NW, NE];
        }
        switch (Board.getRow(seat)) {
            case 9:
                seats = new Set([ES, SE, SW, WS]);
                break;
            case 8:
                seats = new Set([EN, ES, SE, SW, WS, WN]);
                break;
            case 0:
                seats = new Set([EN, WN, NW, NE]);
                break;
            case 1:
                seats = new Set([EN, ES, WS, WN, NW, NE]);
                break;
            default:
                seats = new Set([EN, ES, SE, SW, WS, WN, NW, NE]);
        }
        return mvseats.filter(s => seats.has(s)).map(s => [s, _leg(seat, s)]);
    }

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

    getPawnMoveSeats(seat) {
        let E = seat + 1,
            S = seat - 9,
            W = seat - 1,
            N = seat + 9;
        let mvseats, seats;
        switch (Board.getCol(seat)) {
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
            return (piece.color === base.BLACK && name in rcpName) ? rcpName[name] : name;
        }

        let lineStr = base.BlankBoard.trim().split('\n').map(line => [...line.trim()]);
        for (let piece of this.getLivePieces()) {
            let seat = this.getSeat(piece);
            lineStr[(9 - Board.getRow(seat)) * 2][Board.getCol(seat) * 2] = __getname(piece);
        }
        return `\n${lineStr.map(line => line.join('')).join('\n')}\n`;
    }

    isBottomSide(color) {
        return this.bottomSide === color;
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
        return this.getSideNameSeats(color, name).filter(s => Board.getCol(s) === col);
    }

    getEatedPieces() {
        let livePieces = new Set(this.getLivePieces());
        return this.pies.filter(p => !livePieces.has(p));
    }

    isKilled(color) {
        let otherColor = color === base.BLACK ? base.RED : base.BLACK;
        let kingSeat = this.getKingSeat(color);
        let otherSeat = this.getKingSeat(otherColor);
        if (Board.isSameCol(kingSeat, otherSeat)) {  // 将帅是否对面
            if (every(getSameColSeats(kingSeat, otherSeat).map(s => this.isBlank(seat)))) {
                return true;
            }
            for (let piece of this.getLiveSidePieces(otherColor)) {
                if (piece.isStronge && (piece.getMoveSeats(this).indexOf(kingSeat) >= 0)) {
                    return true;
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
            charls.push(pieceChars.slice(rowno * 9, (rowno + 1) * 9));
        }
        let fen = charls.reverse().map(chars => chars.join('')).join('/');
        for (let [_str, nstr] of __linetonums()) {
            fen = fen.replace(_str, nstr);
        }
        return fen;
    }

    getFen(chessInstance) {
        let currentMove = this.currentMove;
        chessInstance.moves.toFirst(chessInstance.board);
        let fen = `${this.__fen()} ${chessInstance.moves.firstColor === base.BLACK ? 'b' : 'r'} - - 0 0`;
        chessInstance.moves.goTo(currentMove, chessInstance.board);
        //assert this.info['FEN'] === fen, '\n原始:{}\n生成:{}'.format(this.info['FEN'], fen)
        return fen;
    }

    setSeatPieces(seatPieces) {
        this.pies = (new Array(90)).fill(null);
        for (let [seat, piece] of seatPieces) {
            this.pies[seat] = piece;
        }
        this.bottomSide = this.getKingSeat(base.RED) < 45 ? base.RED : base.BLACK;
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
                return 'len(charls) != 90 棋局的位置个数不等于90，有误！';
            let chars = charls.filter(c => c != '_');
            if (chars.length > 32)
                return 'chars.length > 32 全部的棋子个数大于32个，有误！';
            return false;
        }

        let fen = chessInstance.info.info['FEN'];
        let afens = fen.split(' ');
        fen = afens[0];
        let fenstr = fen.split('/').reverse().join('');
        let charls = base.multRepl(fenstr, __numtolines()).split('');
        let info = __isvalid(charls);
        if (info)
            console.log(info);
        let seatChars = []; //= charls.entries().map(c => );
        for (let i = 0; i < charls.length; i++) {
            seatChars.push([i, charls[i]]);
        }
        this.setSeatPieces(chessInstance.pieces.seatPieces(seatChars));
        chessInstance.moves.firstColor = afens[1] === 'b' ? base.BLACK : base.RED;
        this.currentMove = this.rootMove;
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
            chessInstance.moves.firstColor = chessInstance.moves.firstColor === base.BLACK ? base.RED : base.BLACK;
            seatPieces = this.getLivePieces().map(
                p => [this.getSeat(piece), chessInstance.pieces.getOthSidePiece(piece)]);
        } else {
            let rotateSeat = (s) => Math.abs(s - 89);
            let symmetrySeat = (s) => (Math.floor(s / 9) + 1) * 9 - s % 9 - 1;
            let transfun = changeType === 'rotate' ? rotateSeat : symmetrySeat;
            __changeSeat(transfun); // 转换move的seat值
            seatPieces = this.getLivePieces().map(p => [transfun(this.getSeat(piece)), piece]);
        }
        this.setSeatPieces(seatPieces);
        if (changeType != 'rotate')
            this.moves.setMoveInfo(this);
        if (currentMove !== this.rootMove) {
            this.moves.moveTo(currentMove);
        }
    }



*/
private:
    // 棋盘相关常量
    static const int ColNum{ 9 };
    static const int RowNum{ 10 };
    static const int MinCol{ 0 };
    static const int MaxCol{ 8 };
    static const wstring FEN;
    static const wstring ColChars;
    static const map<Side, wstring> Side_ChNums;
    static const map<wchar_t, int> ChNum_Indexs;
    static const map<wchar_t, int> Direction_Nums;
    static const wstring TextBlankBoard; // 文本空棋盘

    vector<Piece*> pieces;//(90); // 默认初值是nullptr
    Side bottomSide{ Side::red }; // 底端站队
};

// 棋盘相关字符串: 类内声明，类外定义
const wstring FEN{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1" };
const wstring ColChars{ L"abcdefghi" };
const map<Side, wstring> Side_ChNums{
    { Side::red, L"一二三四五六七八九" },
    { Side::black, L"１２３４５６７８９" }
};
const map<wchar_t, int> ChNum_Indexs{
    { L'一', 0 }, { L'二', 1 }, { L'三', 2 }, { L'四', 3 }, { L'五', 4 },
    { L'前', 0 }, { L'中', 1 }, { L'后', 1 }
};
const map<wchar_t, int> Direction_Nums{
    { L'进', 1 }, { L'退', -1 }, { L'平', 0 }
};

// 文本空棋盘
const wstring TextBlankBoard{
    L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
    "┃　│　│　│╲│╱│　│　│　┃\n"
    "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
    "┃　│　│　│╱│╲│　│　│　┃\n"
    "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
    "┃　│　│　│　│　│　│　│　┃\n"
    "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
    "┃　│　│　│　│　│　│　│　┃\n"
    "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
    "┃　　　　　　　　　　　　　　　┃\n"
    "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
    "┃　│　│　│　│　│　│　│　┃\n"
    "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
    "┃　│　│　│　│　│　│　│　┃\n"
    "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
    "┃　│　│　│╲│╱│　│　│　┃\n"
    "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
    "┃　│　│　│╱│╲│　│　│　┃\n"
    "┗━┷━┷━┷━┷━┷━┷━┷━┛\n"
}; // 边框粗线

#endif