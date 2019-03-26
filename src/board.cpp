#include "board.h"
#include "instance.h"
#include "move.h"
#include "piece.h"
#include "seat.h"
#include "tools.h"
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace BoardSpace {

//class BoardAide;

Board::Board()
    : bottomColor{ PieceColor::RED }
    , pieces_{ PieceSpace::creatPieces() }
    , seats_{ BoardAide::creatSeats() }
{
}

std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int row, const int col)
{
    return seats_[row * BoardAide::ColNum + col];
}

std::shared_ptr<SeatSpace::Seat>& Board::getSeat(const int rowcol)
{
    return seats_[rowcol / 10 * BoardAide::ColNum + rowcol % 10];
}

std::shared_ptr<SeatSpace::Seat>& Board::getOthSeat(const std::shared_ptr<SeatSpace::Seat>& seat,
    const ChangeType ct)
{
    if (ct == ChangeType::ROTATE) // 旋转
        return seats_[seats_.size() - distance(seats_.begin(), find(seats_.begin(), seats_.end(), seat)) - 1];
    else // ChangeType::SYMMETRY 对称
        return getSeat(seat->row(), BoardAide::ColNum - seat->col());
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getLiveSeats(const PieceColor color,
    const wchar_t name,
    const int col) const
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> someSeats{};
    for_each(seats_.begin(), seats_.end(), [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
        const std::shared_ptr<PieceSpace::Piece>& pie{ seat->piece() };
        if (pie->color() != PieceColor::BLANK // 活的棋子
            && (color == PieceColor::BLANK || color == pie->color()) // 空则两方棋子全选
            && (name == L'\x00' || name == pie->name()) // 空则各种棋子全选
            && (col == -1 || col == seat->col())) // -1则各列棋子全选
            someSeats.push_back(seat);
    });
    return someSeats;
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAllSeats()
{
    return seats_;
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getKingSeats(const PieceSpace::Piece& piece)
{
    bool isBottom{ isBottomSide(piece.color()) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpMidIndex },
        rowUp{ isBottom ? BoardAide::RowLowMidIndex : BoardAide::RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = BoardAide::ColMidLowIndex; col <= BoardAide::ColMidUpIndex; ++col)
            seats.push_back(getSeat(row, col));
    return seats;
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getAdvisorSeats(const PieceSpace::Piece& piece)
{
    bool isBottom{ isBottomSide(piece.color()) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpMidIndex },
        rowUp{ isBottom ? BoardAide::RowLowMidIndex : BoardAide::RowUpIndex }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = BoardAide::ColMidLowIndex; col <= BoardAide::ColMidUpIndex; ++col)
            if ((col + row) % 2 == rmd)
                seats.push_back(getSeat(row, col));
    return seats;
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getBishopSeats(const PieceSpace::Piece& piece)
{
    bool isBottom{ isBottomSide(piece.color()) };
    int rowLow{ isBottom ? BoardAide::RowLowIndex : BoardAide::RowUpLowIndex },
        rowUp{ isBottom ? BoardAide::RowLowUpIndex : BoardAide::RowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = BoardAide::ColMidLowIndex; col <= BoardAide::ColMidUpIndex; col += 2) {
            int gap{ row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                seats.push_back(getSeat(row, col));
        }
    return seats;
}

std::vector<std::shared_ptr<SeatSpace::Seat>> Board::getPawnSeats(const PieceSpace::Piece& piece)
{
    bool isBottom{ isBottomSide(piece.color()) };
    int lfrow{ isBottom ? BoardAide::RowLowUpIndex - 1 : BoardAide::RowUpLowIndex },
        ufrow{ isBottom ? BoardAide::RowLowUpIndex : BoardAide::RowUpLowIndex + 1 },
        ltrow{ isBottom ? BoardAide::RowUpLowIndex : BoardAide::RowLowIndex },
        utrow{ isBottom ? BoardAide::RowUpIndex : BoardAide::RowLowUpIndex };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int col = BoardAide::ColLowIndex; col <= BoardAide::ColUpIndex; col += 2)
        for (int row = lfrow; row <= ufrow; ++row)
            seats.push_back(getSeat(row, col));
    for (int col = BoardAide::ColLowIndex; col <= BoardAide::ColUpIndex; ++col)
        for (int row = ltrow; row <= utrow; ++row)
            seats.push_back(getSeat(row, col));
    return seats;
}

//判断是否将军
const bool Board::isKilled(const PieceColor color)
{
    /*
  PieceColor othColor = color == PieceColor::BLACK ? PieceColor::RED :
  PieceColor::BLACK; int kingSeat{ pPieces->getKingPie(color)->seat() },
      othKingSeat{ pPieces->getKingPie(othColor)->seat() };
  if (isSameCol(kingSeat, othKingSeat)) {
      std::vector<std::shared_ptr<SeatSpace::Seat>> ss{ getSameColSeats(kingSeat, othKingSeat) };
      if (std::all_of(ss.begin(), ss.end(),
              [this](const int s) { return isBlank(s); }))
          return true;
  }
  for (auto& ppie : pPieces->getLiveStrongePies(othColor)) {
      auto ss = ppie->filterMoveSeats(*this);
      if (std::find(ss.begin(), ss.end(), kingSeat) != ss.end())
          return true;
  }
  */
    return false;
}

//判断是否被将死
const bool Board::isDied(const PieceColor color)
{
    /*
  for (auto& ppie : pPieces->getLivePies(color))
      if (ppie->getCanMoveSeats(*this).size() > 0)
          return false;
          */
    return true;
}

const std::wstring Board::getFEN(const std::wstring& pieceChars) const
{
    //'下划线字符串对应数字字符'
    std::vector<std::pair<std::wstring, std::wstring>> line_nums{
        { L"_________", L"9" }, { L"________", L"8" }, { L"_______", L"7" },
        { L"______", L"6" }, { L"_____", L"5" }, { L"____", L"4" },
        { L"___", L"3" }, { L"__", L"2" }, { L"_", L"1" }
    };
    std::wstring fen{};
    for (int i = 81; i >= 0; i -= 9)
        fen += pieceChars.substr(i, 9) + L"/";
    fen.erase(fen.size() - 1, 1);
    std::wstring::size_type pos;
    for (auto& linenum : line_nums)
        while ((pos = fen.find(linenum.first)) != std::wstring::npos)
            fen.replace(pos, linenum.first.size(), linenum.second);
    return fen;
}

void Board::putPieces(const std::wstring& fen)
{
    std::wstring chars{ __getChars(fen) };
    if (seats_.size() != chars.size())
        std::cout << "错误：seats_.size() != chars.size()" << seats_.size() << "=" << chars.size() << "=" << fen.size() << std::endl;

    wchar_t ch{};
    int chIndex{ -1 };
    std::vector<bool> used(pieces_.size(), false);
    for (auto& seat : seats_) {
        if ((ch = chars[++chIndex]) != PieceSpace::nullChar) {
            int pieIndex{ -1 };
            for (auto& pie : pieces_)
                if (!used[++pieIndex] && pie->ch() == ch) {
                    seat->put(pie);
                    used[pieIndex] = true;
                    break;
                }
        } else
            seat->put(PieceSpace::nullPiece);
    }
    setBottomSide();
}

const std::wstring Board::__getChars(const std::wstring& fen) const
{
    std::wstring chars{};
    std::wregex sp{ LR"(/)" };
    for (std::wsregex_token_iterator wti{ fen.begin(), fen.end(), sp, -1 }; wti != std::wsregex_token_iterator{}; ++wti) {
        std::wstringstream line{};
        for (const auto& wch : std::wstring{ *wti })
            line << (isdigit(wch) ? std::wstring(wch - 48, PieceSpace::nullChar) : std::wstring{ wch }); // ASCII: 0:48
        chars.insert(0, line.str());
    }
    return chars;
}

void Board::setBottomSide()
{
    for (auto& seat : seats_) {
        auto pie = seat->piece();
        if (PieceSpace::isKing(pie->name())) {
            bottomColor = pie->color();
            break;
        }
    }
}

const std::wstring Board::changeSide(const ChangeType ct)
{
    std::wstringstream wss{};
    if (ct == ChangeType::EXCHANGE) // 交换红黑方
        for_each(seats_.begin(), seats_.end(), [&](std::shared_ptr<SeatSpace::Seat>& seat) {
            seat->put(seat->piece() == PieceSpace::nullPiece
                    ? PieceSpace::nullPiece
                    : pieces_[(distance(pieces_.begin(),
                                   find(pieces_.begin(), pieces_.end(),
                                       seat->piece()))
                                  + 16)
                          % 32]);
        });
    else // 旋转或对称
        for_each(seats_.begin(), seats_.end(), [&](std::shared_ptr<SeatSpace::Seat>& seat) {
            seat->put(getOthSeat(seat, ct)->piece());
        });
    setBottomSide();
    for_each(seats_.begin(), seats_.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) { wss << seat->piece()->ch(); });
    return wss.str();
}

const std::wstring Board::getIccs(const MoveSpace::Move& move) const
{
    std::wstringstream wss{};
    std::wstring ColChars{ L"abcdefghi" };
    wss << ColChars[move.fseat()->col()] << move.fseat()->row()
        << ColChars[move.tseat()->col()] << move.tseat()->row();
    return wss.str();
}

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::getMoveSeats(const MoveSpace::Move& move, const RecFormat fmt)
{
    if (fmt == RecFormat::ICCS)
        return __getSeatFromICCS(move.iccs());
    else // case RecFormat::ZH: //case RecFormat::CC:
        return __getSeatFromZh(move.zh());
}

const std::wstring Board::toString() const
{
    // 文本空棋盘
    std::wstring textBlankBoard{ L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
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
                                 "┗━┷━┷━┷━┷━┷━┷━┷━┛\n" }; // 边框粗线
    std::map<wchar_t, wchar_t> rcpName{ { L'车', L'車' }, { L'马', L'馬' }, { L'炮', L'砲' } };
    auto __getName = [&](const PieceSpace::Piece& pie) {
        wchar_t name = pie.name();
        return ((pie.color() == PieceColor::BLACK && rcpName.find(name) != rcpName.end())
                ? rcpName[name]
                : name);
    };
    for (auto& seat : getLiveSeats(PieceColor::BLANK))
        textBlankBoard[(BoardAide::ColNum - seat->row()) * 2 * (BoardAide::ColNum * 2) + seat->col() * 2] = __getName(*seat->piece());
    return textBlankBoard;
}

const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>>
Board::__getSeatFromICCS(const std::wstring& ICCS)
{
    std::string iccs{ Tools::ws2s(ICCS) };
    return make_pair(getSeat(iccs[1] - 48, iccs[0] - 97),
        getSeat(iccs[3] - 48, iccs[2] - 97)); // 0:48, a:97
}

// '多兵排序'
const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::__sortPawnSeats(const PieceColor color,
    const wchar_t name)
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ getLiveSeats(color, name) }; // 最多5个兵
    // 按列建立字典，按列排序
    std::map<int, std::vector<std::shared_ptr<SeatSpace::Seat>>> colSeats{};
    for_each(seats.begin(), seats.end(),
        [&](const std::shared_ptr<SeatSpace::Seat>& seat) {
            colSeats[seat->col()].push_back(seat);
        }); // 底边则列倒序,每列位置倒序

    // 整合成一个数组
    auto pos = seats.begin();
    for (auto& colSeat : colSeats)
        if (colSeat.second.size() > 1) // 筛除只有一个位置的列
            pos = copy(colSeat.second.begin(), colSeat.second.end(), pos); //按列存入
    return move(std::vector<std::shared_ptr<SeatSpace::Seat>>{ seats.begin(), pos });
}

