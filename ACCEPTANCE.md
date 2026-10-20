# ACCEPTANCE · 验收证据记录

本文件记录 MusicSyncTool 已实际执行的构建、单元测试、静态检查与人工验收，以及尚未覆盖的边界。
**只记录真实执行过的内容**；环境不可用的项目如实标注为未验证，不伪造证据。

---

## 1. 本次验收环境

| 项 | 值 |
| --- | --- |
| 操作系统 | Windows 11 (x64) |
| 编译器 | MSVC 14.51（Visual Studio 2022 Community，`vcvars64`） |
| Qt | 6.8.3 msvc2022_64（LTS，工程基线，含 Test / Sql / Multimedia / Concurrent / Widgets） |
| 第三方库 | vcpkg `x64-windows`：`taglib`（`tag.lib`）、`utfcpp`（仅头文件） |
| 代码基线 | `f6d8643`（分层重构）→ 本次会话 8 个提交，最新 `docs/classes` 与测试批次见 §4 |
| 图形环境 | 单元测试以 `QT_QPA_PLATFORM=offscreen` 运行（无交互桌面会话） |

---

## 2. 已执行的验证

### 2.1 逐翻译单元语法校验（Debug 语义，全部 30 个 TU）

对 `src/**/*.cpp` 与 `Test/*.cpp` 逐个执行 `cl /Zs`（仅解析、不产出目标文件），编译选项与工程一致：
`/std:c++17 /Zc:__cplusplus /utf-8 /EHsc /W3 /permissive-`，并带工程同款 `QT_*_LIB` 宏与包含路径
（`src`、`src/Core|Data|Services|UI`、`<构建目录>/qt/uic`、Qt 头、vcpkg 头）。

```
================ 语法校验结果（每次修改后复跑） ================
翻译单元        : 30
RESULT=OK       : 30
RESULT=FAILED   : 0
error C*        : 0
warning C*      : 0
```

### 2.2 UI 头文件再生成

工程 `.ui` 变更后使用 Qt 自带 `uic.exe` 重新生成 `<build>/qt/uic/ui_*.h` 共 7 个，全部 `UIC_OK`。
（此前语法校验报出的 `logToFileBox 不是 Ui::SettingsClass 的成员` 即由此前遗留的过期 `ui_Settings.h` 导致，
属于构建产物过期而非源码缺陷。）

### 2.3 单元测试工程编译 + 运行（真实执行）

工程侧的 Qt VS Tools 解决方案未参与本次验收（原因见 §3.1），改用与工程等价的 `cl` + `link` 手工构建测试可执行文件：

```
moc.exe  src/Services/MSTDataSource.h + Test/*.h（9 个测试类）      → 10 个 moc_*.cpp
cl /c    Test/main.cpp + src/Core/StringSimilarity.cpp + src/Data/{QueryItem,LyricIgnoreRule}.cpp
         + src/Services/{Logger,MSTDataSource,MSTFileManager,MSTMediaPlayer,MSTSettingsManager}.cpp
link     Qt6Test/Qt6Widgets/Qt6Multimedia/Qt6Concurrent/Qt6Sql/Qt6Gui/Qt6Core + tag.lib + zlib.lib + zstd.lib
运行     mst_tests.exe <TestClassName> -o <日志>,txt   （每类单独进程，QT_QPA_PLATFORM=offscreen）
```

| 测试类 | 结果 |
| --- | --- |
| TestMusicProperties | 16 passed, 0 failed |
| TestStringSimilarity | 25 passed, 0 failed |
| TestQueryItem | 16 passed, 0 failed |
| TestLyricIgnoreRule | 12 passed, 0 failed |
| TestMSTFileManager | 12 passed, 0 failed |
| TestMSTDataSource | 22 passed, 0 failed |
| TestMSTSettingsManager | 13 passed, 0 failed |
| TestIntegration | 11 passed, 0 failed |
| TestBlackBox | 30 passed, 0 failed |
| **合计** | **157 passed, 0 failed, 0 skipped** |

本次新增的回归用例：

