#ifndef BOARD_H
#define BOARD_H

#include "piece.h"

#include <set>
using std::set;

#include <utility>
using std::pair;

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
    Piece *getPiece(int seat) { return piePtrSeats[seat]; }
    bool isBlank(int seat) { return getPiece(seat)->isBlank(); }
    PieceColor getColor(int seat) { return getPiece(seat)->color(); }
    BoardSide getSide(PieceColor color) {
        return isBottomSide(color) ? BoardSide::bottom : BoardSide::top;
    }
    int getKingSeat(PieceColor color) { return pieces.getKingPie(color).seat; }


    //成员函数，类内声明，类外定义
    vector<Piece *> getLivePieces();
    vector<Piece *> getLivePieces(PieceColor color);
    vector<int> getNameSeats(PieceColor color, wchar_t name);
    vector<int> getNameColSeats(PieceColor color, wchar_t name, int col);
    vector<Piece *> getEatedPieces();

    /*

        isKilled(color) {
            let otherColor = color === base.BLACK ? base.RED :
    base.BLACK; let kingSeat = this.getKingSeat(color); let otherSeat =
    this.getKingSeat(otherColor); if (Board.isSameCol(kingSeat, otherSeat)) {
    // 将帅是否对面 if (every(getSameColSeats(kingSeat, otherSeat).map(s =>
        this.isBlank(seat)))) { return true;
                }
                for (let piece of this.getLiveColorPieces(otherColor)) {
                    if (piece.isStronge &&
        (piece.getMoveSeats(this).indexOf(kingSeat) >= 0)) { return
    true;
                    }
                }
            }
            return false;
        }

        isDied(color) {
            for (let piece of this.getLiveColorPieces(color)) {
                if (this.canMoveSeats(this.getSeat(piece))) {
                    return False;
                }
            }
            return True
        }

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

    // 静态函数，类内定义，内联函数
    static int getRow(int seat) { return seat / 9; }
    static int getCol(int seat) { return seat % 9; }
    static int getSeat(int row, int col) { return row * 9 + col; }
    static int rotateSeat(int seat) { return 89 - seat; }
    static int symmetrySeat(int seat) {
        return (getRow(seat) + 1) * 9 - seat % 9 - 1;
    }
    static bool isSameCol(int seat, int othseat) {
        return getCol(seat) == getCol(othseat);
    }

    //类内声明，类外定义
    static vector<int> getSameColSeats(int seat, int othseat);
    static vector<int> getKingMoveSeats(int seat);
    static vector<int> getAdvisorMoveSeats(int seat);
    // 获取移动、象心行列值
    static vector<pair<int, int>> getBishopMove_CenSeats(int seat);
    // 获取移动、马腿行列值
    static vector<pair<int, int>> getKnightMove_LegSeats(int seat);
    // 车炮可走的四个方向位置
    static vector<vector<int>> getRookCannonMoveSeat_Lines(int seat);
    static vector<int> getPawnMoveSeats(int seat);
    // '多兵排序'
    static vector<int> sortPawnSeats(bool isBottomSide, vector<int> pawnSeats);

    wstring getStaticValue();
    wstring getSeats();
    wstring getMoveSeats();

  private:
    // 棋盘数值常量
    static const int ColNum{9};
    static const int RowNum{10};
    static const int MinCol{0};
    static const int MaxCol{8};
    static const int MinRow{0};
    static const int MaxRow{9};

    // 棋盘位置相关组合: 类内声明，类外定义
    static const vector<int> allSeats;
    static const vector<int> bottomKingSeats;
    static const vector<int> topKingSeats;
    static const vector<int> bottomAdvisorSeats;
    static const vector<int> topAdvisorSeats;
    static const vector<int> bottomBishopSeats;
    static const vector<int> topBishopSeats;
    static const vector<int> bottomPawnSeats;
    static const vector<int> topPawnSeats;

    // 棋盘相关字符串: 类内声明，类外定义
    static const map<PieceColor, wstring> Side_ChNums;
    static const map<wchar_t, int> ChNum_Indexs;
    static const map<wchar_t, int> Direction_Nums;
    static const wstring FEN;
    static const wstring ColChars;
    static const wstring TextBlankBoard; // 文本空棋盘

    Pieces pieces;                     // 一副棋子类
    vector<Piece *> piePtrSeats; // 棋盘容器，顺序号即为位置seat
    PieceColor bottomPieColor;         // 底端棋子颜色
};                                     // Board class end.

wstring test_getRowCols();
wstring test_board();

#endif