//(fseat, tseat)->中文纵线着法
const std::wstring Board::getZh(const MoveSpace::Move& move)
{
    std::wstringstream wss{};
    const std::shared_ptr<SeatSpace::Seat>&fseat{ move.fseat() }, &tseat{ move.tseat() };
    const std::shared_ptr<PieceSpace::Piece>& fromPiece{ fseat->piece() };
    const PieceColor color{ fromPiece->color() };
    const wchar_t name{ fromPiece->name() };
    const int fromRow{ fseat->row() }, fromCol{ fseat->col() }, toRow{ tseat->row() },
        toCol{ tseat->col() };
    bool isSameRow{ fromRow == toRow }, isBottom{ isBottomSide(color) };
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{ getLiveSeats(color, name, fromCol) };

    if (seats.size() > 1 && PieceSpace::isStronge(name)) {
        if (PieceSpace::isPawn(name))
            seats = __sortPawnSeats(color, name);
        int index = distance(seats.begin(), find(seats.begin(), seats.end(), fseat));
        wss << BoardAide::getIndexChar(seats.size(), isBottom, index) << name;
    } else //将帅 , 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wss << name << BoardAide::getColChar(color, isBottom, fromCol);

    wss << BoardAide::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (PieceSpace::isLineMove(fromPiece->name()) && !isSameRow
                   ? BoardAide::getNumChar(color, abs(fromRow - toRow))
                   : BoardAide::getColChar(color, isBottom, toCol));

    //*
    auto mvSeats = __getSeatFromZh(wss.str());
    if (mvSeats.first != fseat || mvSeats.second != tseat) {
        std::wcout << L"fseat:" << fseat->toString() << L"tseat:" << tseat->toString()
                   << L'\n' << L"mvSeats.first:" << mvSeats.first->toString()
                   << L"mvSeats.second:" << mvSeats.second->toString() << std::endl;
    }
    //*/

    return wss.str();
}