- `TestMusicProperties`：`themeMode`/`accentColor` 编解码往返 + 越界回退（主题引擎的持久化口径）；
- `TestMSTSettingsManager`：默认值写入 `settings.json`、主题与日志键往返、**旧文件缺键保留结构体默认值**；
- `TestMSTDataSource`：**旧库 schema 迁移**（缺列补齐 + 幂等）、**搜索 LIKE 通配符转义**、**`getByFileNames` 跨批次分块**（1201 个文件名 / 3 批）。

### 2.4 翻译流程

在本机 Qt 6.8.3 下执行 `lupdate`（`src` → 3 个 `.ts`）与 `lrelease`：

- 三份 `.ts` 均输出 `130 source text(s)`，与源码文案一一对应；XML 结构校验通过（`contexts/messages` 计数一致）；
- `en_US` 无空译文；**`Wenyan` 已 130 条全部译出**（简体源文案 → 繁体文言，不再有 `type="unfinished"`
  `lrelease` 报告 `130 finished and 0 unfinished`）；`zh_Hans` 按既有约定保留 `type="unfinished"`
  （源文案即中文，运行时回落源文案）；
- `.qm` 为构建产物（`.gitignore` 忽略），随 VS 构建的 `lupdate_lrelease` 目标重新生成。

### 2.5 编译期发现的真实缺陷（已修）

| 缺陷 | 表现 | 处置 |
| --- | --- | --- |
| `FileErrorType::DUPLICATE` 与 Windows SDK `nb30.h` 的 `DUPLICATE` 宏冲突 | 枚举被宏替换成 `0x06`，`main.cpp` 编译失败 | 重命名 `DUPLICATE_FILE` |
| 删除 `MSTTagScanner.h` 后 `MSTDataSource.h` 失去传递包含 | `PROPERTIES` 不是命名空间 | 显式 `#include "../Core/MusicProperties.h"` |
| 头文件级 `using namespace PROPERTIES` 被移除 | `MSTMainWindow`/`Settings`/`AddRuleWidget` 找不到名字 | 三个 TU 显式 `using` 声明 |
| `on_actionSettings_triggered` 的 `const` 与 `showSettings()` 不一致 | C2662 | 去掉 `const` |
| 测试夹具 `init()/cleanup()` 声明在 `private:` 下 | QTest 只调用私有槽，夹具从未执行：`ds` 为 `nullptr` 直接崩溃，设置测试还会把 `settings.json` 写进仓库根目录 | 移入 `private Q_SLOTS` |
| `MSTDataSource` 用 `QThreadStorage` 缓存每线程查询对象 | 主线程的线程局部数据在 `QThreadStorage` 销毁时不被清理、槽位 id 被复用，新实例取到上一个实例遗留的、绑定在已关闭连接上的 `QSqlQuery`；表现为 `QSqlQuery::exec: database not open`，或语句发到错误的库 | 改为按线程 id 索引的 `QHash`，随 `closeDB()` 与析构释放 |
| `kmpSimilarity` 默认阈值 0.8 与 `fuzzyMatch` 使用的 0.85 不一致 | 直接调用默认阈值时行为与界面判定不一致 | 默认参数对齐 0.85 |
| `Logger` 落盘开关/消息处理器安装无同步 | 工作线程日志与安装动作存在数据竞争 | 开关与处理器指针改原子量，安装段加锁 |

### 2.6 MSBuild 全量重建 + 链接（Visual Studio 工程，Release|x64 / Test Debug|x64，真实执行）

```
MSBuild.exe MusicSyncTool.vcxproj     -t:Rebuild -p:Configuration=Release -p:Platform=x64
MSBuild.exe Test\MusicSyncToolTests.vcxproj -t:Rebuild -p:Configuration=Debug -p:Platform=x64
```

- **不需要任何 `-p:QtInstall*` 覆盖**：Qt 基线已统一到 `6.8.3_msvc2022_64`，工程自带的 include/lib 路径
  与 Qt VS Tools 解析一致（构建日志中 `6.8.3` 命中 68 处、`6.10.1` 0 处）；
