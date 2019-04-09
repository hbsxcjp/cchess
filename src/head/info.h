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
    const std::wstring toString() const;

private:
    std::shared_ptr<InfoRecord>& getInfoRecord(RecFormat fmt);
    
    std::map<std::wstring, std::wstring> info_;
    std::shared_ptr<InfoRecord> infoRecord_;
};

class InfoRecord {
public:
    InfoRecord() = default;
    virtual ~InfoRecord() = default;

    virtual RecFormat getRecFormat() const = 0;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) = 0;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const = 0;

protected:
    const std::wstring __readInfo_getInfoStr(std::ifstream& ifs);
    void __readInfo_ICCSZH(const std::wstring& moveStr, const RecFormat fmt);
    void __readInfo_CC(const std::wstring& moveStr);
    const std::wstring __getPGNInfo() const;
    const std::wstring __getPGNTxt_ICCSZH(const RecFormat fmt) const;
    const std::wstring __getPGNTxt_CC() const;
};

class XQFInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class PGN_ICCSInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class PGN_ZHInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class PGN_CCInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class BINInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class JSONInfoRecord : public InfoRecord {
public:
    virtual RecFormat getRecFormat();
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs);
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);

}