//中文纵线着法->(fseat, tseat)
const std::pair<const std::shared_ptr<SeatSpace::Seat>, const std::shared_ptr<SeatSpace::Seat>> Board::__getSeatFromZh(const std::wstring& zhStr)
{
    std::shared_ptr<SeatSpace::Seat> fseat{}, tseat{};
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ BoardAide::getColor(zhStr.back()) };
    bool isBottom{ isBottomSide(color) };
    int index{}, movDir{ BoardAide::getMovDir(isBottom, zhStr.at(2)) };
    wchar_t name{ zhStr.front() };

    if (PieceSpace::isPieceName(name)) { // 首字符为棋子名
        seats = getLiveSeats(color, name, BoardAide::getCol(isBottom, BoardAide::getNum(color, zhStr.at(1))));
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && (isBottom == (movDir == -1))) ? 1 : 0; // && PieceSpace::isAdvBish(name) 可不用
    } else {
        name = zhStr.at(1);
        seats = PieceSpace::isPawn(name) ? __sortPawnSeats(color, name) : getLiveSeats(color, name);
        index = BoardAide::getIndex(seats.size(), isBottom, zhStr.front());
    }
    fseat = seats.at(index);

    int num{ BoardAide::getNum(color, zhStr.back()) }, toCol{ BoardAide::getCol(isBottom, num) };
    if (PieceSpace::isLineMove(name))
        tseat = movDir == 0 ? getSeat(fseat->row(), toCol) : getSeat(fseat->row() + movDir * num, fseat->col());
    else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - fseat->col()) }; //  相距1或2列
        tseat = getSeat(fseat->row() + movDir * (PieceSpace::isAdvBish(name) ? colAway : (colAway == 1 ? 2 : 1)),
            toCol);
    }

    /*
    MoveSpace::Move mv{};
    mv.setSeats(fseat, tseat);
    if (zhStr != getZh(mv))
        std::wcout << L"zhStr:" << zhStr << L" getZh(mv):" << getZh(mv) << std::endl;
    //*/
    return make_pair(fseat, tseat);
}

