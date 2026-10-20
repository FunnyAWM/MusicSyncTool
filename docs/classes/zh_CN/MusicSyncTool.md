# MusicSyncTool 类与分层文档

[English Version](../en_US/MusicSyncTool.md)

本文档描述 MusicSyncTool v2.2.3 的分层结构、各层类职责、运行时数据与线程模型，替代早期“单一 `MusicSyncTool` 巨类”的描述。

## 1. 分层与依赖方向

| 层 | 目录 | 职责 | 允许依赖 |
| --- | --- | --- | --- |
| Core | `src/Core` | 基础类型与枚举、纯算法、平台垫片 | 仅 Qt / 标准库 |
| Data | `src/Data` | 数据模型（查询项、忽略规则） | Core |
| Services | `src/Services` | 业务与 IO（数据库、扫描、复制、设置、日志） | Core、Data |
| UI | `src/UI` | 界面与交互（主窗口、表格、设置、弹窗） | Core、Data、Services |

依赖只允许由外层指向内层：`UI → Services → Data → Core`。业务层（`Data`/`Services`/`UI`）不得书写平台条件编译，平台差异统一收敛在 `src/Core` 的平台垫片内。程序入口为 `src/main.cpp`。

```
src/
├── main.cpp                  程序入口：设置、日志开关、主题、单实例分派
├── Core/
│   ├── MusicProperties.h     枚举、编解码函数、跨线程元类型注册
│   ├── SettingEntity.h       SettingsData（设置实体，含默认值）
│   ├── StringSimilarity.h/.cpp  标题/艺术家/专辑相似度比较
│   ├── SingleInstance.h      平台垫片：Windows 命名互斥量 / Linux、macOS 锁文件
│   └── MSTTagUtils.h         TagLib::FileRef 构造封装
├── Data/
│   ├── QueryItem.h/.cpp      音乐条目模型、模糊相等、重复项查找
│   └── LyricIgnoreRule.h/.cpp  歌词忽略规则（纯值类型）
├── Services/
│   ├── MSTDataSource.h/.cpp      音乐元数据库（SQLite）
│   ├── MSTScanController.h/.cpp  扫描流程与并发调度
│   ├── MSTFileManager.h/.cpp     文件复制与空间检查
│   ├── MSTSettingsManager.h/.cpp settings.json 与日志文件管理
│   └── Logger.h/.cpp             分级日志与落盘
└── UI/
    ├── MusicSyncTool/        主窗口及其协作者
    ├── Settings/             设置窗口
    ├── AddRuleWidget/        添加忽略规则弹窗
    ├── LoadingPage/          加载遮罩
    ├── OperationResult/      操作结果弹窗
    ├── ShowDupe/             重复项列表弹窗
    └── AboutPage/            关于页
```

## 2. Core 层

### 2.1 `PROPERTIES` 命名空间（`MusicProperties.h`）

所有跨类共享的枚举与编解码函数集中在此命名空间。

| 枚举 | 取值 | 用途 |
| --- | --- | --- |
| `PathType` | `LOCAL` / `REMOTE` | 区分本地库与设备库 |
| `PlayState` | `PLAYING` / `PAUSED` / `STOPPED` | 播放器状态 |
| `FileErrorType` | `DUPLICATE_FILE` / `LYRIC_NOT_FOUND` / `DISKFULL` | 复制错误 |
| `LoadErrorType` | `FILE_NOT_SCANNABLE` / `TAG_READ_ERROR` | 加载错误 |
| `SortByEnum` | `TITLE` / `ARTIST` / `ALBUM` | 排序字段 |
| `OrderByEnum` | `ASC` / `DESC` | 升降序 |
| `AppErrorType` | `RUNNING` / `NO_PATH` / `DUPLICATE_FILE` / … | 程序级错误 |
| `OperationType` | 扫描/复制等操作类型 | 结果弹窗与进度上下文 |
| `RuleField` | `TITLE` / `ARTIST` / `ALBUM` / `NONE` | 规则匹配字段 |
| `RuleType` | `INCLUDES` / `EXCLUDES` / `NONE` | 规则匹配方式 |
| `QueryRows` | `ALL` / … | 查询列集合 |
| `ThemeMode` | `LIGHT` / `DARK` | 明暗模式 |
| `AccentColor` | `BLUE` / `GREEN` / `PURPLE` / `DARK` | 强调色 |

