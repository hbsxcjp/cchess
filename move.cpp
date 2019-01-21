//#include "piece.h"
#include "move.h"
#include "board.h"
#include "info.h"

#include <iostream>
using std::endl;
using std::wcout;

#include <algorithm>
#include <iomanip>
#include <regex>
#include <sstream>
using namespace std;

void Move::setNext(Move* next)
{
    next->stepNo = stepNo + 1; // 步数
    next->othCol = othCol; // 变着层数
    next->setPrev(this);
    nt = next;
}

void Move::setOther(Move* other)
{
    other->stepNo = stepNo; // 与premove的步数相同
    other->othCol = othCol + 1; // 变着层数
    other->setPrev(prev());
    ot = other;
}

wstring Move::toJSON()
{
    wstring res{};
    return res;
} // JSON

wstring Move::toString()
{
    wstringstream wss{};
    wss << L"<rcm:" << stepNo << L' ' << othCol << L' '
        << maxCol << L" ft:" << fs << L' ' << ts << L" e:" << ep->chName() << L" I:" << ICCS << L" z:" << zh
        << L",r:" << remark << L">";
    return wss.str();
}

// Moves
void Moves::__clear()
{
    moves = {};
    rootMove = Move();
    currentMove = &rootMove;
    firstColor = PieceColor::red; // 棋局载入时需要设置此属性！
    movCount = remCount = remLenMax = othCol = maxRow = maxCol = 0;
}

Moves::Moves() { __clear(); }

Moves::Moves(wstring moveStr, RecFormat fmt, Board& board)
{
    __clear();
    setFrom(moveStr, fmt, board);
}

inline PieceColor Moves::currentColor()
{
    return currentMove->stepNo % 2 == 0
        ? firstColor
        : (firstColor == PieceColor::red ? PieceColor::black
                                         : PieceColor::red);
}

inline vector<Move*> Moves::getPrevMoves(Move* move)
{
    vector<Move*> res{ move };
    while ((move = move->prev()) != &rootMove)
        res.push_back(move);
    std::reverse(res.begin(), res.end());
    return res;
}

// 基本走法
void Moves::forward(Board& board)
{
    if (currentMove->next()) {
        currentMove = currentMove->next();
        board.go(currentMove);
    }
}