- 结果：两个工程均 `0 个错误`、`0 个警告`；Release 产出 `MusicSyncTool.exe` 与 `translations/*.qm` ×3，
  测试工程产出 `MusicSyncToolTests.exe` 并跑通 159 个用例（0 failed）；
- 因本机 VS 实例占用 `vc145.pdb`，命令行构建改用临时 `IntDir`/`OutDir`，构建产物与临时目录均已清理。

### 2.7 内存与分配优化后的复跑（真实执行）

在 §2.6 的同一条命令上复跑（Release 重建 + 链接、测试工程 Debug 重建）：

- 两个工程仍为 `0 个错误`、`0 个警告`；测试用例 **160 passed / 0 failed**（新增 `TestMSTDataSource::getAllFileNames`）；
- 优化点（均不改变行为）：扫描比对改用 `MSTDataSource::getAllFileNames()`（只读一列，避免为每首多构造
  5 个空字段的 `QueryItem`）；`addMusicBatch` 的 `INSERT` 预编译移出循环逐行复用；`fillTableWithItems`
  每行只取一次元素引用（原每行 6 次 `at()` 值拷贝）；`getByFileNames` 改用位置绑定并去掉 `mid()` 子列表拷贝；
- 内存量级为构造层面推算，未做 heap profile：`QueryItem` 约 5 个 `QString` + 2 个整型，每首约两百字节量级，
  只取文件名一列则每首仅一个 `QString`；5000 首曲库的扫描阶段由约 1 MB 降到约 0.3 MB。**未实测，不计入验收结论。**

### 2.8 扫描进度条不动的定位与修复后复跑（真实执行）

- **现象**：加载页进度条在扫描期间停在 0% 或中途不再推进。
- **定位**（代码走查 + `x64/Release/log/lastRun.log` 的扫描耗时记录）：
  1. 最耗时的 `MSTDataSource::addMusicBatch`（逐文件读标签 + 单事务写库）**完全没有进度上报**；
     首次扫描时该阶段占满整个扫描时长（日志中 5.039/5.041 秒两行几乎相同，全部落在这一阶段）；
  2. `favoriteTag` 为空、规则列表为空时收藏/规则两段整体跳过，但原来的分母按 `新增 + 2×filesToScan`
     计入，进度条最多推进到 1/3 就再无变化；
  3. 比对阶段先发一个较小的分母 `compareTotal`，随后切换到不含它的 `globalTotal`，百分比会先到 100% 再回退。
- **改动**：`addMusicBatch` 增加可选 `onProgress(已处理文件数)` 回调（失败文件同样计数，值与列表长度对齐）；
  `MSTScanController::scanConcurrent` 改为整轮扫描只发一次分母（比对存量 + 新增写入 + 实际执行的收藏/规则段），
  跳过阶段不计入分母，批量写入按 20 个一档上报。
- **构建验证**：`MusicSyncTool.vcxproj` Debug|x64 全量 Rebuild、Release|x64 增量构建、
  `Test\MusicSyncToolTests.vcxproj` Debug|x64 构建，三者均 **0 个错误 / 0 个警告**。
- **单元测试**（`Test\x64\Debug\MusicSyncToolTests.exe`，`QT_QPA_PLATFORM=offscreen`，需临时把
  `D:\Qt\6.8.3\msvc2022_64\bin` 与 vcpkg debug bin 加入 `PATH`，否则报 `STATUS_DLL_NOT_FOUND`）：

| 测试类 | 结果 |
| --- | --- |
| TestMusicProperties | 16 passed, 0 failed |
| TestStringSimilarity | 25 passed, 0 failed |
| TestQueryItem | 16 passed, 0 failed |
| TestLyricIgnoreRule | 12 passed, 0 failed |
| TestMSTFileManager | 14 passed, 0 failed |
| TestMSTDataSource | 23 passed, 0 failed |
| TestMSTSettingsManager | 13 passed, 0 failed |
| TestIntegration | 11 passed, 0 failed |
| TestBlackBox | 30 passed, 0 failed |
| **合计** | **160 passed, 0 failed** |

- **未验证**：无交互桌面会话，未在真实曲库上目视确认进度条动画；万首级曲库下 20 个一档的上报密度是否平滑未实测。

