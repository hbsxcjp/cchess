//#include "piece.h"
#include "move.h"
#include "board.h"

#include <algorithm>
#include <regex>
#include <sstream>
using namespace std;

Move::Move(int fseat = nullSeat, int tseat = nullSeat, wstring aremark = L"")
    : remark{aremark}, fs{fseat}, ts{tseat} {}

void Move::setNext(Move *next) {
    next->stepNo = stepNo + 1; // 步数
    next->othCol = othCol;     // 变着层数
    next->setPrev(this);
    nt = next;
}

void Move::setOther(Move *other) {
    other->stepNo = stepNo;     // 与premove的步数相同
    other->othCol = othCol + 1; // 变着层数
    other->setPrev(prev());
    ot = other;
}

void Move::setSeat__ICCS(Board *board) {
    fs = getSeat(static_cast<int>(ColChars.find(da[1])),
                 static_cast<int>(da[0]));
    ts = getSeat(static_cast<int>(ColChars.find(da[3])),
                 static_cast<int>(da[2]));
}

void Move::setICCS() {
    wstringstream wss{};
    wss << getCol(fs) << ColChars[getRow(fs)] << getCol(ts)
        << ColChars[getRow(ts)];
    da = wss.str();
}

wstring Move::getStr(RecFormat fmt) {
    switch (fmt) {
    case RecFormat::ICCS:
        return da;
    case RecFormat::zh:
        return zh;
    default: // 留待以后添加其他的描述格式
        return zh;
    }
}

//根据中文纵线着法描述取得源、目标位置: (fseat, tseat)
void Move::setSeat__ZhStr(Board *board) {
    /*
    int index;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ find_char(Side_ChNums[PieceColor::red], zh[zh.size() - 1])
            ? PieceColor::red
            : PieceColor::black };
    bool isBottomSide = board->isBottomSide(color);
    wchar_t name{ zh[0] };
    auto __getNum = [&](wchar_t ch) {
        return static_cast<int>(Side_ChNums[color].find(ch)) + 1;
    };
    auto __getCol = [&](int num) { return isBottomSide ? 9 - num : num - 1; };
    if (find_char(Pieces::allNames, name)) {
        int col{ __getCol(__getNum(zh[1])) };
        seats = board->getSideNameColSeats(color, name, col);
        //# 排除：士、象同列时不分前后，以进、退区分棋子
        index = (seats.size() == 2 && find_char(Pieces::advbisNames, name) &&
    (zh[2] == L'退') == isBottomSide) ? seats.size() - 1 : 0; } else {
        //# 未获得棋子, 查找某个排序（前后中一二三四五）某方某个名称棋子
        index = ChNum_Indexs[zh[0]];
        name = zh[1];
        seats = board->getSideNameSeats(color, name);
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide, seats);
            //#获取多兵的列
            if (seats.size() == 3 && zh[0] == L'后')
                index += 1;
        } else {
            if (seats.size() < 2) {
                // console.log(`棋子列表少于2个 => ${zhStr} color:${color}name:
                // ${name}\n${this}`);
            }
            if (isBottomSide) //# 修正index
                index = seats.size() - index - 1;
        }
    }
    sort(seats.begin(), seats.end(), [&](int a, int b) { return a < b; });
    // if (seats.length === 0) console.log(`没有找到棋子 =>
    // ${zhStr}color:${color} name: ${name}\n${board}`);
    fs = seats[index];
    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ Direction_Nums[zh[2]] * (isBottomSide ? 1 : -1) },
        num{ __getNum(zh[3]) }, toCol{ __getCol(num) };
    if (find_char(Pieces::lineNames, name)) {
        //#'获取直线走子toseat'
        int row = getRow(fs);
        ts = (movDir == 0) ? getSeat(row, toCol)
                           : (getSeat(row + movDir * num, getCol(fs)));
    } else {
        //#'获取斜线走子：仕、相、马toseat'
        int step = toCol - getCol(fs); //  # 相距1或2列
        if (step < 0)
            step *= -1;
        int inc = find_char(Pieces::advbisNames, name)
            ? step
            : (step == 1 ? 2 : 1);
        ts = getSeat(getRow(fs) + movDir * inc, toCol);
        // console.log(this.tseat);
    }
    // 断言已通过
    // this.setZhStr(board);
    // if (zhStr != this.zhStr)
    //    console.log(board.toString(), zhStr, '=>', this.fseat,
    //    this.tseat,'=>', this.zhStr);

    */
}