void Moves::backward(Board& board)
{
    if (currentMove->prev()) {
        board.back(currentMove);
        currentMove = currentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void Moves::forwardOther(Board& board)
{
    if (currentMove->other()) {
        Move* toMove = currentMove->other();
        board.back(currentMove);
        board.go(toMove);
        currentMove = toMove;
    }
}

// 复合走法
void Moves::backwardTo(Move* move, Board& board)
{
    while (move != currentMove) {
        board.back(move);
        move = move->prev();
    }
}

void Moves::to(Move* move, Board& board)
{
    if (move == currentMove)
        return;
    toFirst(board);
    for (auto& m : getPrevMoves(move))
        board.go(m);
    currentMove = move;
}

inline void Moves::toFirst(Board& board)
{
    while (!isStart())
        backward(board);
}

inline void Moves::toLast(Board& board)
{
    while (!isLast())
        forward(board);
}

inline void Moves::go(Board& board, int inc = 1)
{
    if (inc > 0)
        for (int i = 0; i != inc; ++i)
            forward(board);
    else
        for (int i = inc; i != 0; ++i)
            backward(board);
}

inline void Moves::cutNext() { ; } //currentMove->setNext(nullptr);

inline void Moves::cutOther()
{
    if (currentMove->other())
        currentMove->setOther(currentMove->other()->other());
}

const wstring Moves::getICCS(int fseat, int tseat)
{
    wstringstream wss{};
    wss << getCol(fseat) << ColChars[getRow(fseat)] << getCol(tseat)
        << ColChars[getRow(tseat)];
    return wss.str();
}

//(fseat, tseat)->中文纵线着法
const wstring Moves::getZh(int fseat, int tseat, Board& board) const
{
    wstring firstStr;
    int fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    Piece* fromPiece{ board.getPiece(fseat) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    bool isBottomSide{ board.isBottomSide(color) };
    vector<int> seats{ board.getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };
    auto __getColChar = [&](PieceColor color, int col) {
        return getColChar(color, isBottomSide ? MaxCol - col : col);
    };

    if (length > 1 && find_char(Pieces::strongeNames, name)) {
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide,
                board.getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide) //# '车', '马', '炮'
            seats = reverse(seats);
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        firstStr = indexStr[find_index(seats, fseat)] + name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        firstStr = L"" + name + __getColChar(color, fromCol);

    int toRow{ getRow(tseat) };
    wchar_t toChar{ toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退') };
    wchar_t toColChar;
    if (toRow == fromRow || !find_char(Pieces::lineNames, name))
        toColChar = __getColChar(color, getCol(tseat));
    else
        toColChar = getColChar(color, toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1);
    return firstStr + toChar + toColChar;
    // 断言已通过
    //this.setSeatFromZhStr(board);
    //if (fseat != this.fseat || tseat != this.tseat)
    //    console.log(board.toString(), fseat, tseat, '=>', this.zhStr, '=>',  this.fseat, this.tseat);
}

const pair<int, int> Moves::getSeat__ICCS(wstring ICCS, Board& board)
{
    return make_pair(getSeat(static_cast<int>(ColChars.find(ICCS[1])), static_cast<int>(ICCS[0])),
        getSeat(static_cast<int>(ColChars.find(ICCS[3])), static_cast<int>(ICCS[2])));
}

//中文纵线着法->(fseat, tseat)
const pair<int, int> Moves::getSeat__Zh(wstring zhStr, Board& board) const
{
    int index, fseat, tseat;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ find_char(getColChars(PieceColor::red), zhStr[zhStr.size() - 1])
            ? PieceColor::red
            : PieceColor::black };
    bool isBottomSide = board.isBottomSide(color);
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](wchar_t ch) {
        return static_cast<int>(getColChars(color).find(ch)) + 1;
    };
    auto __getCol = [&](int num) { return isBottomSide ? ColNum - num : num - 1; };
    if (find_char(Pieces::allNames, name)) {
        seats = board.getSideNameColSeats(color, name, __getCol(__getNum(zhStr[1])));
        if (seats.size() < 1)

            wcout << L"棋子列表少于1个:" << zhStr << L' ' << static_cast<int>(color) << name
                  << __getCol(__getNum(zhStr[1])) << L' ' << L'\n' << board.toString();

        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && find_char(Pieces::advbisNames, name) && (zhStr[2] == L'退') == isBottomSide) ? seats.size() - 1 : 0;
    } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = getIndex(zhStr[0]);
        name = zhStr[1];
        seats = board.getSideNameSeats(color, name);
        if (seats.size() < 2)
            wcout << L"棋子列表少于2个:" << zhStr << L' ' << name << L' ' << board.toString();
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide, seats);
            //#获取多兵的列
            if (seats.size() == 3 && zhStr[0] == L'后')
                index += 1;
        } else {
            if (isBottomSide) //# 修正index
                index = seats.size() - index - 1;
        }
    }
    sort(seats.begin(), seats.end(), [&](int a, int b) { return a < b; });
    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ getNum(zhStr[2]) * (isBottomSide ? 1 : -1) },
        num{ __getNum(zhStr[3]) }, toCol{ __getCol(num) };
    if (find_char(Pieces::lineNames, name)) {
        //#'获取直线走子toseat'
        int row = getRow(fseat);
        tseat = (movDir == 0) ? getSeat(row, toCol)
                              : (getSeat(row + movDir * num, getCol(fseat)));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step = toCol - getCol(fseat); //  # 相距1或2列
        if (step < 0)
            step *= -1;
        int inc = find_char(Pieces::advbisNames, name)
            ? step
            : (step == 1 ? 2 : 1);
        tseat = getSeat(getRow(fseat) + movDir * inc, toCol);
    }
    return make_pair(fseat, tseat);
}