### 2.9 CI 全红的三类根因与修复（工作流 run 39346102790）

该 run 的 8 个 job 全部失败，逐个取 job 日志后归为三类：

| 现象（日志原文） | 根因 | 处置 |
| --- | --- | --- |
| `CMake Error at CMakeLists.txt:9 (find_package): Could not find a package configuration file provided by "TagLib"` | Ubuntu 的 `libtag1-dev` 只装 `taglib.pc`，不装 `TagLibConfig.cmake` | 新增 `cmake/FindTagLib.cmake`（config → pkg-config → 手动查找三级回退），根工程与 `Test/CMakeLists.txt` 注册 `CMAKE_MODULE_PATH` |
| `MSTScanController.cpp:11:10: fatal error: 'QtConcurrentRun' file not found`（macOS） | `Test/CMakeLists.txt` 的 `find_package` 与 `target_link_libraries` 都没有 `Concurrent` 组件 | 补 `find_package(Qt6 ... Concurrent ...)` 与 `Qt6::Concurrent` 链接 |
| `This file is for Linux and MacOS only.`（Windows job） | `windows-build` 用 CMake 在 Windows 上配置，而 `CMakeLists.txt` 明确拒绝 | 删除该 job：Windows 主构建走 VS 解决方案，证据见 §2.6 / §2.7 |

同时修正的工作流问题：

- Qt 矩阵由 `6.5.0 / 6.7.0` 改为 `6.8.3`，与 `AGENTS_CPP.md` §1 的基线和 `CMakeLists.txt` 的默认路径一致；
- `-DCMAKE_PREFIX_PATH=$Qt6_DIR` 改为 `${{ env.QT_ROOT_DIR }}`（install-qt-action v4 的导出变量）；
- 根 `CMakeLists.txt` 的硬编码 `set(CMAKE_PREFIX_PATH ...)` 改为仅在调用方未指定时回落，避免覆盖命令行参数；
- `Test/CMakeLists.txt` 以 `Test/` 为顶层构建时补 `project()` 与 `enable_testing()`（原 `add_test` 不会生成 CTest 配置，`ctest` 实际无测试可跑）；
- Linux 覆盖率 job 改用已有的 `-DENABLE_COVERAGE=ON`，不再手工拼 `--coverage` 编译选项；
- 所有跑测试的 job 设 `QT_QPA_PLATFORM=offscreen`（runner 无图形会话），`fail-fast: false` 让矩阵组合都留下日志。

**本地已验证**：

- `FindTagLib.cmake` 用 vcpkg 的 TagLib 2.0 跑最小工程：`Found TagLib: TRUE (found version "2.0")`，`TagLib::tag` 目标可用；
- 手动查找兜底分支（去掉 config 与 pkg-config）同样解析到 `tag.h` / `tag.lib` 并导出目标；
- `ci.yml` 经 PyYAML 解析通过，job 为 `linux-test / macos-test / coverage / code-quality`；
- 本次未改动 C++ 源码，`MusicSyncTool.vcxproj` 的 Debug/Release 构建（§2.7 结论）不受影响，未重复执行。

**未验证**：本机无 Linux/macOS 环境与 Docker，工作流改动只能在推送后由 runner 实际执行验证。

### 2.10 CI 第二轮（run 39356681196）：所有 job 挂在“安装 Qt”

上一轮的三类问题已消失（配置阶段不再报 TagLib / QtConcurrent / Windows 拒绝），新失败点提前到装 Qt 这一步：

```
INFO    : aqt install-qt linux desktop 6.8.3 --autodesktop --outputdir ... --modules qttools qtmultimedia
ERROR   : The packages ['qttools'] were not found while parsing XML of package information!
```

本机用 aqtinstall 复核了 6.8.3 的模块清单：

```
python -m aqt list-qt linux desktop --arch 6.8.3                 → linux_gcc_64
python -m aqt list-qt linux desktop --modules 6.8.3 linux_gcc_64 → debug_info qt3d qt5compat qtcharts ...
```

列表中**没有 `qttools`**：Qt 6 的 LinguistTools（lrelease/lupdate）不在 aqt 的可选模块里，上一轮加 `qttools` 是想当然。