// 根据源、目标位置: (fseat, tseat)取得中文纵线着法描述
void Move::setZhStr(Board *board) {
    /*
    wstring firstStr;
    int fromRow{ getRow(fs) }, fromCol{ getCol(fs) };
    Piece* fromPiece{ board->getPiece(fs) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    bool isBottomSide{ board->isBottomSide(color) };
    vector<int> seats{ board->getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };
    auto __getColChar = [&](PieceColor color, int col) {
        return Side_ChNums[color][isBottomSide ? MaxCol - col : col];
    };

    if (length > 1 && find_char(Pieces::strongeNames, name)) {
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide,
                board->getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide) //# '车', '马', '炮'
            seats = reverse(seats);
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" :
    L"一二三四五") }; firstStr = indexStr[find_index(seats, fs)] + name; } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        firstStr = L"" + name + __getColChar(color, fromCol);

    int toRow{ getRow(ts) };
    zh = firstStr + (toRow == fromRow ? L'平' : (isBottomSide == (toRow >
    fromRow) ? L'进' : L'退'))
        + (toRow == fromRow || !find_char(Pieces::lineNames, name) ?
    __getColChar(color, getCol(ts)) : (Side_ChNums[color][toRow > fromRow ?
    toRow - fromRow - 1 : fromRow - toRow - 1]));
    // 断言已通过
    //this.setSeatFromZhStr(board);
    //if (fseat != this.fseat || tseat != this.tseat)
    //    console.log(board.toString(), fseat, tseat, '=>', this.zhStr, '=>',
    this.fseat, this.tseat);

    */
}

void Move::fromJSON(wstring moveJSON, Board *board) {}
// （rootMove）调用
void Move::fromICCSZh(wstring moveStr, Board *board) {

    /*
    auto __setMoves = [&](Move* move, wstring mvstr, bool isOther) { //# 非递归
        Move* lastMove{ move };
        bool isFirst{ true };
        wsmatch mstr_remark;

        while (regex_search(mvstr, moverg)) {
            //(mstr_remark = regex_search(mvstr, moverg)
            Move newMove{ Move() };
            addMove(newMove);
            newMove.setZhStr(mstr_remark[1]);
            newMove.remark = mstr_remark[2].size() ? mstr_remark[2] : L"";
            if (isOther && isFirst) // # 第一步为变着
                lastMove->setOther(&newMove);
            else
                lastMove->setNext(&newMove);

            isFirst = false;
            lastMove = &newMove;
        }
        return lastMove;
    }
        //wregex moverg{R"(([\u4E00-\u9FA5]{4})(?:\s+\{([\s\S]*?)\})?)"};
        //console.log(moveStr);
        //let moverg = / ([\u4E00-\u9FA5]{4})(?:\s+\{([\s\S]*?)\})?/ugm;
        // 第一届“嘉宝杯”粤沪象棋对抗赛 - 上海胡荣华 (先和) 广东吕钦：
    出现“审形度势”的错误！

        wregex moverg{R"(([^\.\{\}\s/-]{4})(?:\s+\{([\s\S]*?)\})?)"};
        //let moverg = / ([^\.\{\}\s/-]{4})(?:\s+\{([\s\S]*?)\})?/gm; //
    插入:(?= )
        //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?:
    非贪婪 Move *thisMove; wstring leftStr; int index; vector<Move*>
    othMove{this}; bool isOther = false;

        let leftStrs = moveStr.split(/\(\d+\./gm);
        //# 如果注解里存在‘\(\d+\.’的情况，则可能会有误差

        wregex rightrg{L") "};
        while (leftStrs.length > 0) {
        thisMove = isOther ? othMoves[othMoves.length - 1] : othMoves.pop();
        index = leftStrs[0].indexOf(rightrg);
        if (index < 0) {
            //不存在'\) '的情况；# 如果注解里存在'\) '的情况，则可能会有误差
            othMoves.push(__setMoves(thisMove, leftStrs.shift(), isOther));
            isOther = true;
        } else {
            leftStr = leftStrs[0].slice(0, index);
            leftStrs[0] = leftStrs[0].slice(index + 2);
            __setMoves(thisMove, leftStr, isOther);
            isOther = false;
        }
        }
        */
}