void Moves::fromJSON(wstring moveJSON, Board& board) {}

void Moves::fromICCSZh(wstring moveStr, RecFormat fmt, Board& board)
{
    wregex moveReg{ LR"((?:\d+\.)?\s+(["
            "帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平"
            "]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    auto setMoves = [&](Move& thisMove, wstring mvstr, bool isOther) -> Move& { //# 非递归
        vector<pair<wstring, wstring>> mrStrs{};
        for (wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != wsregex_iterator{}; ++p)
            mrStrs.push_back(make_pair((*p)[1], (*p)[2]));
        Move* move{ &thisMove };
        for (auto mr : mrStrs) {
            Move newMove{ Move(nullSeat, nullSeat, mr.second) };
            newMove.zh = mr.first;
            moves.push_back(newMove);
            if (isOther) { // # 第一步为变着
                move->setOther(&moves.back()); // 不能用&newMove，它是临时变量，其地址不可使用！
                isOther = false;
            } else
                move->setNext(&moves.back());
            move = &moves.back();

            //wcout << move.toString() << endl;
        }
        return moves.back();
    };

    Move move;
    bool isOther{ false }; // 首次非变着
    vector<Move> othMoves{ rootMove };
    wregex spleft{ LR"(\(\s*\d+\.)" }, spright{ LR"(\))" };
    wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    for (; wtleft != end; ++wtleft) {
        wsregex_token_iterator wtright{ (*wtleft).first, (*wtleft).second, spright, -1 };
        for (; wtright != end; ++wtright) {
            move = setMoves(othMoves.back(), *wtright, isOther);
            if (!isOther)
                othMoves.pop_back();
            isOther = false;
        }
        othMoves.push_back(move);
        isOther = true;
        /*
        wstring mvStr = *wtleft;
        while (true) {
            Move& thisMove{ othMoves.back() };
            if (!isOther)
                othMoves.pop_back();
            auto pos = mvStr.find(L')');
            if (pos == wstring::npos) {
                othMoves.push_back(setMoves(thisMove, mvStr, isOther));
                isOther = true;
                break;
            } else {
                setMoves(thisMove, mvStr.substr(0, pos), isOther);
                mvStr = mvStr.substr(pos + 1);
                isOther = false;
            }
        }
        */
    }
    for (auto m : moves)
        wcout << m.toString() << endl;
}

