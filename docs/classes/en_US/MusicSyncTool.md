# MusicSyncTool Architecture and Class Reference

[中文版本](../zh_CN/MusicSyncTool.md)

This document describes the layering, per-layer class responsibilities, runtime data and threading model of MusicSyncTool v2.2.3. It replaces the earlier description of a single monolithic `MusicSyncTool` class.

## 1. Layers and Dependency Direction

| Layer | Directory | Responsibility | May depend on |
| --- | --- | --- | --- |
| Core | `src/Core` | Base types and enums, pure algorithms, platform shims | Qt / standard library only |
| Data | `src/Data` | Data models (query item, ignore rule) | Core |
| Services | `src/Services` | Business logic and IO (database, scanning, copying, settings, logging) | Core, Data |
| UI | `src/UI` | Widgets and interaction (main window, table, settings, dialogs) | Core, Data, Services |

Dependencies may only point inward: `UI → Services → Data → Core`. Business layers (`Data`/`Services`/`UI`) must not contain platform conditional compilation; all platform differences are confined to the platform shims in `src/Core`. The entry point is `src/main.cpp`.

```
src/
├── main.cpp                  Entry point: settings, file logging switch, theme, single-instance dispatch
├── Core/
│   ├── MusicProperties.h     Enums, encode/decode helpers, cross-thread metatype registration
│   ├── SettingEntity.h       SettingsData (settings entity with defaults)
│   ├── StringSimilarity.h/.cpp  Title/artist/album similarity comparison
│   ├── SingleInstance.h      Platform shim: Windows named mutex / Linux, macOS lock file
│   └── MSTTagUtils.h         TagLib::FileRef construction wrapper
├── Data/
│   ├── QueryItem.h/.cpp      Music entry model, fuzzy equality, duplicate lookup
│   └── LyricIgnoreRule.h/.cpp  Lyric ignore rule (plain value type)
├── Services/
│   ├── MSTDataSource.h/.cpp      Music metadata database (SQLite)
│   ├── MSTScanController.h/.cpp  Scan flow and concurrency scheduling
│   ├── MSTFileManager.h/.cpp     File copy and free-space checks
│   ├── MSTSettingsManager.h/.cpp settings.json and log file management
│   └── Logger.h/.cpp             Leveled logging with optional file output
└── UI/
    ├── MusicSyncTool/        Main window and its collaborators
    ├── Settings/             Settings dialog
    ├── AddRuleWidget/        Add-ignore-rule dialog
    ├── LoadingPage/          Loading overlay
    ├── OperationResult/      Operation result dialog
    ├── ShowDupe/             Duplicate list dialog
    └── AboutPage/            About page
```

## 2. Core Layer

### 2.1 The `PROPERTIES` namespace (`MusicProperties.h`)

All enums and encode/decode helpers shared across classes live here.

| Enum | Values | Purpose |
| --- | --- | --- |
| `PathType` | `LOCAL` / `REMOTE` | Distinguish local library from device library |
| `PlayState` | `PLAYING` / `PAUSED` / `STOPPED` | Player state |
| `FileErrorType` | `DUPLICATE_FILE` / `LYRIC_NOT_FOUND` / `DISKFULL` | Copy errors |
| `LoadErrorType` | `FILE_NOT_SCANNABLE` / `TAG_READ_ERROR` | Load errors |
| `SortByEnum` | `TITLE` / `ARTIST` / `ALBUM` | Sort field |
| `OrderByEnum` | `ASC` / `DESC` | Sort direction |
| `AppErrorType` | `RUNNING` / `NO_PATH` / `DUPLICATE_FILE` / … | Application-level errors |
| `OperationType` | Scan/copy operation kinds | Result dialog and progress context |
| `RuleField` | `TITLE` / `ARTIST` / `ALBUM` / `NONE` | Rule match field |
| `RuleType` | `INCLUDES` / `EXCLUDES` / `NONE` | Rule match mode |
| `QueryRows` | `ALL` / … | Column set to query |
| `ThemeMode` | `LIGHT` / `DARK` | Light/dark mode |
| `AccentColor` | `BLUE` / `GREEN` / `PURPLE` / `DARK` | Accent colour |

Helpers: `toShort(SortByEnum/OrderByEnum/ThemeMode/AccentColor)` encodes an enum into a persistable `short`; `toSortBy/toOrderBy/toThemeMode/toAccentColor` decodes and falls back to the default value when out of range. The enum-to-`short` mapping is the contract stored in `settings.json` and the numeric values must not be changed casually.

Enums used as cross-thread signal parameters must be registered through `registerMetatypes()` (idempotent); otherwise queued connections from `QtConcurrent` worker threads to the GUI thread silently fail at runtime.

