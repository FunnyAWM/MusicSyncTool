# AGENTS_CPP.md · C++/Qt 工程约束

> 本文件承接仓库 `AGENTS.md` 中与 C++、Qt、构建系统、平台实现、数据层和交付有关的规则。
> 通用工作流、Git、需求边界、文档同步和验收记录要求见 [`AGENTS_GENERAL.md`](AGENTS_GENERAL.md)。
> 行为事实源是 `README.md` 与 `docs/classes/`。

## 1. 技术基线

| 项 | 约束 |
| --- | --- |
| 编译标准 | C++17；CMake 侧必须 `CMAKE_CXX_STANDARD 17` + `CMAKE_CXX_STANDARD_REQUIRED ON`，MSVC 侧不得提升到更高标准 |
| Qt | Qt 6.8 LTS 为唯一基线：Windows 全部配置（含测试工程）统一用 `6.8.3_msvc2022_64`，CMake 侧按本机 Qt 6.8+ 安装；不得使用高于 Qt 6.8 引入的 API |
| Qt 模块 | Widgets、Sql、Multimedia、Concurrent；测试追加 Test；翻译用 LinguistTools |
| 编译器 | MSVC 2022（v143，`ConformanceMode`）；Linux/macOS 侧 GCC/Clang（`-Wall -Wextra`，CMake 已设置） |
| 第三方依赖 | TagLib、utfcpp，一律经 vcpkg 管理；新增依赖须先评估 vcpkg 可用性与体积 |
| 目标平台 | Windows（x64 为主，Win32 保留）；Linux、macOS 仅 CMake 侧支持 |
| 界面文件 | 界面使用 `src/UI/**/*.ui`（AUTOUIC/QtUic），禁止继续把整棵界面树堆进构造函数 |
| 代码检查 | VS Debug 配置已启用 ClangTidy；构建产物不得入库 |

升级 Qt 基线时必须同步五处：`MusicSyncTool.vcxproj` 的 `QtInstall`（四个配置）与写死的 include/lib 路径（当前为 `D:\Qt\6.8.3\msvc2022_64`）、`Test/MusicSyncToolTests.vcxproj` 的 `QtInstall`、`CMakeLists.txt` 的 `CMAKE_PREFIX_PATH` 与 `QT NAMES`、`README.md` / `README_en_US.md` 构建说明、`ACCEPTANCE.md`。

## 2. C++ 语言规则

项目默认 C++17，以可读性和代码量减少为前提；禁止为了使用语言特性而使用。

### 2.1 禁用

- `std::string_view`：使用 `QStringView`、`QLatin1String` 或 `const QString &`。
- `std::filesystem`：使用 `QDir`、`QFileInfo`、`QStorageInfo` 等 Qt 文件 API。
- `std::any`：使用 `QVariant`。
- `std::execution` / 并行算法：并发统一走 `QtConcurrent`（见 §6）。
- C++20 及以上特性一律禁用（指定初始化器、概念、协程、模块、`std::format`、`std::span`、`std::ranges` 等）。

### 2.2 允许的 C++17 特性

`std::optional`、`std::variant`、`if constexpr`、结构化绑定、折叠表达式、`[[nodiscard]]`、`[[maybe_unused]]`、`std::clamp`、`std::as_const` 可以使用，但必须实际提升可读性或减少代码。`if constexpr` 只用于类型分支，不得用来绕过平台或错误处理规则。

### 2.3 项目既有约定（新代码必须延续）

- 头文件使用 include guard：`#ifndef <文件名大写>_H` + 结尾注释，风格与现有头一致；不得混用 `#pragma once`。
- 包含路径使用与所在目录匹配的相对路径（如 `Services` 内引用 `"../Core/MusicProperties.h"`）。
- 枚举一律 `enum class`，集中放 `PROPERTIES`（`src/Core/MusicProperties.h`）；数值与枚举转换用其中的 `toShort` / `toXxx` 助手，禁止散落 `static_cast` 魔法数。
- 公开接口返回值优先用 `[[nodiscard]]`。
- 命名跟随现状：类型与函数驼峰、成员函数小驼峰；新模块不得另立命名风格。

## 3. 目录与分层

```text
src/
├── Core/       基础类型（PROPERTIES 枚举、SettingsData）、纯算法（StringSimilarity）、平台垫片（SingleInstance、MSTTagUtils）
├── Data/       数据模型（QueryItem、LyricIgnoreRule），不依赖 Services/UI
├── Services/   业务与 IO：Logger、MSTDataSource、MSTFileManager、MSTMediaPlayer、
│               MSTScanController、MSTSettingsManager、MSTTagScanner
├── UI/         各页面（AboutPage、AddRuleWidget、LoadingPage、OperationResult、Settings、
│               ShowDupe、MusicSyncTool：MSTMainWindow/MSTTableManager/MSTMediaController/MSTErrorReporter）
└── main.cpp    装配入口：样式表加载、单实例分发、主窗口
```