void Move::fromCC(wstring moveStr, Board *board) {}

wstring Move::toJSON() {
    wstring res{};
    return res;
} // JSON

wstring Move::toString() {
    wstringstream wss{};
    wss << L"{stepNo:" << stepNo << L" othCol:" << othCol << L" maxCol:"
        << maxCol << L" fseat:" << fs << L" tseat:" << ts << L" zhStr:" << zh
        << L",remark:" << remark << L"}";
    return wss.str();
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void Move::initSet(function<void(Move *, Board *)> setFunc, Board *board) {
    function<void(Move *)> __set = [&](Move *move) {
        setFunc(move, board);
        board->go(move);
        if (move->next())
            __set(move->next());
        board->back(move);
        if (move->other())
            __set(move->other());
    };
    if (next())
        __set(next()); // 驱动函数
}

// Moves
inline PieceColor Moves::currentColor() {
    return currentMove->stepNo % 2 == 0
               ? firstColor
               : (firstColor == PieceColor::red ? PieceColor::black
                                                : PieceColor::red);
}

vector<Move *> Moves::getPrevMoves(Move *move) {
    vector<Move *> res{move};
    while ((move = move->prev()) != rootMove)
        res.insert(res.begin(), move);
    return res;
}

// 基本走法
void Moves::forward(Board *board) {
    if (currentMove->next()) {
        currentMove = currentMove->next();
        board->go(currentMove);
    }
}

void Moves::backward(Board *board) {
    if (currentMove->prev()) {
        board->back(currentMove);
        currentMove = currentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void Moves::forwardOther(Board *board) {
    if (currentMove->other()) {
        Move *toMove = currentMove->other();
        board->back(currentMove);
        board->go(toMove);
        currentMove = toMove;
    }
}

// 复合走法
void Moves::to(Move *move, Board *board) {
    if (move == currentMove)
        return;
    toFirst(board);
    for (auto &m : getPrevMoves(move))
        board->go(m);
    currentMove = move;
}

void Moves::toFirst(Board *board) {
    while (!isStart())
        backward(board);
}

void Moves::toLast(Board *board) {
    while (!isLast())
        forward(board);
}

void Moves::go(Board *board, int inc = 1) {
    if (inc > 0)
        for (int i = 0; i != inc; ++i)
            forward(board);
    else
        for (int i = inc; i != 0; ++i)
            backward(board);
}

// 添加着法，复合走法
void Moves::addMove(int fseat, int tseat, wstring remark, Board *board,
                    bool isOther = false) {
    Move move{Move(fseat, tseat, remark)};
    move.setZhStr(board);
    if (isOther) {
        currentMove->setOther(&move);
        forwardOther(board);
    } else {
        currentMove->setNext(&move);
        forward(board);
    }
    moves.push_back(move);
}

inline void Moves::cutNext() { currentMove->setNext(nullptr); }

inline void Moves::cutOther() {
    if (currentMove->other())
        currentMove->setOther(currentMove->other()->other());
}

void Moves::setFrom(wstring moveStr, Board *board, RecFormat fmt) {
    __clear();
    if (fmt == RecFormat::JSON)
        rootMove->fromJSON(moveStr, board);
    // rootMove->initSet(rootMove.setZhStr, board);
    else {
        if (fmt == RecFormat::CC)
            rootMove->fromCC(moveStr, board);
        else
            rootMove->fromICCSZh(moveStr, board);
        if (fmt == RecFormat::ICCS)
            rootMove->initSet(rootMove->setSeat__ICCS, board);
        else
            rootMove->initSet(rootMove->setSeat__ZhStr, board);
    }
    __initNums(board);
}

wstring Moves::toString() {
    wstringstream wss;
    function<void(Move *)> __remark = [&](Move *move) {
        if (move->remark.size() != 0)
            wss << L"\n{" << move->remark << L"}\n";
    };
    __remark(rootMove);
    function<void(Move *, bool)> __moveStr = [&](Move *move, bool isOther) {
        int boutNum = (move->stepNo + 1) / 2;
        bool isEven = move->stepNo % 2 == 0;
        if (isOther)
            wss << L"(" << boutNum << L". " << (isEven ? L"... " : L"");
        else {
            if (isEven)
                wss << L" ";
            else
                wss << boutNum << L". ";
        }
        wss << move->getStr(RecFormat::zh) << L' ';
        __remark(move);
        if (move->other()) {
            __moveStr(move->other(), true);
            wss << L") ";
        }
        if (move->next())
            __moveStr(move->next(), false);
    };

    // 驱动调用函数
    if (rootMove->next())
        __moveStr(rootMove->next(), false);
    return wss.str();
}

wstring Moves::toLocaleString() {
    wstringstream remstrs{};
    wstring lstr((maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((maxRow + 1) * 2, lstr);
    // rootMove->setZhStr(L"1.开始");

    function<void(Move *)> __setChar = [&](Move *move) {
        int firstcol = move->maxCol * 5;
        for (int i = 0; i < 4; ++i)
            // console.log(lineStr[move.stepNo * 2] || `${move.stepNo * 2}`);
            lineStr[move->stepNo * 2][firstcol + i] =
                move->getStr(RecFormat::zh)[i];
        if (move->remark.size())
            remstrs << L"(" << move->stepNo << L"," << move->maxCol << L"): {"
                    << move->remark << L'\n';
        if (move->next()) {
            int row{move->stepNo * 2 + 1};
            lineStr[row][firstcol + 1] = L' ';
            lineStr[row][firstcol + 2] = L'↓';
            lineStr[move->stepNo * 2 + 1][firstcol + 2] = L' ';
            __setChar(move->next());
        }
        if (move->other()) {
            int linel = move->other()->maxCol * 5;
            for (int i = firstcol + 4; i < linel; ++i)
                lineStr[move->stepNo * 2][i] = L'…';
            __setChar(move->other());
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

void Moves::__clear() {
    moves = {};
    rootMove = nullptr;
    currentMove = nullptr;
    firstColor = PieceColor::red; // 棋局载入时需要设置此属性！
    movCount = remCount = remLenMax = othCol = maxRow = maxCol = 0;
}

void Moves::__initNums(Board *board) {
    function<void(Move *)> setNums = [&](Move *move) {
        movCount += 1;
        if (move->remark.size()) {
            remCount += 1;
            int length = move->remark.size();
            if (length > remLenMax)
                remLenMax = length;
        }
        move->maxCol = maxCol; // # 本着在视图中的列数
        if (move->othCol > othCol)
            othCol = move->othCol;
        if (move->stepNo > maxRow)
            maxRow = move->stepNo;
        board->go(move);
        if (move->next())
            setNums(move->next());
        board->back(move);
        if (move->other()) {
            maxCol += 1;
            setNums(move->other());
        }
    };

    if (rootMove->next())
        setNums(rootMove->next());
    // # 驱动调用递归函数
}

wstring Moves::test_moves() {
    wstring ws{readTxt("01.pgn")};
    
    //writeTxt("out.pgn", ws);
    return ws;
}