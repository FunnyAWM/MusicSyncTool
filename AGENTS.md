# AGENTS.md · 工程约束导航（MusicSyncTool）

> 本文件是仓库级 Agent 入口，只保留导航、事实源和跨文档核心约束。
> 具体规则按任务类型分别读取 [`AGENTS_GENERAL.md`](AGENTS_GENERAL.md) 与 [`AGENTS_CPP.md`](AGENTS_CPP.md)。
> 产品行为事实以 [`README.md`](README.md) / [`README_en_US.md`](README_en_US.md) 与 [`docs/classes/`](docs/classes/) 为准；验收证据记录在 [`ACCEPTANCE.md`](ACCEPTANCE.md)（尚无该文件时按 `AGENTS_GENERAL.md` §5 创建并持续追加）。

## 0. 项目事实速览

- 名称与版本：MusicSyncTool v2.2.3；版本号以 `CMakeLists.txt` 的 `PROJECT_VERSION` 与 `MusicSyncTool.vcxproj` 为准，修改需同步两处。
- 技术栈：C++17 + Qt 6（模块 Widgets、Sql、Multimedia、Concurrent、Test），依赖 TagLib 与 utfcpp（vcpkg 管理）。
- 主构建：Windows 使用 Visual Studio 2022（`MusicSyncTool.sln` + `MusicSyncTool.vcxproj` + Qt VS Tools，vcpkg 集成）；`CMakeLists.txt` 仅用于 Linux / macOS，并明确拒绝在 Windows 上通过 CMake 配置。
- 单元测试：`Test/` 目录，基于 Qt Test，入口 `Test/main.cpp`；CMake 侧由 `BUILD_TESTS` 选项启用（`ctest`），VS 侧为 `Test/MusicSyncToolTests.vcxproj`。
- 分层：`src/Core`（基础类型、纯算法、平台垫片）→ `src/Data`（数据模型）→ `src/Services`（业务与 IO）→ `src/UI`（界面）；入口为 `src/main.cpp`。
- 配置与数据：设置存于 `settings.json`（`MSTSettingsManager` / `SettingsData`）；媒体元数据存于 SQLite 数据库 `musicInfo.db`（`MSTDataSource` 管理）；运行日志位于 `applicationDirPath()/log/`。
- 翻译与资源：`translations/Translation_{zh_Hans,en_US,Wenyan}.ts` 由 QtMsBuild `lupdate_lrelease` 构建，qm 输出到 `translations/`；静态资源在 `resources/MusicSyncTool.qrc`，样式表按 base → mode → accent 顺序加载。

## 1. 动手之前

### 1.1 强制工作顺序

1. 检查目标文件真实存在并读取当前内容，不凭记忆覆盖文件。
2. 行为变更先在 `README.md` / `docs/classes/` 找到对应描述；没有事实源依据时先询问，不得凭猜测发明需求。
3. 开始修改前说明理解、影响文件和验收方式。
4. 修改后按相关的 `AGENTS_GENERAL.md` / `AGENTS_CPP.md` 条款和验收清单自检。

### 1.2 规则归属

| 内容 | 唯一正文 |
| --- | --- |
| 通用工作流、需求边界、Git、文档同步、验收和协作 | [`AGENTS_GENERAL.md`](AGENTS_GENERAL.md) |
| C++/Qt、构建系统、平台、分层、并发、数据、翻译和交付自检 | [`AGENTS_CPP.md`](AGENTS_CPP.md) |
| 产品行为、功能边界、安装与源码构建说明 | [`README.md`](README.md) / [`README_en_US.md`](README_en_US.md) |
| 接口与类文档 | [`docs/classes/`](docs/classes/) |
| 已执行的构建、测试、平台会话和剩余边界 | [`ACCEPTANCE.md`](ACCEPTANCE.md) |

任务开始时必须读取本文件，并按任务涉及的范围继续读取对应的专项文件；不得把本文件与专项文件当成两套并行规则。

### 1.3 禁止擅自扩大范围

产品功能边界以 `README.md` 的描述为准（音乐与歌词文件的识别和拷贝、歌词标签处理、收藏筛选、重复项展示、设置与多语言等）。不得借修复或重构之名注入账号体系、云同步、网络同步、在线曲库等未在事实源中描述的能力；确有必要时先提出并获得确认，再更新事实源。

## 2. 跨文档核心约束

- 规则与事实源冲突时，以事实源的行为描述为准，并同步修订受影响文档；不得让工程规则与事实源长期不一致。
- 业务层（`src/Data`、`src/Services`、`src/UI`）不得书写平台条件编译；平台差异统一收敛到 `src/Core` 的平台垫片，遵循 `AGENTS_CPP.md` §4。
- 数据库 schema 变更必须考虑旧库迁移（`MSTDataSource::initTable` 必须保持幂等）；持久化路径、连接命名、配置键和统计口径遵循 `AGENTS_CPP.md` §7 与事实源。
- 不得使用 `AGENTS.md`、`AGENTS_GENERAL.md`、`AGENTS_CPP.md` 之外的第二套工程约束正文；新增规则必须放入对应唯一正文并更新本导航。
- 构建、单元测试、静态检查和人工验收必须区分；工具不可用或环境性失败须在 `ACCEPTANCE.md` 记录原因与未验证边界。

## 3. 交付前最小检查

- [ ] 工作区状态已检查，无关修改未被覆盖或混入提交。
- [ ] 已读取对应的 `AGENTS_GENERAL.md` / `AGENTS_CPP.md` 条款。
- [ ] 行为、平台、配置、schema、翻译或资源变更已同步 `README.md` / `docs/classes/` 等事实源。
- [ ] 目标平台的构建（Windows 走 VS Debug/Release；Linux/macOS 走 CMake）和相关单元测试已执行；工具不可用或环境性失败已记录原因。
- [ ] 未提交构建产物（`MusicSyncTool/`、`Test/x64/`）、`*.obj` / `*.tlog` / `*.log`、`diagnosis/` 输出、运行时生成的 `settings.json`、`musicInfo.db` 与日志文件。

## 4. 版本记录

**版本**：AGENTS.md v1.0 · 与 `AGENTS_GENERAL.md` v1.0、`AGENTS_CPP.md` v1.0 配套
**变更记录**：
- v1.0 — 依据 MusicSyncTool v2.2.3 项目现状建立工程约束体系：根文件收敛为工程约束导航与跨文档核心规则；通用规则归入 `AGENTS_GENERAL.md`，C++/Qt 专项规则归入 `AGENTS_CPP.md`；事实源为 `README` 与 `docs/classes/`，验收证据由 `ACCEPTANCE.md` 按需创建沉淀。
