#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

// 棋盘端部
enum class BoardSide { bottom, top };


class Board {
  public:
    // Board(wstring pgn) {}
    Board() : pieces(Pieces()), bottomPieColor(PieceColor::red) {
        vector<const Piece *> piePtrSeats(90, &Piece::nullPie);
    }

    //成员函数，类内定义，内联函数
    bool isBottomSide(PieceColor color) { return bottomPieColor == color; }
    bool isBlank(int seat) { return getPiece(seat)->isBlank(); }
    Piece *getPiece(int seat) { return piePtrSeats[seat]; }
    PieceColor getColor(int seat) { return getPiece(seat)->color(); }
    BoardSide getSide(PieceColor color) {
        return isBottomSide(color) ? BoardSide::bottom : BoardSide::top;
    }

    //类内声明，类外定义
    bool isKilled(PieceColor color);
    bool isDied(PieceColor color);
    // '获取棋子可走的位置, 不能被将军'
    vector<int> canMoveSeats(int fseat);


    /*

        __go(move) {
            let fseat = move.fseat,
                tseat = move.tseat;
            move.eatPiece = this.piePtrSeats[tseat];
            this.piePtrSeats[tseat] = this.piePtrSeats[fseat];
            this.piePtrSeats[fseat] = null;
            return move.eatPiece;
        }

        __back(move) {
            let fseat = move.fseat,
                tseat = move.tseat;
            this.piePtrSeats[fseat] = this.piePtrSeats[tseat];
            this.piePtrSeats[tseat] = move.eatPiece;
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
                pieceChars = this.piePtrSeats.map(p => piece.char);
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
            this.piePtrSeats = (new Array(90)).fill(null);
            for (let [seat, piece] of seatPieces) {
                this.piePtrSeats[seat] = piece;
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

    const wstring toString();

private:
    Pieces pieces;                     // 一副棋子类
    vector<Piece *> piePtrSeats; // 棋盘容器，顺序号即为位置seat
    PieceColor bottomPieColor;         // 底端棋子颜色
};                                     // Board class end.

extern wstring test_getRowCols();
extern wstring test_board();

#endif