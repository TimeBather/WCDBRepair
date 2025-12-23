#include "WCDBCpp.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

struct Options {
    std::string command;
    std::string dbPath;
    bool showProgress = true;

    bool hasKey = false;
    std::vector<unsigned char> keyBytes;
    int cipherPageSize = 4096;
    WCDB::Database::CipherVersion cipherVersion = WCDB::Database::CipherVersion::DefaultVersion;
};

static void printUsage()
{
    std::fprintf(stderr,
                 "WCDB 修复工具 (Windows)\n"
                 "\n"
                 "用法:\n"
                 "  wcdb-repair check  <dbPath>\n"
                 "  wcdb-repair backup <dbPath>\n"
                 "  wcdb-repair repair <dbPath> [--key-hex <hex>] [--cipher-page-size <n>] [--cipher-version <default|1|2|3|4>] [--no-progress]\n"
                 "  wcdb-repair deposit <dbPath>\n"
                 "  wcdb-repair contains-deposited <dbPath>\n"
                 "  wcdb-repair remove-deposited <dbPath>\n"
                 "\n"
                 "说明:\n"
                 "  - repair 会调用 WCDB 的 Database::retrieve() 进行修复。\n"
                 "  - 若数据库为加密库，可使用 --key-hex 指定密钥（十六进制）。\n");
}

static bool isHexChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hexVal(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    return -1;
}

static bool parseHex(const std::string& hex, std::vector<unsigned char>& out)
{
    std::string s;
    s.reserve(hex.size());
    for (char c : hex) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            continue;
        if (!isHexChar(c))
            return false;
        s.push_back(c);
    }
    if ((s.size() % 2) != 0)
        return false;

    out.clear();
    out.reserve(s.size() / 2);
    for (size_t i = 0; i < s.size(); i += 2) {
        int hi = hexVal(s[i]);
        int lo = hexVal(s[i + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out.push_back(static_cast<unsigned char>((hi << 4) | lo));
    }
    return true;
}

static bool parseInt(const std::string& s, int& out)
{
    if (s.empty())
        return false;
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    if (end == nullptr || *end != '\0')
        return false;
    if (v < 0 || v > 1'000'000'000L)
        return false;
    out = static_cast<int>(v);
    return true;
}

static bool parseCipherVersion(const std::string& s, WCDB::Database::CipherVersion& out)
{
    if (s == "default") {
        out = WCDB::Database::CipherVersion::DefaultVersion;
        return true;
    }
    if (s == "1") {
        out = WCDB::Database::CipherVersion::Version1;
        return true;
    }
    if (s == "2") {
        out = WCDB::Database::CipherVersion::Version2;
        return true;
    }
    if (s == "3") {
        out = WCDB::Database::CipherVersion::Version3;
        return true;
    }
    if (s == "4") {
        out = WCDB::Database::CipherVersion::Version4;
        return true;
    }
    return false;
}

static bool parseArgs(const std::vector<std::string>& argv, Options& opt)
{
    if (argv.size() < 2)
        return false;

    const std::string& cmd = argv[1];
    if (cmd == "-h" || cmd == "--help" || cmd == "help") {
        opt.command = "help";
        return true;
    }

    if (argv.size() < 3)
        return false;

    opt.command = cmd;
    opt.dbPath = argv[2];

    for (size_t i = 3; i < argv.size(); i++) {
        const std::string& a = argv[i];
        if (a == "--no-progress") {
            opt.showProgress = false;
            continue;
        }
        if (a == "--key-hex") {
            if (i + 1 >= argv.size())
                return false;
            std::vector<unsigned char> bytes;
            if (!parseHex(argv[i + 1], bytes))
                return false;
            opt.hasKey = true;
            opt.keyBytes = std::move(bytes);
            i++;
            continue;
        }
        if (a == "--cipher-page-size") {
            if (i + 1 >= argv.size())
                return false;
            int v = 0;
            if (!parseInt(argv[i + 1], v))
                return false;
            opt.cipherPageSize = v;
            i++;
            continue;
        }
        if (a == "--cipher-version") {
            if (i + 1 >= argv.size())
                return false;
            WCDB::Database::CipherVersion v;
            if (!parseCipherVersion(argv[i + 1], v))
                return false;
            opt.cipherVersion = v;
            i++;
            continue;
        }
        return false;
    }

    return true;
}

static void applyCipherIfNeeded(WCDB::Database& db, const Options& opt)
{
    if (!opt.hasKey)
        return;
    const WCDB::UnsafeData key = WCDB::UnsafeData::immutable(opt.keyBytes.data(), opt.keyBytes.size());
    db.setCipherKey(key, opt.cipherPageSize, opt.cipherVersion);
}

#if defined(_WIN32)
static std::string utf8FromWide(const wchar_t* w)
{
    if (w == nullptr)
        return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0)
        return {};
    // len includes the trailing '\0' when cchWideChar == -1
    std::string out;
    out.resize(static_cast<size_t>(len), '\0');
    // NOTE: In C++14, std::string::data() returns const char*, so use &out[0].
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], len, nullptr, nullptr);
    if (!out.empty()) {
        out.pop_back(); // remove trailing '\0'
    }
    return out;
}
#endif

static int run(const std::vector<std::string>& argv)
{
    Options opt;
    if (!parseArgs(argv, opt) || opt.command.empty()) {
        printUsage();
        return 2;
    }

    if (opt.command == "help") {
        printUsage();
        return 0;
    }

    WCDB::Database db(opt.dbPath);
    applyCipherIfNeeded(db, opt);

    if (opt.command == "check") {
        bool corrupted = db.checkIfCorrupted();
        std::printf("corrupted=%s\n", corrupted ? "true" : "false");
        return corrupted ? 1 : 0;
    }

    if (opt.command == "backup") {
        bool ok = db.backup();
        std::printf("backup=%s\n", ok ? "ok" : "failed");
        return ok ? 0 : 1;
    }

    if (opt.command == "deposit") {
        bool ok = db.deposit();
        std::printf("deposit=%s\n", ok ? "ok" : "failed");
        return ok ? 0 : 1;
    }

    if (opt.command == "contains-deposited") {
        bool yes = db.containsDeposited();
        std::printf("containsDeposited=%s\n", yes ? "true" : "false");
        return yes ? 0 : 1;
    }

    if (opt.command == "remove-deposited") {
        bool ok = db.removeDeposited();
        std::printf("removeDeposited=%s\n", ok ? "ok" : "failed");
        return ok ? 0 : 1;
    }

    if (opt.command == "repair") {
        auto lastPrint = std::chrono::steady_clock::now();
        double score = db.retrieve([&](double progress, double /*increment*/) -> bool {
            if (!opt.showProgress)
                return true;
            auto now = std::chrono::steady_clock::now();
            if (now - lastPrint < std::chrono::milliseconds(250))
                return true;
            lastPrint = now;
            std::printf("progress=%.4f\n", progress);
            std::fflush(stdout);
            return true;
        });
        std::printf("repairScore=%.6f\n", score);
        return score > 0 ? 0 : 1;
    }

    printUsage();
    return 2;
}

} // namespace

#if defined(_WIN32)
int wmain(int argc, wchar_t* argvW[])
{
    std::vector<std::string> argv;
    argv.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; i++) {
        argv.push_back(utf8FromWide(argvW[i]));
    }
    return run(argv);
}
#else
int main(int argc, char* argvC[])
{
    std::vector<std::string> argv;
    argv.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; i++) {
        argv.emplace_back(argvC[i] ? argvC[i] : "");
    }
    return run(argv);
}
#endif