> The first `FileErrorType` enumerator is named `DUPLICATE_FILE` rather than `DUPLICATE`: the Windows SDK header `nb30.h` defines `DUPLICATE` as the macro `0x06`, and `SingleInstance.h` is the very first include of `main.cpp` (pulling in `<Windows.h>`), so using `DUPLICATE` would be macro-replaced and fail to compile.

### 2.2 `SettingsData` (`SettingEntity.h`)

The settings entity and the only structure passed between the UI and the persistence layer. Every member carries a default value so that a missing key or a malformed `settings.json` never leaves uninitialized data.

| Member | Default | Meaning |
| --- | --- | --- |
| `ignoreLyric` | `false` | Skip lyric checking |
| `recursiveScan` | `false` | Scan sub-directories recursively |
| `sortBy` | Title | Sort field code |
| `orderBy` | Ascending | Sort direction code |
| `language` | empty | UI language |
| `favoriteTag` | empty | Custom "favorite" tag |
| `rules` | empty list | Lyric ignore rules |
| `themeMode` | Light | Theme mode code |
| `accentColor` | Green | Accent colour code |
| `logToFile` | `false` | Write logs to file |

### 2.3 `StringSimilarity`

Decides similarity for title/artist/album: non-CJK text uses KMP substring matching for a fast decision, while CJK text uses edit-distance similarity against a stricter threshold. The edit distance uses a rolling array, so memory is O(min(n, m)) instead of O(n·m).

### 2.4 Platform shims

- `SingleInstance.h`: Windows uses a named mutex (`CreateMutex`); Linux/macOS use `/tmp/<appName>.lock` with an `fcntl` write lock. Both return a handle/descriptor that `main.cpp` releases on exit. Platform macros are allowed only here, in `MSTTagUtils.h`, and at the single-instance dispatch in `main.cpp`.
- `MSTTagUtils.h`: wraps `TagLib::FileRef` construction. TagLib calls are allowed only here and in the scan/copy services.

## 3. Data Layer

### 3.1 `QueryItem`

Music entry model (title, artist, album, genre, year, track, file name).

- `operator==` performs fuzzy comparison (title/artist/album via `StringSimilarity`) and is used for de-duplication;
- `findDuplicatesInSorted(const QList<QueryItem>&)` does a single pass over a sorted list and returns the duplicated file names, feeding the duplicate dialog.

### 3.2 `LyricIgnoreRule`

A lyric ignore rule made of `RuleType` + `RuleField` + rule name.

- Plain value type: it does not inherit `QObject` and has no `Q_OBJECT`, so it is freely copyable and container-friendly;
- Provides bidirectional conversion between enums and Chinese strings (`lyricRulesToString` / `ignoreRulesToString` / `stringToIgnoreRules` / `stringToLyricRules`), which is the contract of the `rules` array in `settings.json`;
- Since it no longer inherits `QObject`, its translation context is fixed to `"LyricIgnoreRule"`; callers that used the class `tr()` must now import the `PROPERTIES` names explicitly.

## 4. Services Layer

### 4.1 `MSTDataSource`

Each music directory owns one SQLite connection (`<dir>/musicInfo.db`); the class handles table creation, migration, queries and batch updates.

Threading and locking:

- All database access happens under `dbMutex`; transaction boundaries (batch writes) must be held by a single lock scope, otherwise statements from other threads leak into the transaction;
- `QSqlQuery` instances are indexed per thread id so that each thread owns one, and sharing a query object across threads is forbidden; **`QThreadStorage` is deliberately not used**: main-thread thread-local data is not cleaned up when the `QThreadStorage` object is destroyed, its slot id is reused by later instances, and a new instance would pick up the query object left behind by a previous instance (bound to an already closed connection). These query objects are released explicitly by `closeDB()` and by the destructor;
- `addMusicBatch` rolls back and returns `false` on failure, reporting the failed file names through an output parameter;
- Exposes the page size (`pageSize`, default 200) and batch updates `updateFavoriteBatch` / `updateRuleHitBatch` for favourite and rule-hit state.

Schema and migration:

- Table `musicInfo(title, artist, album, genre, year, track, favorite, ruleHit, fileName)`;
- The version lives in SQLite `user_version`, with `schemaVersion` as the current constant;
- `initTable()` is idempotent: after creating the table it compares versions and adds any missing columns through `ensureColumn`; the new version is written only when every column succeeded, otherwise the old version is kept so the next launch retries.

Query API:

| API | Description |
| --- | --- |
| `getMusicToTable(pageNum, sortBy, orderBy)` | Fetch one page in the requested order |
| `getFavorite(...)` / `getRuleHit(...)` | Favourite page / rule-hit page, with the true total exposed via `getLastFavoriteCount()` |
| `searchMusic(text)` / `searchMusic(text, pageNum)` | Search; the paged variant truncates with `LIMIT/OFFSET` in SQL to keep whole-library hits out of memory |
| `getSearchCount(text)` | Total hit count for pagination |
| `getByFileNames(fileNames)` | Batch fetch by file name; chunked by 500 (SQLite variable limit) using positional binding, so no `":fN"` placeholder names are built |
| `getAllFileNames()` | Reads only the `fileName` column and reserves capacity; used by the scan comparison so each entry avoids a `QueryItem` with five extra empty fields |
| `getCount()` / `getAll(rows)` | Total entry count / full read |

- `addMusicBatch` prepares its single `INSERT` once, outside the loop, and reuses it per row instead of recompiling SQL for every record.

LIKE wildcards (`%`, `_`) in search text are escaped by `escapeLikePattern` and used with `ESCAPE`, so user input is never treated as a wildcard.

### 4.2 `MSTScanController`

The scheduler of the scan flow and the gatekeeper of concurrency discipline.

- `startScan` / `scanConcurrent` / `loadPage`: scan a directory according to `SettingsData` (including `recursiveScan`, which walks sub-directories with `QDirIterator`);
- Scan jobs run on worker threads via `QtConcurrent::run`. Inside a job it is forbidden to touch GUI objects such as `QTableWidget` or `QLabel`, and forbidden to use shared `QSqlQuery` members; results always travel back to the GUI thread through signals;
- Signals: `scanStart`, `scanFinished`, `scanTotal(int)`, `scanCurrent(int)`, `errorOccurred(AppErrorType)`, `loadErrorOccurred(QString, LoadErrorType)`, `tablePageReady(PathType, QList<QueryItem>, unsigned short, unsigned short)`;
- It owns `QFutureWatcher<void> scanWatcher` and exposes `isScanRunning()` / `waitForFinished()`; the window must wait for the job before destruction so a running job never touches destroyed objects;
- `setFavorite` / `setRuleHit` support batch marking with a progress offset; `resetScan` clears the "already scanned" flag (read/written across threads, hence `std::atomic<bool>`).

### 4.3 `MSTFileManager`

File copying and free-space checks.

- `supportedFormat`: the list of supported audio formats; `isFormatSupported(fileName)` tests a name;
- `copyable(filePath)` combines the target directory's free space into the decision; `isFull(source, target)` reports a full disk; `rollBackCopy(fileName)` removes a partial file;
- `copyMusicFiles(source, fileList, target, ignoreLyric, onProgress, onError, onStart, onFinish, onTotal, shouldCancel)`: reports progress, errors and totals through callbacks. Error kinds use `int` codes (`0` = duplicate, `1` = missing lyric, `2` = disk full) which the caller (UI layer) translates into `FileErrorType` and emits back to the GUI thread;
- `shouldCancel` is an optional `const std::atomic<bool>*`: it is read once (relaxed) before each song starts, so no new song begins after it is set and **the current song and its lyric file are always copied completely**; cancelling never rolls back finished files, and the final progress stops at the number of handled songs instead of 100%.

### 4.4 `MSTSettingsManager`

Single owner of the runtime files (`settings.json`, `log/`).

- `baseDir()` defaults to `QCoreApplication::applicationDirPath()` and never depends on the process working directory; tests can redirect it with `setBaseDir()` (empty string restores the default);
- `settingsFilePath()` = `baseDir()/settings.json`; `logDirPath()` = `baseDir()/log`;
- `createDefaultSettings()` generates the default file from the `SettingsData` member defaults, keeping the struct and the file in sync;
- `loadSettings(entity)` keeps the struct defaults for keys missing from older files (`sortBy`, `orderBy`, `themeMode`, `accentColor`), so old settings files need no manual upgrade and never end up with invalid codes such as 0; on failure it creates the default file and returns `false`;
- `saveSettings(entity)` writes all keys plus the `rules` array; a failed write only logs an error and returns `false`, it never terminates the process;
- Log helpers: `getDateFromLog` / `writeLog` / `buildLogFileName` / `cleanLog`.

`settings.json` keys: `ignoreLyric`, `sortBy`, `orderBy`, `language`, `favoriteTag`, `recursiveScan`, `themeMode`, `accentColor`, `logToFile`, `rules[]` (each with `ruleType`, `ruleField`, `ruleName`).

### 4.5 `Logger`

Static logging utility with levels `Info` / `Debug` / `Warn` / `Error` / `Fatal`.

- All output goes through Qt's message system (`qInfo`/`qDebug`/`qWarning`/`qCritical`);
- `setLogToFile(true)` installs the Qt message handler on first use and appends every message to `<baseDir>/log/lastRun.log`; it can be toggled repeatedly and `isLogToFile()` reports the current state;
- `Fatal` logs an unrecoverable error and terminates the process; use it only when the program truly cannot continue.

## 5. UI Layer

### 5.1 `MSTMainWindow`