处置：

- 根 `CMakeLists.txt` 把 `find_package(Qt6 COMPONENTS LinguistTools)` 降为 `QUIET`，找到才调 `qt_add_translations`，否则打印提示并跳过——qm 生成只在打包时必需，不该让构建失败；
- 工作流三处 `modules` 去掉 `qttools`，只保留 `qtmultimedia`；
- 顺带修正 `FindTagLib.cmake` 的 pkg-config 分支：`taglib.pc` 的 `Cflags` 是 `-I<prefix>/include/taglib`，而源码写的是 `#include <taglib/…>`，必须同时保留父目录 `<prefix>/include`，否则 Linux 上仍会找不到头文件（本地用等价逻辑验证展开结果为 `…/include/taglib;…/include`）。

**未验证**：Linux 上整包构建（含 UI 层）与测试运行、macOS 的 Test 构建，仍只有推送后由 runner 给出结论。

---

## 3. 未验证 / 未执行（如实记录）

### 3.1 Visual Studio 解决方案构建 —— 已执行（详见 §2.6）

Qt 基线统一到 6.8.3 后，工程自带配置即可完成 Release 重建、链接与测试工程构建，此前“内联路径与
本机 Qt 版本不符、无法命令行构建”的限制已消除。仍建议在另一台仅安装 Qt + vcpkg 的机器上复跑一次，
排除本机残留路径（如 vcpkg 根目录 `E:\vcpkg`）带来的偶合。

### 3.2 Linux / macOS（CMake）—— 未执行

本环境为 Windows，`CMakeLists.txt` 明确拒绝在 Windows 上配置；未做交叉验证。平台垫片
（`SingleInstance.h` 的 `_WIN32` / `__linux__` / `__APPLE__` 分支）只经过代码走查，无运行时证据。

### 3.3 图形界面人工验收 —— 未执行

无交互桌面会话，未执行“扫描真实曲库 → 复制到设备 → 主题切换 → 设置保存 → 重复项/收藏筛选”
的端到端人工走查。相关逻辑仅由单元测试覆盖：
`paginationBoundaries`、`searchAcrossColumns`、`sortConsistency`、`settingsRoundtrip`、
`favoriteFilter`、`ruleHitFilter`、`formatValidation`（TestIntegration/TestBlackBox）。

### 3.4 打包与分发 —— 未执行

未生成 msi/绿色版安装包，未验证 `langinfo.json` 与自定义语言文件的实际加载路径。

---

## 4. 独立复审（code-reviewer）问题清单与处置

对本次改动（`8b2fb48`、`eef6941`、`21e3a2f` 及其前置修复提交）做了一轮只读代码复审。处置如下。

### 4.1 已修复

| 编号 | 问题 | 处置 |
| --- | --- | --- |
| B2 | `releaseThreadQueries()` 在析构路径上不加锁，可能删除其它线程正在使用的 `QSqlQuery` | `~MSTDataSource()` 改为持 `dbMutex` 后再释放，并在注释中写明调用方需先结束任务 |
| I3-3 | `Logger` 的 `logToFileEnabled` 与 `previousHandler` 非原子、安装无锁 | 两者改 `std::atomic`，安装段用独立互斥量保证只执行一次 |
| I5 | `setFavorite()` 直接对可能为空的 `TagLib::FileRef` 取 `properties()` | 与 `addMusicBatch` 一致：先判 `isNull()/tag()`，失败经 `loadErrorOccurred` 上报 |
| M4 | `loadSettings()` 仅对主题/强调色/日志键做缺键回退，`sortBy`/`orderBy` 缺键会被写成 0 | 统一为 `contains` 判定，缺键保留结构体默认值；`legacyFileKeepsDefaults` 用例改为覆盖该路径 |

### 4.2 已记录、本次不改（含理由）

