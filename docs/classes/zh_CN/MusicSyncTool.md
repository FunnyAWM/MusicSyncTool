# MusicSyncTool：程序主界面、核心操作类

## 1. 成员列表

| 类型                     | 成员              | 描述              |
|:-----------------------|:----------------|-----------------|
| Ui::MusicSyncToolClass | ui              | 程序主界面组件类        |
| QTranslator*           | translator      | 待加载翻译器（全局）      |
| QString                | localPath       | 本地路径            |
| QString                | remotePath      | 设备路径            |
| QSqlDatabase           | dbLocal         | 本地数据库连接         |
| QSqlDatabase           | dbRemote        | 设备数据库连接         |
| QSqlQuery              | queryLocal      | 本地数据库查询         |
| QSqlQuery              | queryRemote     | 设备数据库查询         |
| LoadingPage*           | loading         | 加载界面组件（全局）      |
| unsigned short         | currentPage[2]  | 当前页码            |
| unsigned short         | totalPage[2]    | 总页码数            |
| bool                   | favoriteOnly[2] | 是否仅显示喜爱的音乐（指示位） |
| const short            | PAGESIZE        | 每页显示行数          |
| set                    | entity          | 设置实体类           |
| QMediaPlayer           | *mediaPlayer    | 媒体播放器类（全局）      |
| QAudioOutput           | *audioOutput    | 音频输出类（全局）       |
| QString                | nowPlaying      | 正在播放文件名         |
| QStringList            | errorList       | 加载及复制的错误列表      |
| const QStringList      | supportedFormat | 支持的格式清单         |

## 2. 方法列表

- explicit MusicSyncTool(QWidget* parent = nullptr)
  - 参数列表：
    - QWidget* parent：父组件指针（一般应为空指针）
  - 描述：初始化界面并设定数据库连接

- ~MusicSyncTool() override
  - 描述：关闭数据库连接
- void addToErrorList(const QString&, FileErrorType)
  - 参数列表：
    - const QString & file：出现复制错误的文件名
    - FileErrorType error：复制错误类型
  - 描述：将出现复制错误的文件名加入到错误列表
- void addToErrorList(const QString&, LoadErrorType)
  - 参数列表：
    - const QString & file：出现加载错误的文件名
    - LoadErrorType error：加载错误类型
  - 描述：将出现读取错误的文件名加入到错误列表
- static void cleanLog()
  - 描述：清理日志文件
- void connectSlots() const
  - 描述：连接动态定义的组件信号和槽
- void copyMusic(const QString&, const QStringList&, const QString&)
  - 参数列表：
    - const QString& source：源文件所在目录
    - const QStringList& fileList：要复制的文件列表
    - const QString& target：复制操作的目标目录
  - 描述：从源文件目录复制列表中文件到目标目录（不满足复制条件的加入错误列表）
- void endMedia(QMediaPlayer::PlaybackState) const
  - 参数列表：
    - const QMediaPlayer::PlaybackState state：要设定的播放状态
  - 描述：停止播放并刷新播放器部分（媒体结束时调用）
- [[nodiscard]] QString getLanguage() const
  - 返回：设置的语言（可忽略）
  - 描述：从设置实体中取得语言设定
- QStringList getDuplicatedMusic(PathType)
  - 返回：重复的文件列表
  - 参数列表：
    - PathType path：
- QStringList getSelectedMusic(PathType);
- void getFavoriteMusic(PathType, unsigned short);
- void getMusic(PathType, unsigned short);
- void getMusicConcurrent(PathType, unsigned short);
- void initDatabase();
- void initMediaPlayer() const;
- void initUI();
- [[nodiscard]] bool isFormatSupported(const QString&) const;
- static bool isFull(const QString&, const QString&);
- static bool isRuleHit(const LyricIgnoreRuleSingleton&, const TagLib::Tag*);
- void loadLanguage();
- void loadSettings();
- static void loadDefaultSettings();
- void openFolder(PathType);
- void popError(PET);
- void searchMusic(PathType, const QString&);
- void setAvailableSpace(PathType) const;
- void setMediaWidget(PlayState) const;
- void setNowPlayingTitle(const QString&) const;
- void setSliderPosition(qint64) const;
- void setTotalLength(PathType, int);
- void showOperationResult(OperationType);
- void showSettings() const;
- void setFavorite(const QStringList&, PathType, const TagLib::String&, const QDateTime&);
- void setRuleHit(const QStringList&, PathType, const QList<LyricIgnoreRuleSingleton>&, const QDateTime&);
- static QDateTime getDateFromLog(const QString&);
- static void writeLog(const QString&, const QDateTime&);