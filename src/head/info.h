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

    virtual bool is(RecFormat fmt) const = 0;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const = 0;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const = 0;
};

class XQFInfoRecord : public InfoRecord {
public:
    using InfoRecord::InfoRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;

    static unsigned char KeyXYf{}, KeyXYt{};
    static int KeyRMKSize{}, F32Keys[32]{};
};

class PGNInfoRecord : public InfoRecord {
public:
    using InfoRecord::InfoRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;

protected:
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const;
    //void __readInfo_CC(const std::wstring& moveStr);
    const std::wstring __getPGNInfo() const;
    //const std::wstring __getPGNTxt_ICCSZH(const RecFormat fmt) const;
    //const std::wstring __getPGNTxt_CC() const;
};

class BINInfoRecord : public InfoRecord {
public:
    using InfoRecord::InfoRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

class JSONInfoRecord : public InfoRecord {
public:
    using InfoRecord::InfoRecord;
    virtual bool is(RecFormat fmt) const;
    virtual std::map<std::wstring, std::wstring> read(std::ifstream& ifs) const;
    virtual void write(std::ofstream& ofs, std::map<std::wstring, std::wstring>& info) const;
};

const std::wstring getFEN(const std::wstring& pieceChars);
const std::wstring getPieceChars(const std::wstring& fen);
}