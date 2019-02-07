//#include "piece.h"
#include "move.h"
#include "board.h"
#include "info.h"

#include <iostream>
using std::endl;
using std::wcout;

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <regex>
#include <sstream>
using namespace std;

void Move::setNext(shared_ptr<Move> next)
{
    next_ptr = next;
    if (next) {
        next->stepNo = stepNo + 1; // 步数
        next->othCol = othCol; // 变着层数
        auto pre = make_shared<Move>();
        *pre = *this;
        next->setPrev(pre);
    }
}

void Move::setOther(shared_ptr<Move> other)
{
    other_ptr = other;
    if (other) {
        other->stepNo = stepNo; // 与premove的步数相同
        other->othCol = othCol + 1; // 变着层数
        auto pre = make_shared<Move>();
        *pre = *this;
        other->setPrev(pre);
    }
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
        << maxCol << L" ft:" << fromseat << L' ' << toseat << L" e:" << eatPie_ptr->chName() << L" I:" << ICCS << L" z:" << zh
        << L",r:" << remark << L">";
    return wss.str();
}

// Moves
Moves::Moves() { __clear(); }

Moves::Moves(wstring moveStr, Info& info, Board& board)
    : Moves()
{
    RecFormat fmt{ info.info[L"Format"] == L"zh" ? RecFormat::zh
                                                 : (info.info[L"Format"] == L"ICCS" ? RecFormat::ICCS
                                                                                    : RecFormat::CC) };
    if (fmt == RecFormat::JSON)
        fromJSON(moveStr);
    else {
        if (fmt == RecFormat::CC)
            fromCC(moveStr);
        else
            fromICCSZh(moveStr, fmt);
    }
    __initSetSeat(fmt, board);
    __initSetNum(board);
}

Moves::Moves(ifstream& ifs, vector<int>& Keys, vector<int>& F32Keys, Board& board)
    : Moves()
{
    fromXQF(ifs, Keys, F32Keys);
    __initSetSeat(RecFormat::XQF, board);
    __initSetNum(board);
}

inline PieceColor Moves::currentColor()
{
    return currentMove->stepNo % 2 == 0
        ? firstColor
        : (firstColor == PieceColor::red ? PieceColor::black
                                         : PieceColor::red);
}

inline vector<shared_ptr<Move>> Moves::getPrevMoves(shared_ptr<Move> move)
{
    vector<shared_ptr<Move>> res{ move };
    while (!move->prev())
        res.push_back(move->prev());
    std::reverse(res.begin(), res.end());
    return res;
}

// 基本走法
void Moves::forward(Board& board)
{
    if (currentMove->next()) {
        currentMove = currentMove->next();
        board.go(*currentMove);
    }
}

void Moves::backward(Board& board)
{
    if (currentMove->prev()) {
        board.back(*currentMove);
        currentMove = currentMove->prev();
    }
}

//'移动到当前节点的另一变着'
void Moves::forwardOther(Board& board)
{
    if (currentMove->other()) {
        shared_ptr<Move> toMove = currentMove->other();
        board.back(*currentMove);
        board.go(*toMove);
        currentMove = toMove;
    }
}

// 复合走法
void Moves::backwardTo(shared_ptr<Move> move, Board& board)
{
    while (move != currentMove) {
        board.back(*move);
        move = move->prev();
    }
}