依赖方向：`UI -> Services -> Data -> Core`；`main.cpp -> UI/Services/Core`。

硬性要求：

- `Core/` 中的纯算法与类型不得依赖 Qt Widgets/QtGui。
- SQL 只允许出现在 `Services/MSTDataSource*`；UI 与其他服务通过 `MSTDataSource` 接口访问数据。
- TagLib 调用只允许出现在 `Services/MSTTagScanner*` 与 `Core/MSTTagUtils.h`；其他位置使用 `MSTTagUtils::createFileRef` 封装，不得直接构造 `TagLib::FileRef`。
- 新增纯数据结构放入 `Data/`，平台实现放入 `Core/` 垫片，业务编排放入 `Services/`，界面放入 `UI/`，不跨层放置。
- 相对路径包含风格统一，不允许为个别文件引入新的包含目录约定（CMake/vcxproj 已按层配置包含路径）。

## 4. 平台条件编译与能力

- 平台宏（`_WIN32` / `_WIN64` / `__linux__` / `__APPLE__`）只允许出现在 `src/Core/SingleInstance.h`、`src/Core/MSTTagUtils.h` 与 `src/main.cpp` 的单实例分派处。
- `src/Data`、`src/Services`、`src/UI` 一律不得书写平台条件编译与平台专属头（`Windows.h`、`unistd.h` 等）。既有违规点（`MSTFileManager.cpp` 中直接按平台构造 `TagLib::FileRef`、`MSTMainWindow.cpp` 中的 `#if defined(__linux__)` 包含 `unistd.h`）在触及相关代码时收敛到 `Core/` 垫片后删除。
- 新增平台能力必须走 `Core/` 垫片：按平台返回统一签名，不可用时返回明确失败（`false` / `-1` / 空值），由 UI 层提示，不得静默返回看似合理的默认值。
- 单实例现状：Windows 用命名互斥量，Linux 用 `/tmp/<appName>.lock` 文件锁；互斥量句柄与文件描述符的释放只在 `main.cpp` 完成，其他位置不得持有。
- Linux 侧注意路径口径：锁文件与日志路径拼接统一用 `QDir` / `QString` 操作，不手写分隔符。

## 5. Qt API 与界面约束

禁止：

- `QRegExp`（用 `QRegularExpression`，`MSTDataSource.cpp` 已是正确范例）；
- `SIGNAL()` / `SLOT()` 宏（用函数指针或 Lambda 新式语法）；
- `QDesktopWidget`、`QX11Info` 等 Qt5 遗留 API；
- `QT_DEPRECATED*` 标记成员。

约定：

- QObject 生命周期：构造函数传 `parent`，明确所有权；跨对象持有优先使用父节点或引用，避免裸 `new` 后无人释放。
- 不可拷贝的 QObject 派生类按现状管理（本项目现状为值语义类 + 引用成员，不强制 `Q_DISABLE_COPY`，新类自洽即可）。
- 用户可见字符串必须 `tr()`；占位符用 `%1`。
- 样式表：`resources/MusicSyncTool.qrc` 统一登记，qss 按 base → mode → accent 顺序加载（见 `main.cpp`），新增样式必须保持该顺序语义且补进 qrc。
- 界面结构变更优先改 `.ui`；动态控件 Slots 连接集中在现有 `connectSlots` 类入口，禁止散落连接。
- 界面文案、默认值、单位（页大小 `PAGESIZE`、分页页码从 1 开始）与现状保持一致，变更需同步文档。

## 6. 并发与线程

- 后台任务统一 `QtConcurrent::run`（现状：`MSTScanController::scanConcurrent` / `loadPage`、`MSTMainWindow::copyMusic`）。
- 用 `QtConcurrent::run(&Class::method, this, ...)` 在 `this` 上执行时，方法体内只允许访问局部拷贝与线程安全的 Services 接口；禁止直接读写 UI 控件、`QWidget`、`QSqlQuery` 实例成员。
- 后台任务与 UI 的数据交换通过信号槽回到 UI 线程（现有 `scanTotal` / `scanCurrent` / `loadStarted` / `loadFinished` / `errorOccurred` / `loadErrorOccurred` 即为约定通道）。
- 新增共享可变状态必须先加互斥或改为任务内局部汇总；不在线程间传递裸指针，需要时传值或传智能指针。
- `MSTMediaPlayer` 与 `QMediaPlayer` 只在 GUI 线程使用，不进线程池。

## 7. 数据层与持久化