The main window, and the assembly point of all collaborators as well as the collection point at the thread boundary.

| Member | Description |
| --- | --- |
| `ui` | Generated widget members |
| `translator` | QTranslator |
| `local` / `remote` | Path and data-source holder for the local and device libraries |
| `tableManager` | Table and pagination management |
| `scanController` | Scan scheduling |
| `errorReporter` | Load/copy errors and result dialogs |
| `mediaController` | Playback control |
| `loading` | Loading overlay |
| `entity` | Current settings entity |

Main responsibilities:

- `initDatabase()` / `loadSettings()` / `initUI()`: the assembly order is database → settings → UI;
- `applyTheme(entity)`: three-stage theme loading (see §7), applied immediately after settings are saved;
- `getMusic` / `getMusicConcurrent` / `searchMusic` / `getFavoriteMusic` / `getDuplicatedMusic`: drive scanning, searching, favourite filtering and duplicate listing;
- `copyMusic(source, fileList, target)`: copies on worker threads through `MSTFileManager::copyMusicFiles`; progress and errors travel back to the GUI thread through signals;
- `showSettings()`: creates the settings dialog with `WA_DeleteOnClose` so closing it from the title bar cannot leak the window;
- `popError(AppErrorType)` / `showOperationResult(OperationType)`: error and result prompts;
- Destruction first waits for scan/copy jobs, then releases the data sources and widgets.

### 5.2 Other UI classes

| Class | Responsibility |
| --- | --- |
| `MSTTableManager` | Controller of both tables: pagination and total pages, current page, favourite-only switch, row filling and selection |
| `MSTErrorReporter` | Keeps the load/copy error list and the operation result dialog, translating error codes into localized text |
| `MSTMediaController` | Play/pause, progress and volume interaction, now-playing title; playback state is reported as `PlayState` |
| `Settings` | Settings dialog: language, sorting, ignore lyrics, recursive scan, file logging, theme (mode + accent colour) and the rule table; returns `confirmPressed(SettingsData)` |
| `AddRuleWidget` | Adds a lyric ignore rule; returns `sendRules(LyricIgnoreRule)` |
| `LoadingPage` | Loading overlay and progress display for scan/copy; `setCancelable(true)` shows a Cancel button under the progress bar and emits `cancelRequested()`, while scan tasks keep it hidden |
| `OperationResult` | Operation result list dialog |
| `ShowDupe` | Duplicate list dialog |
| `AboutPage` | About page |

## 6. Threading Model

| Scenario | Thread | Contract |
| --- | --- | --- |
| Directory scan, tag reading, file copy | `QtConcurrent` worker thread | IO and computation only; results are queued back to the GUI thread through signals |
| Database access | Both worker and GUI threads share the connection | Serialized by `MSTDataSource::dbMutex`; one `QSqlQuery` per thread |
| Widget refresh, dialogs, table filling | GUI thread | Only inside slots |
| Job lifetime | GUI thread | `QFutureWatcher` owns the job; `waitForFinished()` before destruction/close |

Cross-thread signal parameters must already be registered in `PROPERTIES::registerMetatypes()`, and parameter types must be integral or an already declared metatype so moc can resolve them.

## 7. Theme and Stylesheets

Stylesheets are concatenated in a fixed order and applied with a single `setStyleSheet`; a missing segment is skipped so the UI stays usable:

1. `:/MusicSyncTool/base-style.qss` (base styles)
2. Mode: `style-light.qss` or `style-dark-mode.qss`
3. Accent: `style.qss` (blue) / `style-green.qss` / `style-purple.qss` / `style-dark.qss`

The theme comes from `themeMode` and `accentColor` in `settings.json`. The two combo boxes in the `Settings` dialog store the encoded value directly in `itemData`, so they do not depend on item order.

## 8. Runtime Data

| File | Location | Description |
| --- | --- | --- |
| `settings.json` | `applicationDirPath()` | User settings, keys in §4.4 |
| `musicInfo.db` | one per music directory | Metadata cache, schema version in §4.1 |
| `log/lastRun.log` | `applicationDirPath()/log` | Run log when `logToFile` is enabled |
| `translations/langinfo.json` | program directory | Data source for the language combo box |
| `translations/*.qm` | program directory | Translation files (`Translation_zh_Hans` / `Translation_en_US` / `Translation_Wenyan`) |

## 9. Build and Tests

- Windows: open `MusicSyncTool.sln` with Visual Studio 2022; dependencies are provided by vcpkg (`taglib`, `utfcpp`); configurations are Debug/Release.
- Linux / macOS: `CMakeLists.txt` provides a CMake build (`BUILD_TESTS` enables unit tests); CMake is not used for Windows builds.
- Unit tests: the `Test/` directory is based on Qt Test with `Test/main.cpp` as entry point; each test class implementation is included by that entry point.