const std::wstring Board::test() // const
{
    std::wstringstream wss{};
    // Piece test
    wss << std::setw(4) << "color" << std::setw(6) << "char" << std::setw(5) << "name" << std::setw(8)
        << "isKing" << std::setw(8) << "isPawn" << std::setw(8) << "Stronge" << std::setw(8)
        << "Line\n";
    for (auto& pie : pieces_)
        wss << pie->toString() << L'\n';
    wss << PieceSpace::nullPiece->toString() << L'\n';

    // Board test
    std::wstring fen{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
    // std::wstring fen{ L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" };
    std::wstring chars{ __getChars(fen) };
    wss << fen << L'\n' << chars << L'\n';
    putPieces(fen);

    wss << toString();
    return wss.str();
}

const std::vector<std::shared_ptr<SeatSpace::Seat>> Board::BoardAide::creatSeats()
{
    std::vector<std::shared_ptr<SeatSpace::Seat>> seats{};
    for (int row = 0; row < RowNum; ++row)
        for (int col = 0; col < ColNum; ++col)
            seats.push_back(std::make_shared<SeatSpace::Seat>(row, col, PieceSpace::nullPiece));
    return seats;
}

const wchar_t Board::BoardAide::getChar(const PieceColor color, const int index) { return numChars.at(color)[index]; };

const std::wstring Board::BoardAide::getPreChars(const int length)
{
    switch (length) {
    case 2:
        return L"前后";
    case 3:
        return L"前中后";
    default:
        return L"一二三四五";
    }
}

const PieceColor Board::BoardAide::getColor(const wchar_t numZh) { return numChars.at(PieceColor::RED).find(numZh) != std::wstring::npos ? PieceColor::RED : PieceColor::BLACK; }
const wchar_t Board::BoardAide::getIndexChar(const int length, const bool isBottom, const int index) { return getPreChars(length)[isBottom ? length - index - 1 : index]; }
const wchar_t Board::BoardAide::getMovChar(const bool isSameRow, bool isBottom, bool isForward) { return movChars.at(isSameRow ? 1 : (isBottom == isForward ? 2 : 0)); }
const wchar_t Board::BoardAide::getNumChar(const PieceColor color, const int num) { return getChar(color, num - 1); };
const wchar_t Board::BoardAide::getColChar(const PieceColor color, bool isBottom, const int col) { return getChar(color, isBottom ? ColNum - col - 1 : col); };
const int Board::BoardAide::getIndex(const int seatsLen, const bool isBottom, const wchar_t preChar)
{
    int index{ static_cast<int>(getPreChars(seatsLen).find(preChar)) };
    return isBottom ? seatsLen - index - 1 : index;
}
const int Board::BoardAide::getMovDir(const bool isBottom, const wchar_t movChar) { return static_cast<int>(movChars.find(movChar) - 1) * (isBottom ? 1 : -1); }
const int Board::BoardAide::getNum(const PieceColor color, const wchar_t numChar) { return static_cast<int>(numChars.at(color).find(numChar)) + 1; }
const int Board::BoardAide::getCol(bool isBottom, const int num) { return isBottom ? ColNum - num : num - 1; }

const int Board::BoardAide::RowNum{ 10 };
const int Board::BoardAide::RowLowIndex{ 0 };
const int Board::BoardAide::RowLowMidIndex{ 2 };
const int Board::BoardAide::RowLowUpIndex{ 4 };
const int Board::BoardAide::RowUpLowIndex{ 5 };
const int Board::BoardAide::RowUpMidIndex{ 7 };
const int Board::BoardAide::RowUpIndex{ 9 };
const int Board::BoardAide::ColNum{ 9 };
const int Board::BoardAide::ColLowIndex{ 0 };
const int Board::BoardAide::ColMidLowIndex{ 3 };
const int Board::BoardAide::ColMidUpIndex{ 5 };
const int Board::BoardAide::ColUpIndex{ 8 };
const std::wstring Board::BoardAide::movChars{ L"退平进" };
const std::map<PieceColor, std::wstring> Board::BoardAide::numChars{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
}