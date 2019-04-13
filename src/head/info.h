#ifndef INFO_H
#define INFO_H

#include <map>
#include <memory>
#include <string>

enum class RecFormat;

namespace InfoSpace {

class Info {
public:
    Info() = default;

    void read(std::ifstream& ifs, RecFormat fmt);
    void write(std::ofstream& ofs, RecFormat fmt) const;
    void setFEN(const std::wstring& pieceChars);
    const std::wstring getPieceChars() const;
    const std::wstring toString() const;

private:
    void readXQF(std::ifstream& ifs);
    void writeXQF(std::ofstream& ofs) const;
    void readPGN(std::ifstream& ifs);
    void writePGN(std::ofstream& ofs) const;
    void readBIN(std::ifstream& ifs);
    void writeBIN(std::ofstream& ofs) const;
    void readJSON(std::ifstream& ifs);
    void writeJSON(std::ofstream& ofs) const;

    std::map<std::wstring, std::wstring> infoMap_;
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);

static unsigned char KeyXYf{}, KeyXYt{};
static int version{}, KeyRMKSize{}, F32Keys[32]{};
}