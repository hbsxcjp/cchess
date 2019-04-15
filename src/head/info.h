#ifndef INFO_H
#define INFO_H

#include <map>
#include <memory>
#include <string>
#include <vector>

enum class RecFormat;

namespace InfoSpace {

struct Key {
    unsigned char Version_XQF{}, KeyXYf{}, KeyXYt{}, F32Keys[32]{};
    int KeyRMKSize{};
};

class Info {
public:
    Info();

    void read(std::istream& is, RecFormat fmt);
    void write(std::ostream& os, RecFormat fmt) const;
    void setFEN(const std::wstring& pieceChars);
    const std::wstring getPieceChars() const;
    const Key& getKey() { return key_; }
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
    Key key_;
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);
}

#endif