编解码函数：`toShort(SortByEnum/OrderByEnum/ThemeMode/AccentColor)` 把枚举编码为可持久化的 `short`；`toSortBy/toOrderBy/toThemeMode/toAccentColor` 做反解并对越界值回退默认值。枚举到 `short` 的映射是 `settings.json` 的事实口径，不得随意改动数值。

跨线程信号使用的枚举必须经 `registerMetatypes()` 注册（幂等），否则经 QtConcurrent 工作线程到 GUI 线程的排队连接会在运行期静默失效。

> `FileErrorType` 的第一个枚举值命名为 `DUPLICATE_FILE` 而非 `DUPLICATE`：Windows SDK 的 `nb30.h` 会把 `DUPLICATE` 定义成宏 `0x06`，而 `SingleInstance.h` 会在 `main.cpp` 中最先引入 `<Windows.h>`，直接使用 `DUPLICATE` 会导致枚举被宏替换而编译失败。

### 2.2 `SettingsData`（`SettingEntity.h`）

设置实体，是界面与持久化层之间唯一的传递结构；所有成员都带默认值，保证 `settings.json` 缺键或解析失败时不会留下未初始化数据。

| 成员 | 默认值 | 说明 |
| --- | --- | --- |
| `ignoreLyric` | `false` | 是否忽略歌词检查 |
| `recursiveScan` | `false` | 是否递归扫描子目录 |
| `sortBy` | 标题 | 排序字段编码 |
| `orderBy` | 升序 | 升降序编码 |
| `language` | 空 | 界面语言 |
| `favoriteTag` | 空 | 自定义“喜欢的音乐”标签 |
| `rules` | 空列表 | 歌词忽略规则 |
| `themeMode` | 明亮 | 明暗模式编码 |
| `accentColor` | 绿色 | 强调色编码 |
| `logToFile` | `false` | 是否将日志写入文件 |

### 2.3 `StringSimilarity`

提供标题/艺术家/专辑的相似度判定：非 CJK 文本用 KMP 子串匹配快速判定，CJK 文本用编辑距离相似度并按更严格的阈值比较。编辑距离采用滚动数组实现，空间复杂度为 O(min(n,m))。

### 2.4 平台垫片

- `SingleInstance.h`：Windows 用命名互斥量（`CreateMutex`），Linux/macOS 用 `/tmp/<应用名>.lock` + `fcntl` 写锁，两者都返回可供 `main.cpp` 释放的句柄/描述符。平台宏只允许出现在本文件、`MSTTagUtils.h` 与 `main.cpp` 的单实例分派处。
- `MSTTagUtils.h`：封装 `TagLib::FileRef` 构造，TagLib 调用只允许出现在此处与扫描/复制相关服务中。

## 3. Data 层

### 3.1 `QueryItem`

音乐条目模型（标题、艺术家、专辑、流派、年份、音轨、文件名）。

- `operator==` 使用模糊比较（标题/艺术家/专辑经 `StringSimilarity` 判定），用于去重；
- `findDuplicatesInSorted(const QList<QueryItem>&)`：对已排序列表做单次遍历，返回重复文件名列表，供“重复项”弹窗使用。

### 3.2 `LyricIgnoreRule`

歌词忽略规则，由 `RuleType` + `RuleField` + 规则名组成。

- 纯值类型：不继承 `QObject`、不含 `Q_OBJECT`，可自由拷贝与放入容器；
- 提供枚举与中文字符串的双向转换（`lyricRulesToString` / `ignoreRulesToString` / `stringToIgnoreRules` / `stringToLyricRules`），是 `settings.json` 中 `rules` 数组的事实口径；
- 因不再继承 `QObject`，翻译上下文固定为 `"LyricIgnoreRule"`，需使用该类 `tr()` 的调用方应改为显式引入 `PROPERTIES` 名字。

## 4. Services 层

### 4.1 `MSTDataSource`