void Moves::fromCC(wstring moveStr, Board& board)
{
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void Moves::__initSet(RecFormat fmt, Board& board)
{
    function<void(Move*)> __set = [&](Move* move) {
        if (fmt == RecFormat::ICCS) {
            move->setSeat(getSeat__ICCS(move->ICCS, board));
            move->zh = getZh(move->fseat(), move->tseat(), board);
        } else {
            move->setSeat(getSeat__Zh(move->zh, board));
            move->ICCS = getICCS(move->fseat(), move->tseat());
        }

        wcout << move->toString() << '\n';

        board.go(move);
        if (move->next())
            __set(move->next());
        board.back(move);
        if (move->other())
            __set(move->other());
    };

    if (rootMove.next())
        __set(rootMove.next()); // 驱动函数
}

void Moves::__initNums(Board& board)
{
    function<void(Move&)> setNums = [&](Move& move) {
        wcout << move.toString() << endl;

        movCount += 1;
        if (move.remark.size()) {
            remCount += 1;
            int length = move.remark.size();
            if (length > remLenMax)
                remLenMax = length;
        }
        move.maxCol = maxCol; // # 本着在视图中的列数
        if (move.othCol > othCol)
            othCol = move.othCol;
        if (move.stepNo > maxRow)
            maxRow = move.stepNo;
        board.go(&move);
        if (move.next())
            setNums(*move.next());
        board.back(&move);
        if (move.other()) {

            //wcout << move.toString() << endl;

            maxCol += 1;
            setNums(*move.other());
        }
    };

    if (moves[0].next() == nullptr)
        wcout << L"moves[0].next()==nullptr" << endl;

    if (rootMove.next())
        setNums(*rootMove.next());
    // # 驱动调用递归函数
}

void Moves::setFrom(wstring moveStr, RecFormat fmt, Board& board)
{
    __clear();
    if (fmt == RecFormat::JSON)
        fromJSON(moveStr, board);
    else {
        if (fmt == RecFormat::CC)
            fromCC(moveStr, board);
        else
            fromICCSZh(moveStr, fmt, board);
        //__initSet(fmt, board);
        /*
        if (fmt == RecFormat::ICCS)
            __initSet(rootMove->setSeat__ICCS, board);
        else
            __initSet(rootMove->setSeat__ZhStr, board);
            */
    }
    __initNums(board);
}

wstring Moves::toString()
{
    wstringstream wss;
    function<void(Move&)> __remark = [&](Move& move) {
        if (move.remark.size() != 0)
            wss << L"\n{" << move.remark << L"}\n";
    };
    __remark(rootMove);

    function<void(Move&, bool)> __moveStr = [&](Move& move, bool isOther) {
        int boutNum = (move.stepNo + 1) / 2;
        bool isEven = move.stepNo % 2 == 0;
        if (isOther)
            wss << L"(" << boutNum << L". " << (isEven ? L"... " : L"");
        else if (isEven)
            wss << L" ";
        else
            wss << boutNum << L". ";
        wss << move.zh << L' ';
        __remark(move);
        if (move.other()) {
            __moveStr(*move.other(), true);
            wss << L") ";
        }
        if (move.next())
            __moveStr(*move.next(), false);
    };

    // 驱动调用函数
    if (rootMove.next())
        __moveStr(*rootMove.next(), false);
    return wss.str();
}

wstring Moves::toLocaleString()
{
    wstringstream remstrs{};
    wstring lstr((maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((maxRow + 1) * 2, lstr);
    // rootMove->setZhStr(L"1.开始");

    function<void(Move&)> __setChar = [&](Move& move) {
        int firstcol = move.maxCol * 5;
        for (int i = 0; i < 4; ++i)
            // console.log(lineStr[move.stepNo * 2] || `${move.stepNo * 2}`);
            lineStr[move.stepNo * 2][firstcol + i] = move.zh[i];
        if (move.remark.size())
            remstrs << L"(" << move.stepNo << L"," << move.maxCol << L"): {"
                    << move.remark << L'\n';
        if (move.next()) {
            int row{ move.stepNo * 2 + 1 };
            lineStr[row][firstcol + 1] = L' ';
            lineStr[row][firstcol + 2] = L'↓';
            lineStr[move.stepNo * 2 + 1][firstcol + 2] = L' ';
            __setChar(*move.next());
        }
        if (move.other()) {
            int linel = move.other()->maxCol * 5;
            for (int i = firstcol + 4; i < linel; ++i)
                lineStr[move.stepNo * 2][i] = L'…';
            __setChar(*move.other());
        }
    };
    __setChar(rootMove);

    wstringstream wss{};
    wss << L"\n着法深度：" << maxRow << L", 变着广度：" << othCol
        << L", 视图宽度：" << maxCol << L"着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L'\n';
    for (auto line : lineStr)
        wss << line << L'\n';
    return wss.str() + remstrs.str();
}

wstring Moves::test()
{
    auto info_moves = getHead_Body("01.pgn");
    Info info = Info(info_moves.first);
    Board board = Board(info.getFEN());
    //RecFormat fmt = info.info[L"Format"] == L"zh" ? RecFormat::zh : RecFormat::ICCS;
    Moves moves(info_moves.second, RecFormat::zh, board);

    return moves.toLocaleString(); //moves.toString(); //L""; //  + board.toString();// +
    //return toString();
}