#ifndef INFO_H
#define INFO_H

#include <map>
#include <memory>
#include <string>

enum class RecFormat;

namespace InfoSpace {

struct Key {
    int KeyXYf{}, KeyXYt{}, version{}, KeyRMKSize{}, F32Keys[32]{};
};
static Key key{};

class Info {
public:
    Info();

    void read(std::istream& is, RecFormat fmt);
    void write(std::ostream& os, RecFormat fmt) const;
    void setFEN(const std::wstring& pieceChars);
    const std::wstring getPieceChars() const;
    const std::wstring toString() const;

private:
    void readXQF(std::istream& is);
    void writeXQF(std::ostream& os) const;
    void readPGN(std::istream& is);
    void writePGN(std::ostream& os) const;
    void readBIN(std::istream& is);
    void writeBIN(std::ostream& os) const;
    void readJSON(std::istream& is);
    void writeJSON(std::ostream& os) const;

    std::map<std::wstring, std::wstring> infoMap_;
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);
}

#endif