- 数据库为 SQLite（`QSQLITE`），库文件名 `musicInfo.db`，位于所选音乐目录下；本地与远程各一个 `MSTDataSource` 实例，连接名必须区分（现状经 `setConnectionName` 管理）。
- 唯一表 `musicInfo(title, artist, album, genre, year, track, favorite, ruleHit, fileName)`。
- schema 变更：`initTable` 的 `CREATE TABLE IF NOT EXISTS` 必须保持幂等；新增列或改列必须提供旧库迁移（`ALTER TABLE` + 校验），不得让旧版数据库直接打开失败；引入迁移时在提交说明中写明影响版本。
- 批量写（收藏、规则命中）必须包在事务里；查询统一 `prepareStatement` + `bindValue` 参数化，禁止字符串拼接 SQL（`IN (...)` 动态占位符按 `getByFileNames` 的现有写法生成命名参数）。
- 时间戳与日期：扫描记录写入 `log/` 的 lastScan 文件，格式 `QDateTime::toString()` 默认口径；解析失败回退 1970-01-01 全量扫描的逻辑不得改变。
- 设置文件 `settings.json`，键表：`ignoreLyric`、`recursiveScan`、`sortBy`、`orderBy`、`language`、`favoriteTag`、`rules[]{ruleType,ruleField,ruleName}`；增删键必须同步 `SettingsData`、`createDefaultSettings` 默认值和文档；解析失败时重建默认设置并告知用户（现状 `Logger::Warn` + 返回 false）。
- 所有运行时文件（`settings.json`、`log/`、`musicInfo.db`、`diagnosis/` 输出）必须以 `QCoreApplication::applicationDirPath()` 或其下目录为锚点构造绝对路径；禁止依赖进程工作目录的相对路径打开。
- 数据库、配置、日志文件不入库（`.gitignore` 已覆盖，新增运行时文件必须补规则）。

## 8. 错误处理与注释

- 只在系统边界校验：用户输入路径、文件与数据库操作返回值、平台 API 返回值。
- 全部日志经 `Logger`（`Info/Debug/Warn/Error/Fatal`，静态类）；`Logger::Fatal` 会 `std::exit`，仅限不可恢复的初始化失败，普通失败用 `Error` 返回错误状态。
- 不得吞异常、空 `catch`、用 `assert` 替代运行时错误处理。
- 加载/拷贝错误按现状汇入错误列表，经 `MSTErrorReporter` / `OperationResult` 界面呈现，不在服务层直接弹窗。
- 注释使用中文，说明"为什么"而不是重复代码；平台兼容技巧和绕过必须写一行原因注释；有意采用全量扫描、全量重拷、朴素算法时使用 `ponytail:` 注释说明上限与升级路径。
- 非平凡逻辑必须有最小可运行检查，优先 Qt Test；不引入新的测试框架。

## 9. 翻译、资源与打包

- 新增/修改用户可见字符串必须 `tr()`，并跑 `lupdate` 更新 `translations/Translation_{zh_Hans,en_US,Wenyan}.ts`；三种语言都要有对应译文，缺失时保留英文原文并登记。
- `langinfo.json` 存在根目录与 `translations/` 两份，内容必须保持一致；自定义语言文件的生成流程以 `README.md` 第 3 节为准。
- qm 由 QtMsBuild `lupdate_lrelease` 构建，输出 `translations/`；Release 打包 `QtDeploy=true` 且 `QtDeployNoTranslations=true`，翻译文件随 `translations/` 目录手动部署，不得依赖 windeployqt 补翻。
- 资源（图标、样式）统一经 `resources/MusicSyncTool.qrc` 引用，使用 `:/MusicSyncTool/...` 路径；不得散落磁盘相对路径。
- 打包（msi / 绿色版）产物与数据库、日志、设置文件隔离，安装与解压即用口径以 `README.md` 为准。

## 10. 构建、测试与交付自检

### 10.1 Windows（VS 2022）

- 在 VS 中打开 `MusicSyncTool.sln`，分别构建 Debug|x64 与 Release|x64；Release 构建执行 windeployqt（`QtDeploy`）。
- 测试构建 `Test/MusicSyncToolTests.vcxproj`（Debug 配置带 ClangTidy）。
- 修改 `QtInstall`、vcpkg 依赖或项目属性后必须两个配置各构建一次。

### 10.2 Linux / macOS（CMake）

- `CMakeLists.txt` 仅接受 Linux/Darwin，在 Windows 上配置会 `SEND_ERROR` 拒绝，属预期行为。
- 按需修改 `CMAKE_PREFIX_PATH` 指向本机 Qt 6.8+ 后：

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

- `Test/CMakeLists.txt` 提供 `ENABLE_COVERAGE`、`ENABLE_SANITIZER` 选项，按需开启并记录到 `ACCEPTANCE.md`。

### 10.3 静态与自检

- 静态检查确认：业务层无平台宏、无禁用 Qt API、无 C++20 特性、无字符串拼接 SQL。
- ClangTidy 在 VS Debug 配置可用；工具不可用时在 `ACCEPTANCE.md` 记录原因。
- 构建与测试后确认：Debug/Release 构建状态、相关单元测试结果、事实源与 AGENTS 文档同步情况、`ACCEPTANCE.md` 中未验证边界清晰。
- 构建产物、`MusicSyncTool/`、`Test/x64/`、`diagnosis/` 输出、数据库、日志与设置文件不得提交。
