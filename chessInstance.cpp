#include "chessInstance.h"

#include <regex>
#include <fstream>
#include <sstream>
#include <utility>

using namespace std;
using namespace Board_base;

ChessInstance::ChessInstance(const char* filename)
{
    setFrom(filename);
}

void ChessInstance::setFrom(const char* filename)
{
    auto info_moves = getHead_Body(filename);
    info = Info(info_moves.first);
    board = Board(info.getFEN());
    RecFormat fmt = info.info[L"Format"] == L"zh" ? RecFormat::zh : RecFormat::ICCS;
    moves = Moves(info_moves.second, fmt, &board);
}

wstring ChessInstance::toString()
{
    return info.toString() + board.toString() + moves.toString();
}

wstring ChessInstance::toLocaleString()
{
    return info.toString() + board.toString() + moves.toString();
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

/*
vector<pair<wstring, wstring>> getMoves(wstring body) {
    vector<pair<wstring, wstring>> vws{};

    wregex pat{LR"((?:\d+\.)?\s+(["
            "帅仕相马车炮兵将士象卒一二三四五六七八九１２３４５６７８９前中后进退平"
            "]{4}\b)(?:\s+\{([\s\S]*?)\})?)"};
    // 获取无分支走法的全部着法
    auto move_rems = [&](wstring ws) {
        vector<pair<wstring, wstring>> vw{};
        for (wsregex_iterator p(ws.begin(), ws.end(), pat);
             p != wsregex_iterator{}; ++p)
            vw.push_back(make_pair((*p)[1], (*p)[2]));
        return vw;
    };

    auto left_rightStr = [&](wstring tempStr) {

    };

    wstring leftStr{body}, rightStr{};
    wstring::size_type pos;
    // wregex sp{LR"(\(\s*\d+\.)"};
    while (leftStr.size() > 0) {
        if ((pos = leftStr.find(L'(')) != wstring::npos) {
            rightStr = leftStr.substr(pos);
            leftStr.erase(pos);

            wcout << leftStr << L"\n\n" << rightStr;
        } else {
            vws = move_rems(leftStr);
        }
    }

    for (auto ss : vws)
        wcout << ss.first << L' ' << ss.second << L'\n';
    return vws;
}
*/

wstring ChessInstance::test()
{
    return toString();
}
