# WCDBRepair (Windows)

Based on [WCDB C++ Corruption/Backup/Repair](https://github.com/Tencent/wcdb/wiki/C%2B%2B-%E6%8D%9F%E5%9D%8F%E3%80%81%E5%A4%87%E4%BB%bd%E3%80%81%E4%BF%AE%E5%A4%8D), this repo provides a **Windows WCDB database repair CLI tool** and builds artifacts via GitHub Actions.

## Features

- **Corruption check**: `check` (`Database::checkIfCorrupted()`)
- **Manual backup**: `backup` (`Database::backup()`)
- **Repair**: `repair` (`Database::retrieve()` with progress + score)
- **Deposit & cleanup**: `deposit` / `contains-deposited` / `remove-deposited`
- **Encrypted DB**: `--key-hex` / `--cipher-page-size` / `--cipher-version`
- **SQLCipher compatibility pragmas**: `--kdf-iter` / `--cipher-hmac-algorithm`
- **SQL trace**: enabled by default (disable via `--no-sql-trace`)

## Build locally (Windows)

Requires: Visual Studio (MSVC), CMake, Ninja (recommended).

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
.\build\wcdb-repair.exe --help
```

## Examples

```bash
# Check corruption (may be slow)
.\wcdb-repair.exe check "C:\path\to\db.sqlite"

# Repair (prints PROGRESS=... and RESULT=repair score=...)
.\wcdb-repair.exe repair "C:\path\to\db.sqlite"

# Encrypted DB repair (hex key)
.\wcdb-repair.exe repair "C:\path\to\db.sqlite" --key-hex 001122AABBCC --cipher-version 4 --cipher-page-size 4096

# Encrypted DB with non-default SQLCipher params
.\wcdb-repair.exe repair "C:\path\to\db.sqlite" --key-hex 001122AABBCC --kdf-iter 4000 --cipher-hmac-algorithm HMAC_SHA1

# Deposit (when repair fails or you want to postpone repair)
.\wcdb-repair.exe deposit "C:\path\to\db.sqlite"
```

## GitHub Actions

Workflow: `.github/workflows/build-windows.yml`  
Every push/PR builds on `windows-latest` and uploads `wcdb-repair-windows.zip`.