单个音乐目录对应一个 SQLite 连接（`<目录>/musicInfo.db`），负责建表、迁移、查询与批量更新。

线程与锁：

- 所有数据库访问都在 `dbMutex` 保护下进行；事务边界（批量写入）必须整体持锁，否则其他线程的语句会混入事务；
- `QSqlQuery` 按线程 id 索引做到每线程一个实例，禁止跨线程共享查询对象；**不使用 `QThreadStorage`**：主线程的线程局部数据在 `QThreadStorage` 对象销毁时不会被清理，槽位 id 会被后续实例复用，新实例会取到上一个实例遗留的、绑定在已关闭连接上的查询对象；这些查询对象随 `closeDB()` 与析构显式释放；
- 批量写入 `addMusicBatch` 失败时回滚并返回 `false`，同时通过输出参数回传失败文件；
- 设置分页大小 `pageSize`（默认 200）、`updateFavoriteBatch` / `updateRuleHitBatch` 做收藏与规则命中状态的批量落库。

schema 与迁移：

- 表 `musicInfo(title, artist, album, genre, year, track, favorite, ruleHit, fileName)`；
- 版本号存于 SQLite `user_version`，`schemaVersion` 为当前版本常量；
- `initTable()` 幂等：建表后比对版本，逐列 `ensureColumn` 补齐历史库缺失的列，全部成功才写入新版本号，失败则保留旧版本号以便下次启动重试。

查询接口：

| 接口 | 说明 |
| --- | --- |
| `getMusicToTable(pageNum, sortBy, orderBy)` | 按排序取某一页 |
| `getFavorite(...)` / `getRuleHit(...)` | 收藏页 / 规则命中页，另通过 `getLastFavoriteCount()` 暴露真实总数 |
| `searchMusic(text)` / `searchMusic(text, pageNum)` | 搜索；分页版本在 SQL 层用 `LIMIT/OFFSET` 截断，避免整库命中结果进入内存 |
| `getSearchCount(text)` | 搜索命中总数，用于分页计算 |
| `getByFileNames(fileNames)` | 按文件名批量取；按 SQLite 变量上限 500 个一块分批，用位置绑定，不生成 `":fN"` 占位符名 |
| `getAllFileNames()` | 只取 `fileName` 一列并预留容量；扫描比对文件名时使用，避免为每首多构造 5 个空字段的 `QueryItem` |
| `getCount()` / `getAll(rows)` | 总条目数 / 全量读取 |

搜索中的 LIKE 通配符（`%`、`_`）由 `escapeLikePattern` 转义并使用 `ESCAPE`，用户输入不会被当作通配符。

- 批量写入 `addMusicBatch` 的 `INSERT` 预编译语句在循环外只 `prepare()` 一次，逐行复用，避免每条记录重新编译 SQL。

### 4.2 `MSTScanController`

扫描流程的调度者与并发纪律的守门人。

- `startScan` / `scanConcurrent` / `loadPage`：按 `SettingsData`（含 `recursiveScan`）扫描目录，递归模式用 `QDirIterator` 遍历子目录；
- 扫描任务经 `QtConcurrent::run` 在工作线程执行，任务内**禁止**触碰 `QTableWidget`、`QLabel` 等 GUI 对象，也禁止直接使用共享的 `QSqlQuery` 实例成员；结果一律通过信号回 GUI 线程；
- 信号：`scanStart`、`scanFinished`、`scanTotal(int)`、`scanCurrent(int)`、`errorOccurred(AppErrorType)`、`loadErrorOccurred(QString, LoadErrorType)`、`tablePageReady(PathType, QList<QueryItem>, unsigned short, unsigned short)`；
- 持有 `QFutureWatcher<void> scanWatcher`，并通过 `isScanRunning()` / `waitForFinished()` 暴露任务状态，窗口析构前必须等待任务结束，避免任务访问已销毁对象；
- `setFavorite` / `setRuleHit` 支持带进度偏移的批量标记，`resetScan` 复位“已扫描”标记（跨线程读写，故为 `std::atomic<bool>`）。

### 4.3 `MSTFileManager`

文件复制与空间检查。