| 编号 | 问题 | 不改的理由 / 后续建议 |
| --- | --- | --- |
| B1/B3 | 低层 `prepareStatement` / `bindValue` / `execQuery` 三段各自加锁，`bindValue` 不持 `dbMutex`，破坏 “prepare → bind → exec” 的原子性 | 契约是“调用方持 `dbMutex`、单线程顺序调用”，当前调用点均满足；改为单一入口 `execPrepared(sql, bindings)` 属接口重构，建议后续单独提交 |
| I1 | `startScan()` 在 GUI 线程 `waitForFinished()` 阻塞等待上一轮扫描；`scanned` 标志在复位与置位间存在逻辑竞态 | 阻塞等待是并发改造阶段为保证“不访问已销毁对象”而选择的保守方案；改进（信号链式触发下一次扫描、generation token）建议后续单独提交 |
| I2 | `copyOperationInProgress` 有两处复位来源 | `QFutureWatcher::finished` 必在 future 结束后于 GUI 线程发出，未发现可复现的“永久卡在 true”路径；保留现状并记录 |
| I3-1 | `Logger::Fatal` 走 `qCritical` + `std::exit`，日志文件中该行级别显示为 `[ERROR]`（消息体带 `[FATAL]`） | 语义一致性问题，改动会影响退出行为，留待专门评审 |
| I4 | `getByFileNames` 某一批失败时 `continue`，上层看到的条目会少一批且不报错 | SQL 层面已有 `Logger::Error`；要让 UI 感知需要改签名或引入失败计数（涉及调用方与测试），本次仅记录 |
| I6/I7 | `MSTMainWindow` 成员“部分由 parent 托管、部分手动 delete”的双轨生命周期 | 现有顺序经走查正确（析构先 `waitForFinished()` 再关库）；统一为所有权管理属后续整理项 |
| M1 | `scanned[static_cast<int>(path)]` 依赖枚举值 0/1 | 建议加 `static_assert` 固化 `PathType` 取值，属清理项 |
| M2 | `Logger` 反向依赖 `MSTSettingsManager::logDirPath()` | 建议改为注入日志目录；涉及 API 变更，历次清理提交未纳入 |
| M5 | 复制收尾用 `onProgress(fileList.size())`，全部失败时进度条仍显示满 | 属展示口径，建议区分“已处理”与“已成功” |
| M6 | `searchMusic` 三列共用同一命名占位符 `:text` | 本机 Qt 6.8.3 实测绑定正常（`searchMusic_escapesWildcards` 覆盖）；如需兼容更老驱动可改为多占位符 |

### 4.3 经证据驳回

| 编号 | 复审结论 | 实际证据 |
| --- | --- | --- |
| M3 | 认为 `Test/main.cpp` 的 `QTest::qExec(t, argc - firstArg + 1, argv + firstArg - 1)` “没有真正剔除类名” | 指定类名时该实参传入的是 `argv + 1` 且长度为 `argc - 1`，类名正好落在 `argv[0]` 位置（QTest 不使用），其余选项从索引 1 起被正常解析。实测 `mst_tests.exe TestStringSimilarity -o <file>,txt -v2` 只运行该类、且选项生效（生成完整输出文件，`Totals: 25 passed`），可判定该结论不成立 |

---

## 5. 剩余边界与建议

1. **VS 解决方案构建**：Release 重建 + 链接、测试工程 Debug 构建已在 §2.6 完成（Qt VS Tools 的
   moc/uic/rcc/lupdate 目标全部通过）；剩余待补为在独立机器上复跑，确认不依赖本机路径。
2. **GUI 人工验收**：大曲库（≥5000 首）扫描、复制中断与磁盘写满、主题即时切换、旧 `settings.json`
   与旧 `musicInfo.db` 升级路径的实机走查。
3. **CMake 侧**：Linux/macOS 上执行 `cmake -DBUILD_TESTS=ON` 构建并运行 `ctest`，补齐平台垫片的运行时证据。
4. **接口与并发整理**：按 §4.2 的 B1/B3、I1、I2、I4 建议，逐步收敛为单一数据库语句入口、
   非阻塞的扫描续跑与复制进度口径。

---

**版本**：ACCEPTANCE.md v1.0
**记录时间**：2026-10-16
**对应代码**：`docs/classes` 文档重写批次之后、含 §2.5/§4.1 全部修复的工作树状态
