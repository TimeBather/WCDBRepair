# WCDBRepair（Windows）

基于 [WCDB C++ “损坏、备份、修复”](https://github.com/Tencent/wcdb/wiki/C%2B%2B-%E6%8D%9F%E5%9D%8F%E3%80%81%E5%A4%87%E4%BB%BD%E3%80%81%E4%BF%AE%E5%A4%8D) 的示例，提供一个 **Windows 下的 WCDB 数据库修复命令行工具**，并通过 GitHub Actions 自动编译产物。

## 功能

- **损坏检测**：`check`（对应 `Database::checkIfCorrupted()`）
- **手动备份**：`backup`（对应 `Database::backup()`）
- **修复**：`repair`（对应 `Database::retrieve()`，输出修复进度与 score）
- **转存/清理转存**：`deposit` / `contains-deposited` / `remove-deposited`
- **加密库支持**：`--key-hex` / `--cipher-page-size` / `--cipher-version`

## 本地编译（Windows）

需要：Visual Studio（MSVC）、CMake、Ninja（推荐）。

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
.\build\wcdb-repair.exe --help
```

## 使用示例

```bash
# 检测是否损坏（耗时：会遍历库内容，谨慎使用）
.\wcdb-repair.exe check "C:\path\to\db.sqlite"

# 修复（输出 progress=... 与 repairScore=...）
.\wcdb-repair.exe repair "C:\path\to\db.sqlite"

# 加密库修复（示例：hex key）
.\wcdb-repair.exe repair "C:\path\to\db.sqlite" --key-hex 001122AABBCC --cipher-version 4 --cipher-page-size 4096

# 转存（修复失败或暂时不修复时使用）
.\wcdb-repair.exe deposit "C:\path\to\db.sqlite"
```

## GitHub Actions

仓库内置工作流：`.github/workflows/build-windows.yml`  
每次 push/PR 会在 `windows-latest` 上构建，并上传 `wcdb-repair-windows.zip` 作为产物。