- `supportedFormat`：支持的音频格式清单；`isFormatSupported(fileName)` 判定格式；
- `copyable(filePath)` 结合目标目录空间判断是否可复制；`isFull(source, target)` 判断磁盘是否已满；`rollBackCopy(fileName)` 回滚半成品文件；
- `copyMusicFiles(source, fileList, target, ignoreLyric, onProgress, onError, onStart, onFinish, onTotal, shouldCancel)`：以回调形式上报进度、错误与总数，错误类型用 `int` 编码（`0`=重复、`1`=歌词缺失、`2`=磁盘已满），由调用方（UI 层）翻译为 `FileErrorType` 并发信号回 GUI 线程；
- `shouldCancel` 为可选的 `const std::atomic<bool>*`：每首歌曲开始前以 `relaxed` 序读取一次，置位后不再开始新的歌曲，**当前歌曲与其歌词必然完整复制**后才退出；取消不回滚已完成文件，收尾进度停在已处理的歌曲数而非 100%。

### 4.4 `MSTSettingsManager`

运行时文件（`settings.json`、`log/`）的统一管理者。

- `baseDir()` 默认取 `QCoreApplication::applicationDirPath()`，不依赖进程工作目录；测试可用 `setBaseDir()` 重定向到临时目录，传空串恢复默认；
- `settingsFilePath()` = `baseDir()/settings.json`；`logDirPath()` = `baseDir()/log`；
- `createDefaultSettings()` 从 `SettingsData` 的成员默认值生成默认文件，保证结构体与文件口径一致；
- `loadSettings(entity)` 读取时对缺键做“保留结构体默认值”处理（`sortBy`、`orderBy`、`themeMode`、`accentColor`），旧版本设置文件因此无需手工升级，也不会被写成 0 等非法编码；加载失败会生成默认文件并返回 `false`；
- `saveSettings(entity)` 写入全部键与 `rules` 数组；写入失败只记录错误并返回 `false`，不终止进程；
- 日志相关：`getDateFromLog` / `writeLog` / `buildLogFileName` / `cleanLog`。

`settings.json` 键一览：`ignoreLyric`、`sortBy`、`orderBy`、`language`、`favoriteTag`、`recursiveScan`、`themeMode`、`accentColor`、`logToFile`、`rules[]`（每项含 `ruleType`、`ruleField`、`ruleName`）。

### 4.5 `Logger`

静态日志工具，分级输出 `Info` / `Debug` / `Warn` / `Error` / `Fatal`。

- 所有输出统一经 Qt 消息机制（`qInfo`/`qDebug`/`qWarning`/`qCritical`）；
- `setLogToFile(true)` 首次调用时安装 `qInstallMessageHandler`，随后把所有消息追加写入 `<baseDir>/log/lastRun.log`，可反复开关；`isLogToFile()` 查询当前状态；
- `Fatal` 记录不可恢复错误并终止进程，仅在确实无法继续运行时使用。

## 5. UI 层

### 5.1 `MSTMainWindow`

主窗口，同时是各协作者的装配点与线程边界上的收口处。

| 成员 | 说明 |
| --- | --- |
| `ui` | 生成的界面组件 |
| `translator` | 翻译器 |
| `local` / `remote` | 本地库与设备库的路径与数据源持有者 |
| `tableManager` | 表格与分页管理 |
| `scanController` | 扫描调度 |
| `errorReporter` | 加载/复制错误与结果弹窗 |
| `mediaController` | 播放控制 |
| `loading` | 加载遮罩 |
| `entity` | 当前设置实体 |

主要职责：

- `initDatabase()` / `loadSettings()` / `initUI()`：装配顺序固定为数据库 → 设置 → 界面；
- `applyTheme(entity)`：三段式主题加载（见 §7），设置保存后即时生效；
- `getMusic` / `getMusicConcurrent` / `searchMusic` / `getFavoriteMusic` / `getDuplicatedMusic`：驱动扫描、搜索、收藏筛选与重复项展示；
- `copyMusic(source, fileList, target)`：经 `MSTFileManager::copyMusicFiles` 在工作线程复制，进度与错误经信号回 GUI 线程登记；
- `showSettings()`：创建设置窗口并设 `WA_DeleteOnClose`，避免标题栏关闭造成窗口泄漏；
- `popError(AppErrorType)` / `showOperationResult(OperationType)`：错误与结果提示；
- 析构时先等待扫描/复制任务结束，再释放数据源与界面。