void Moves::to(shared_ptr<Move> move, Board& board)
{
    if (move == currentMove)
        return;
    toFirst(board);
    for (auto& m : getPrevMoves(move))
        board.go(*m);
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

inline void Moves::cutNext() { currentMove->setNext(nullptr); }

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

const pair<int, int> Moves::getSeat__ICCS(wstring ICCS)
{
    return make_pair(getSeat(static_cast<int>(ColChars.find(ICCS[1])), static_cast<int>(ICCS[0])),
        getSeat(static_cast<int>(ColChars.find(ICCS[3])), static_cast<int>(ICCS[2])));
}

//(fseat, tseat)->中文纵线着法
const wstring Moves::getZh(int fseat, int tseat, Board& board) const
{
    wstringstream wss{};
    int fromRow{ getRow(fseat) }, fromCol{ getCol(fseat) };
    Piece* fromPiece{ board.getPiece(fseat) };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->chName() };
    bool isBottomSide{ board.isBottomSide(color) };
    vector<int> seats{ board.getSideNameColSeats(color, name, fromCol) };
    int length{ static_cast<int>(seats.size()) };
    auto __getChar = [&](int col) {
        return getNumChar(color, isBottomSide ? MaxCol - col : col);
    };

    if (length > 1 && find_char(Pieces::strongeNames, name)) {
        if (find_char(Pieces::pawnNames, name)) {
            seats = sortPawnSeats(isBottomSide,
                board.getSideNameSeats(color, name));
            length = seats.size();
        } else if (isBottomSide) //# '车', '马', '炮'
            reverse(seats.begin(), seats.end());
        wstring indexStr{ length == 2 ? L"前后" : (length == 3 ? L"前中后" : L"一二三四五") };
        wss << indexStr[find_index(seats, fseat)] << name;
    } else
        //#仕(士)和相(象)不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << __getChar(fromCol);

    int toRow{ getRow(tseat) };
    //wcout << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退')) << endl;

    wss << (toRow == fromRow ? L'平' : (isBottomSide == (toRow > fromRow) ? L'进' : L'退'))
        << (find_char(Pieces::lineNames, name) && toRow != fromRow
                   ? getNumChar(color, toRow > fromRow ? toRow - fromRow - 1 : fromRow - toRow - 1)
                   : __getChar(getCol(tseat)));
    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const pair<int, int> Moves::getSeat__Zh(wstring zhStr, Board& board) const
{
    int index, fseat, tseat;
    vector<int> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ find_char(getNumChars(PieceColor::red), zhStr[zhStr.size() - 1])
            ? PieceColor::red
            : PieceColor::black };
    bool isBottomSide = board.isBottomSide(color);
    wchar_t name{ zhStr[0] };
    auto __getNum = [&](wchar_t ch) {
        return static_cast<int>(getNumChars(color).find(ch)) + 1;
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
    fseat = seats[index];

    // '根据中文行走方向取得棋子的内部数据方向（进：1，退：-1，平：0）'
    int movDir{ getIndex(zhStr[2]) * (isBottomSide ? 1 : -1) },
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

void Moves::fromICCSZh(wstring moveStr, RecFormat fmt)
{
    wregex moveReg{ LR"((?:\d+\.)?\s+(["
            "帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平"
            "]{4}\b)(?:\s+\{([\s\S]*?)\})?)" };
    //# 走棋信息 (?:pattern)匹配pattern,但不获取匹配结果;  注解[\s\S]*?: 非贪婪
    auto setMoves = [&](shared_ptr<Move> move, wstring mvstr, bool isOther) { //# 非递归
        vector<pair<wstring, wstring>> mrStrs{};
        for (wsregex_iterator p(mvstr.begin(), mvstr.end(), moveReg);
             p != wsregex_iterator{}; ++p)
            mrStrs.push_back(make_pair((*p)[1], (*p)[2]));
        for (auto mr : mrStrs) {
            auto newMove = make_shared<Move>();
            newMove->zh = mr.first;
            newMove->remark = mr.second;
            if (isOther) { // # 第一步为变着
                move->setOther(newMove);
                isOther = false;
            } else
                move->setNext(newMove);
            move = newMove;
        }
        return move;
    };

    shared_ptr<Move> move;
    bool isOther{ false }; // 首次非变着
    vector<shared_ptr<Move>> othMoves{ rootMove };
    wregex rempat{ LR"(\{([\s\S]*?)\}\s*\d+\.\s+)" }, spleft{ LR"(\(\s*\d+\.)" }, spright{ LR"(\))" };
    wsregex_token_iterator wtleft{ moveStr.begin(), moveStr.end(), spleft, -1 }, end{};
    wsmatch res;
    if (regex_search((*wtleft).first, (*wtleft).second, res, rempat))
        rootMove->remark = res.str(1);
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
    }
}

void Moves::fromJSON(wstring moveJSON)
{
}

void Moves::fromCC(wstring moveStr)
{
}

void Moves::fromXQF(ifstream& ifs, vector<int>& Keys, vector<int>& F32Keys)
{
    int version{ Keys[0] }, KeyXYf{ Keys[1] }, KeyXYt{ Keys[2] }, KeyRMKSize{ Keys[3] };
    wcout << L"ver_XYf_XYt_Size: " << version << L'/'
          << KeyXYf << L'/' << KeyXYt << L'/' << KeyRMKSize << endl;

    function<void(Move&)> __readmove = [&](Move& move) {
        auto __byteToSeat = [&](int a, int b) {
            int xy = __subbyte(a, b);
            return getSeat(xy % 10, xy / 10);
        };
        auto __readbytes = [&](char* byteStr, int size) {
            int pos = ifs.tellg();
            ifs.read(byteStr, size);
            if (version > 10) // '字节串解密'
                for (int i = 0; i != size; ++i)
                    byteStr[i] = __subbyte((unsigned char)(byteStr[i]), F32Keys[(pos + i) % 32]);
        };
        auto __readremarksize = [&]() {
            char byteSize[4];
            __readbytes(byteSize, 4);
            int size{ *(int*)(unsigned char*)(byteSize) };
            return size - KeyRMKSize;
        };

        char data[4];
        __readbytes(data, 4);
        //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
        auto seats = make_pair(__byteToSeat(data[0], 0X18 + KeyXYf), __byteToSeat(data[1], 0X20 + KeyXYt));
        move.setSeat(seats);

        wcout << L"fs_ts: " << move.fseat() << L'/' << move.tseat() << endl;

        char ChildTag = data[2];
        int RemarkSize = 0;
        if (version <= 0x0A) {
            char b = 0;
            if (ChildTag & 0xF0)
                b = b | 0x80;
            if (ChildTag & 0x0F)
                b = b | 0x40;
            ChildTag = b;
            RemarkSize = __readremarksize();
        } else {
            ChildTag = ChildTag & 0xE0;
            if (ChildTag & 0x20)
                RemarkSize = __readremarksize();
        }
        if (RemarkSize > 0) { // # 如果有注解
            char rem[RemarkSize + 1];
            __readbytes(rem, RemarkSize);
            move.remark = s2ws(rem);

            wcout << L"remark: " << move.remark << L'/' << endl;
        }

        if (ChildTag & 0x80) { //# 有左子树
            move.setNext(make_shared<Move>());
            __readmove(*move.next());
        }
        if (ChildTag & 0x40) { // # 有右子树
            move.setOther(make_shared<Move>());
            __readmove(*move.other());
        }
    };

    //ifs.seekg(1024);
    __readmove(*rootMove);
}

// （rootMove）调用, 设置树节点的seat or zhStr'  // C++primer P512
void Moves::__initSetSeat(RecFormat fmt, Board& board)
{
    function<void(Move&)> __set = [&](Move& move) {
        switch (fmt) {
        case RecFormat::ICCS: {
            move.setSeat(getSeat__ICCS(move.ICCS));
            move.zh = getZh(move.fseat(), move.tseat(), board);
            break;
        }
        case RecFormat::zh:
        case RecFormat::CC: {
            move.setSeat(getSeat__Zh(move.zh, board));
            //*
            wstring zh{ getZh(move.fseat(), move.tseat(), board) };
            // wcout << move.toString() << L'\n';
            if (move.zh != zh) {
                wcout << L"move.zh: " << move.zh << L'\n'
                      << L"getZh( ): " << zh << L'\n'
                      << move.toString() << L'\n' << board.toString() << endl;
                return;
            } //*/
            move.ICCS = getICCS(move.fseat(), move.tseat());
            break;
        }
        case RecFormat::JSON:
        case RecFormat::XQF: {
            move.ICCS = getICCS(move.fseat(), move.tseat());
            move.zh = getZh(move.fseat(), move.tseat(), board);

            //*
            auto seats = getSeat__Zh(move.zh, board);
            // wcout << move.toString() << L'\n';
            if ((seats.first != move.fseat()) || (seats.second != move.tseat())) {
                wcout << L"move.fs_ts: " << move.fseat() << L' ' << move.tseat() << L'\n'
                      << L"getSeat__Zh( ): " << move.zh << L'\n'
                      << move.toString() << L'\n' << board.toString() << endl;
                return;
            } //*/

            break;
        }
        }

        board.go(move);
        if (move.next())
            __set(*move.next());
        board.back(move);
        if (move.other())
            __set(*move.other());
    };

    if (rootMove->next())
        __set(*rootMove->next()); // 驱动函数
    toFirst(board); // 复原board
}

void Moves::__initSetNum(Board& board)
{
    function<void(Move&)> __setNums = [&](Move& move) {
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
        board.go(move);
        if (move.next())
            __setNums(*move.next());
        board.back(move);
        if (move.other()) {
            maxCol += 1;
            __setNums(*move.other());
        }
    };

    if (rootMove->next())
        __setNums(*rootMove->next()); // # 驱动调用递归函数
    toFirst(board); // 复原board
}

void Moves::__clear()
{
    rootMove = make_shared<Move>();
    currentMove = rootMove;
    firstColor = PieceColor::red; // 棋局载入时需要设置此属性！
    movCount = remCount = remLenMax = othCol = maxRow = maxCol = 0;
}

wstring Moves::toString()
{
    wstringstream wss;
    function<void(Move&)> __remark = [&](Move& move) {
        if (move.remark.size() > 0)
            wss << L"\n{" << move.remark << L"}\n";
    };
    __remark(*rootMove);

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
    if (rootMove->next())
        __moveStr(*rootMove->next(), false);
    return wss.str();
}

wstring Moves::toLocaleString()
{
    wstringstream remstrs{};
    wstring lstr((maxCol + 1) * 5, L'　');
    vector<wstring> lineStr((maxRow + 1) * 2, lstr);
    function<void(Move&)> __setChar = [&](Move& move) {
        int firstcol = move.maxCol * 5;
        for (int i = 0; i < 4; ++i)
            lineStr[move.stepNo * 2][firstcol + i] = move.zh[i];
        if (move.remark.size())
            remstrs << L"(" << move.stepNo << L"," << move.maxCol << L"): {"
                    << move.remark << L"}\n";
        if (move.next()) {
            lineStr[move.stepNo * 2 + 1][firstcol + 2] = L'↓';
            __setChar(*move.next());
        }
        if (move.other()) {
            int linel = move.other()->maxCol * 5;
            for (int i = firstcol + 4; i < linel; ++i)
                lineStr[move.stepNo * 2][i] = L'…';
            __setChar(*move.other());
        }
    };
    __setChar(*rootMove);

    wstringstream wss{};
    wss << L"\n着法深度：" << maxRow << L", 变着广度：" << othCol
        << L", 视图宽度：" << maxCol << L", 着法数量：" << movCount
        << L", 注解数量：" << remCount << L", 注解最长：" << remLenMax << L"\n";
    lineStr[0][2] = L'开';
    lineStr[0][3] = L'始';
    for (auto line : lineStr)
        wss << line << L'\n';
    return wss.str() + remstrs.str();
}