### 5.2 其余界面类

| 类 | 职责 |
| --- | --- |
| `MSTTableManager` | 两个表格的控制者：分页与总页数、当前页、仅看收藏开关、行填充与选中态 |
| `MSTErrorReporter` | 维护加载/复制错误列表与操作结果弹窗，把错误码翻译为本地化文案 |
| `MSTMediaController` | 播放/暂停、进度条与音量交互、正在播放标题；播放状态经 `PlayState` 回传 |
| `Settings` | 设置窗口：语言、排序、忽略歌词、递归扫描、日志落盘、主题（明暗 + 强调色）与规则表；`confirmPressed(SettingsData)` 回传设置 |
| `AddRuleWidget` | 添加歌词忽略规则；`sendRules(LyricIgnoreRule)` 回传 |
| `LoadingPage` | 扫描/复制期间的加载遮罩与进度显示；`setCancelable(true)` 在进度条右下方显示取消按钮并发出 `cancelRequested()`，扫描任务保持隐藏 |
| `OperationResult` | 操作结果列表弹窗 |
| `ShowDupe` | 重复项列表弹窗 |
| `AboutPage` | 关于页 |

## 6. 线程模型

| 场景 | 执行线程 | 约定 |
| --- | --- | --- |
| 目录扫描、标签读取、文件复制 | `QtConcurrent` 工作线程 | 只做 IO 与计算；结果经信号排队投递回 GUI 线程 |
| 数据库访问 | 工作线程与 GUI 线程共用连接 | 统一由 `MSTDataSource::dbMutex` 串行化；`QSqlQuery` 每线程一个 |
| 界面刷新、弹窗、表格填充 | GUI 线程 | 只在槽函数中执行 |
| 任务生命周期 | GUI 线程 | `QFutureWatcher` 持有任务；析构/关闭前调用 `waitForFinished()` |

跨线程信号参数必须已在 `PROPERTIES::registerMetatypes()` 中注册，参数类型使用可在 moc 侧解析的整型或已声明元类型。

## 7. 主题与样式

样式按固定顺序拼接后整体 `setStyleSheet`，缺失的段会被跳过以保证界面仍可用：

1. `:/MusicSyncTool/base-style.qss`（基础样式）
2. 明暗模式：`style-light.qss` 或 `style-dark-mode.qss`
3. 强调色：`style.qss`（蓝）/ `style-green.qss` / `style-purple.qss` / `style-dark.qss`

样式由 `settings.json` 的 `themeMode` 与 `accentColor` 决定，`Settings` 窗口的两个下拉框的 `itemData` 直接存储编码值，不依赖下拉项顺序。

## 8. 运行时数据

| 文件 | 位置 | 说明 |
| --- | --- | --- |
| `settings.json` | `applicationDirPath()` | 用户设置，见 §4.4 键一览 |
| `musicInfo.db` | 每个音乐目录一个 | 元数据缓存，schema 版本见 §4.1 |
| `log/lastRun.log` | `applicationDirPath()/log` | `logToFile` 开启时的运行日志 |
| `translations/langinfo.json` | 程序目录 | 语言下拉框数据源 |
| `translations/*.qm` | 程序目录 | 翻译文件（`Translation_zh_Hans` / `Translation_en_US` / `Translation_Wenyan`） |

## 9. 构建与测试

- Windows：Visual Studio 2022 打开 `MusicSyncTool.sln`，依赖由 vcpkg 提供（`taglib`、`utfcpp`）；构建配置为 Debug/Release。
- Linux / macOS：`CMakeLists.txt` 提供 CMake 构建（`BUILD_TESTS` 打开单元测试），Windows 下不在 CMake 侧构建。
- 单元测试：`Test/` 目录基于 Qt Test，入口 `Test/main.cpp`，各测试类的实现由入口统一包含。
