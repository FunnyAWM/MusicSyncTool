/**
 * @file MusicSyncTool.cpp
 * @brief 音乐同步工具主窗口类的实现文件
 * @details 实现音乐文件扫描、管理、播放和同步的核心功能，包括用户界面初始化、
 *          数据库操作、文件处理、多线程扫描、媒体播放控制等功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 * @note 使用Qt框架和TagLib库进行音频文件元数据处理
 */

// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#pragma warning(disable : 6031)
#include "MusicSyncTool.h"
#include <algorithm>
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "../../Logger.h"

#if defined(__linux)
#include <unistd.h>
#endif

/**
 * @brief 音乐同步工具主窗口构造函数
 * @param parent 父窗口部件指针，默认为nullptr
 * @details 初始化音乐同步工具主窗口，包括数据库连接、设置加载、
 *          用户界面初始化、语言加载、媒体播放器初始化和信号槽连接
 */
MusicSyncTool::MusicSyncTool(QWidget* parent) : // NOLINT(*-pro-type-member-init)
    QMainWindow(parent), translator(new QTranslator(this)), player(new MSTMediaPlayer(this)) {
    initDatabase();
    loadSettings();
    initUI();
    loadLanguage();
    initMediaPlayer();
    connectSlots();
    setMediaWidget(PlayState::STOPPED);
}

/**
 * @brief 音乐同步工具主窗口析构函数
 * @details 清理资源，包括关闭数据库连接、删除动态分配的对象，
 *          确保程序正常退出时没有内存泄漏
 */
MusicSyncTool::~MusicSyncTool() {
    if (local.isOpen()) {
        local.closeDB();
    }
    if (remote.isOpen()) {
        remote.closeDB();
    }
    delete copyStats;
    player.reset();
    delete loading;
}

/**
 * @brief 初始化数据库连接
 * @details 为本地和远程数据源设置不同的连接名称，用于区分不同的数据库实例
 *          本地数据库用于存储本地路径的音乐文件信息，远程数据库用于存储远程路径的音乐文件信息
 */
void MusicSyncTool::initDatabase() {
    local.setConnectionName("local");
    remote.setConnectionName("remote");
}

/**
 * @brief 从设置文件加载用户配置
 * @details 读取settings.json文件并解析其中的配置信息，包括：
 *          - 是否忽略歌词
 *          - 排序方式和排序顺序
 *          - 界面语言设置
 *          - 喜爱标签设置
 *          - 歌词忽略规则列表
 *          如果文件不存在或格式错误，将加载默认设置
 */
void MusicSyncTool::loadSettings() {
    QFile file("settings.json");
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::Warn("No settings file found, creating default setting file named settings.json");
        file.close();
        loadDefaultSettings();
        return;
    }
    const QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
    if (settings.isNull()) {
        Logger::Warn("No settings file found, creating default setting file named settings.json");
        file.close();
        loadDefaultSettings();
        return;
    }
    QJsonObject obj = settings.object();
    entity.ignoreLyric = obj["ignoreLyric"].toBool();
    entity.sortBy = static_cast<short>(obj["sortBy"].toInt());
    entity.orderBy = static_cast<short>(obj["orderBy"].toInt());
    entity.language = obj["language"].toString();
    entity.favoriteTag = obj["favoriteTag"].toString();
    QList<LyricIgnoreRule> rules;
    QJsonArray rulesArray = obj["rules"].toArray();
    for (QJsonValue rule : rulesArray) {
        QJsonObject ruleObj = rule.toObject();
        rules.append(LyricIgnoreRule(LyricIgnoreRule::stringToIgnoreRules(ruleObj["ruleType"].toString()),
                                     LyricIgnoreRule::stringToLyricRules(ruleObj["ruleField"].toString()),
                                     ruleObj["ruleName"].toString()));
    }
    entity.rules = rules;
    file.close();
}

/**
 * @brief 加载默认设置（当settings.json文件不存在时）
 * @details 创建默认的设置文件，包含以下默认值：
 *          - ignoreLyric: false（不忽略歌词）
 *          - sortBy: 按标题排序
 *          - orderBy: 升序排列
 *          - language: 空字符串（将使用中文作为默认语言）
 *          - favoriteTag: 空字符串
 *          - rules: 空的规则数组
 */
void MusicSyncTool::loadDefaultSettings() {
    QFile file("settings.json");
    file.open(QIODevice::WriteOnly);
    QJsonObject obj;
    obj["ignoreLyric"] = false;
    obj["sortBy"] = toShort(SortByEnum::TITLE);
    obj["orderBy"] = toShort(OrderByEnum::ASC);
    obj["language"] = "";
    obj["favoriteTag"] = "";
    obj["rules"] = QJsonArray();
    file.write(QJsonDocument(obj).toJson());
    file.close();
}

/**
 * @brief 初始化媒体播放器模块
 * @details 设置媒体播放器的初始音量为50%（0.5），
 *          为后续的音频播放功能做准备
 */
void MusicSyncTool::initMediaPlayer() const { player->setVolume(0.5); }

/**
 * @brief 根据设置加载语言文件
 * @details 读取translations/langinfo.json配置文件，根据用户设置的语言加载相应的翻译文件。
 *          如果用户未设置语言，默认使用中文。加载翻译文件后，安装翻译器并重新翻译界面文本。
 *          支持的语言包括中文、英文等多种语言。
 */
void MusicSyncTool::loadLanguage() {
    QFile file(QApplication::applicationDirPath() + "/translations/langinfo.json");
    if (!file.open(QIODevice::ReadOnly)) {
        popError(PET::NOLANG);
        Logger::Fatal("Error opening langinfo.json:" + file.errorString());
        exit(EXIT_FAILURE);
    }
    const QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonArray langArray = langinfo.array();
    for (QJsonValue lang : langArray) {
        if (entity.language == "") {
            entity.language = "中文";
        }
        QJsonObject langObj = lang.toObject();
        if (langObj["lang"].toString() == entity.language) {
            // ReSharper disable once CppNoDiscardExpression
            translator->load("translations/" + langObj["fileName"].toString());
            break;
        }
    }
    qApp->installTranslator(translator);
    ui.retranslateUi(this);
}

/**
 * @brief 初始化用户界面组件
 * @details 设置主窗口的用户界面，包括：
 *          - 加载UI布局文件
 *          - 设置窗口图标
 *          - 配置表格控件的列宽自适应模式
 *          - 初始化音量滑块和标签的默认值（50%）
 *          - 设置播放状态显示文本
 */
void MusicSyncTool::initUI() {
    ui.setupUi(this);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
    ui.tableWidgetLocal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.tableWidgetRemote->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.volumeSlider->setValue(50);
    ui.volumeLabel->setText(tr("音量：") + "50%");
    ui.nowPlaying->setText(tr("播放已结束。"));
}

/**
 * @brief 根据错误类型弹出错误对话框
 * @param type 错误类型枚举值
 * @details 根据不同的错误类型显示相应的错误消息对话框：
 *          - NOAUDIO: 没有选定音频文件
 *          - NPS: 没有选定路径
 *          - NFT: 没有设置喜爱标签
 *          - NFS: 没有选定文件
 *          - FIRST: 已经是第一页
 *          - LAST: 已经是最后一页
 *          - RUNNING: 程序已在运行
 *          - NOLANG: 找不到语言配置文件
 *          - DBERROR: 数据库操作错误
 */
void MusicSyncTool::popError(const PET type) {
    switch (type) {
    case PET::NOAUDIO:
        QMessageBox::critical(this, tr("错误"), tr("没有选定音频！（提示：可以通过双击表格中的歌曲来预览）"));
        break;
    case PET::NPS:
        QMessageBox::critical(this, tr("错误"), tr("没有选定路径！"));
        break;
    case PET::NFT:
        QMessageBox::critical(this, tr("错误"), tr("没有设置喜爱标签！请在设置中指定！"));
        break;
    case PET::NFS:
        QMessageBox::critical(this, tr("错误"), tr("没有选定文件！"));
        break;
    case PET::FIRST:
        QMessageBox::information(this, tr("提示"), tr("已经是第一页了！"));
        break;
    case PET::LAST:
        QMessageBox::information(this, tr("提示"), tr("已经是最后一页了！"));
        break;
    case PET::RUNNING:
        QMessageBox::critical(this, tr("错误"), tr("程序已在运行！"));
        break;
    case PET::NOLANG:
        QMessageBox::critical(this, tr("错误"), tr("找不到程序语言配置文件，程序即将退出！"));
        break;
    case PET::DBERROR:
        QMessageBox::critical(this, tr("错误"), tr("操作数据库中数据时出现严重错误，程序即将退出！"));
        break;
    default:
        break;
    }
}

/**
 * @brief 根据播放状态设置媒体播放器控件
 * @param state 播放状态枚举值
 * @details 根据播放状态更新用户界面的播放控制按钮和显示信息：
 *          - PLAYING: 设置按钮文本为"暂停"，显示当前播放的歌曲信息
 *          - PAUSED: 设置按钮文本为"播放"
 *          - STOPPED: 设置按钮文本为"播放"，显示"播放已结束"
 *          同时更新音量标签显示当前音量百分比
 */
void MusicSyncTool::setMediaWidget(const PlayState state) const {
    if (state == PlayState::PLAYING) {
        ui.playControl->setText(tr("暂停"));
        setNowPlayingTitle(player->getNowPlaying());
    } else if (state == PlayState::PAUSED) {
        ui.playControl->setText(tr("播放"));
    } else if (state == PlayState::STOPPED) {
        ui.playControl->setText(tr("播放"));
        ui.nowPlaying->setText(tr("播放已结束。"));
    }
    ui.volumeLabel->setText(tr("音量：") + QString::number(ui.volumeSlider->value()) + "%");
}

/**
 * @brief 弹出文件夹选择对话框并将路径加载到数据库
 * @param path 路径类型（本地或远程）
 * @details 打开文件夹选择对话框让用户选择音乐文件夹路径。
 *          根据路径类型创建相应的文件管理器和数据库连接：
 *          - LOCAL: 设置本地路径，创建本地文件管理器，打开本地数据库
 *          - REMOTE: 设置远程路径，创建远程文件管理器，打开远程数据库
 *          如果数据库打开失败，程序将退出
 */
void MusicSyncTool::openFolder(const PathType path) {
    const QString dir = QFileDialog::getExistingDirectory();
    if (dir == "") {
        return;
    }
    (path == PathType::LOCAL ? local.getPath() : remote.getPath()) = dir;
    if (path == PathType::LOCAL) {
        local.setPath(dir);
        localManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
        if (!local.openDB()) {
            exit(EXIT_FAILURE);
        }
    } else {
        remote.setPath(dir);
		remoteManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
        if (!remote.openDB()) {
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief 从选定路径获取音乐文件（多线程调用入口）
 * @param path 路径类型（本地或远程）
 * @param page 页码
 * @details 这是音乐文件扫描的多线程入口函数。检查是否已选择路径，
 *          然后启动并发线程执行实际的音乐文件扫描和加载操作，
 *          避免阻塞用户界面
 */
void MusicSyncTool::getMusic(PathType path, unsigned short page) {
    Logger::Info("Scanning started");
    if (local.getPath() == "" && remote.getPath() == "") {
        Logger::Warn("No path selected");
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::getMusicConcurrent, this, path, page);
}

/**
 * @brief 从选定路径获取音乐文件并加载到表格（主要操作，并发执行）
 * @param path 路径类型（本地或远程）
 * @param page 页码
 * @details 核心的音乐文件扫描和处理函数，执行以下操作：
 *          1. 初始化数据库表和日志文件名
 *          2. 扫描目录获取新文件列表，过滤不支持的格式
 *          3. 比较新旧文件列表，识别需要删除和添加的文件
 *          4. 删除不存在的文件记录，添加新文件到数据库
 *          5. 设置喜爱标签和规则匹配
 *          6. 计算分页信息并加载数据到表格控件
 *          7. 发出进度信号供UI更新
 */
void MusicSyncTool::getMusicConcurrent(const PathType path, const unsigned short page) {
    const clock_t start = clock();
	MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    emit ds.loadStarted();
    favoriteOnly[path == PathType::LOCAL ? 0 : 1] = false;
    QStringList pathForLog = (path == PathType::LOCAL ? local.getPath() : remote.getPath()).split("/");
    QString logFileNameBuilder = "lastScan";
    if (!pathForLog[0].isEmpty()) {
        pathForLog[0].remove(":");
    } else {
        pathForLog.removeFirst();
    }
    for (const QString& tempStr : pathForLog) {
        logFileNameBuilder += " - " + tempStr;
    }
    logFileNameBuilder += ".log";
    logFileNameBuilder = QCoreApplication::applicationDirPath() + "/log/" + logFileNameBuilder;
    QDateTime dateTime = getDateFromLog(logFileNameBuilder);
    const QDir dir(path == PathType::LOCAL ? local.getPath() : remote.getPath());
    
    ds.initTable();
    QStringList newFileList = dir.entryList(QDir::Files);
    for (const QString& file : newFileList) {
        if (!MSTFileManager::isFormatSupported(file)) { // Remove unsupported files
            newFileList.removeOne(file);
        }
    }
    QList<QueryItem> tempList = ds.getAll({QueryRows::FILENAME});
    QStringList oldFileList;
    for (const QueryItem& item : tempList) {
        oldFileList.append(item.getFileName());
    }
    newFileList.sort();
    oldFileList.sort();
    QStringList deleteList;
    for (int i = 0; i < oldFileList.size(); i++) {
        if (!newFileList.contains(oldFileList.at(i))) {
            deleteList.append(oldFileList.at(i));
            oldFileList.removeAt(i);
            i = i - 2 < -1 ? -1 : i - 2;
        }
    }
    for (int i = 0; i < newFileList.size(); i++) {
        if (oldFileList.contains(newFileList.at(i)) || !MSTFileManager::isFormatSupported(newFileList.at(i))) {
            newFileList.removeAt(i);
            i = i - 2 < 0 ? -1 : i - 2;
        }
    }
    if (!ds.deleteMusic(deleteList)) {
        popError(PET::DBERROR);
        exit(EXIT_FAILURE);
    }
    newFileList.removeOne("musicInfo.db");
    emit total(newFileList.size() + oldFileList.size());
    emit current(0);
    const TagLib::String key(entity.favoriteTag.toStdString());
    for (int i = 0; i < newFileList.size(); i++) {
        emit current(i);
        QString file = newFileList.at(i).toUtf8();
        if (!MSTFileManager::isFormatSupported(file)) {
            continue;
        }
        if (!ds.addMusic(file)) {
            Logger::Warn("Error adding music: " + file);
            addToErrorList(file, LoadErrorType::FNS);
        }
    }
    setFavorite(path, key, dateTime);
    setRuleHit(path, entity.rules, dateTime);
    totalPage[path == PathType::LOCAL ? 0 : 1] = static_cast<short>(ds.getCount() / ds.getPageSize() + 1);
    const auto lastPageSize = static_cast<short>(ds.getCount() % ds.getPageSize());
    emit total(ds.getPageSize());
    QList<QueryItem> items =
        ds.getMusicToTable(page, static_cast<SortByEnum>(entity.sortBy), static_cast<OrderByEnum>(entity.orderBy));
    QTableWidget* targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
    targetTable->clearContents();
    if (page == totalPage[(path == PathType::LOCAL ? 0 : 1)]) {
        targetTable->setRowCount(lastPageSize);
    } else {
        targetTable->setRowCount(ds.getPageSize());
    }
    int progress = 0;
    for (const QueryItem& item : items) {
        emit current(progress);
        targetTable->setItem(progress, 0, new QTableWidgetItem(item.getTitle()));
        targetTable->setItem(progress, 1, new QTableWidgetItem(item.getArtist()));
        targetTable->setItem(progress, 2, new QTableWidgetItem(item.getAlbum()));
        targetTable->setItem(progress, 3, new QTableWidgetItem(item.getGenre()));
        targetTable->setItem(progress, 4, new QTableWidgetItem(QString::number(item.getYear())));
        targetTable->setItem(progress, 5, new QTableWidgetItem(QString::number(item.getTrack())));
        progress++;
    }
    const clock_t end = clock();
    Logger::Info("Scanning finished in " + QString::number(static_cast<double>(end - start) / CLOCKS_PER_SEC) + " seconds");
    (path == PathType::LOCAL ? ui.pageLocal : ui.pageRemote)
        ->setText(QString::number(currentPage[(path == PathType::LOCAL ? 0 : 1)]) + "/" +
                  QString::number(totalPage[(path == PathType::LOCAL ? 0 : 1)]));
    dateTime = QDateTime::currentDateTime();
    writeLog(logFileNameBuilder, dateTime);
    emit ds.loadFinished();
}

/**
 * @brief 根据文本搜索音乐文件
 * @param path 路径类型（本地或远程）
 * @param text 搜索文本
 * @details 在指定的数据源中搜索包含指定文本的音乐文件。
 *          如果搜索文本为空，则重新加载所有音乐文件到第一页。
 *          搜索结果将显示在相应的表格控件中，包括标题、艺术家、
 *          专辑、流派、年份和音轨号等信息
 */
void MusicSyncTool::searchMusic(const PathType path, const QString& text) {
    if (text == "") {
        getMusic(path, 1);
        return;
    }
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    QTableWidget* targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
    targetTable->clearContents();
    const QList<QueryItem> items = ds.searchMusic(text);
    targetTable->setRowCount(items.count());
    for (int i = 0; i < items.count(); i++) {
        targetTable->setItem(i, 0, new QTableWidgetItem(items.at(i).getTitle()));
        targetTable->setItem(i, 1, new QTableWidgetItem(items.at(i).getArtist()));
        targetTable->setItem(i, 2, new QTableWidgetItem(items.at(i).getAlbum()));
        targetTable->setItem(i, 3, new QTableWidgetItem(items.at(i).getGenre()));
        targetTable->setItem(i, 4, new QTableWidgetItem(QString::number(items.at(i).getYear())));
        targetTable->setItem(i, 5, new QTableWidgetItem(QString::number(items.at(i).getTrack())));
    }
}

/**
 * @brief 将文件添加到错误列表（文件操作错误）
 * @param file 文件名
 * @param error 文件错误类型
 * @details 根据不同的文件操作错误类型生成相应的错误信息并添加到错误列表：
 *          - DUPLICATE: 文件已存在
 *          - LNF: 找不到歌词文件
 *          - DISKFULL: 磁盘已满
 *          错误信息将包含操作类型和具体的错误原因
 */
void MusicSyncTool::addToErrorList(const QString& file, const FileErrorType error) {
    switch (error) {
    case FileErrorType::DUPLICATE:
        errorList.append(tr("复制") + file + tr("失败：文件已存在"));
        break;
    case FileErrorType::LNF:
        errorList.append(tr("复制") + file + tr("失败：找不到歌词文件"));
        break;
    case FileErrorType::DISKFULL:
        errorList.append(tr("复制") + file + tr("失败：磁盘已满"));
        break;
    }
}

/**
 * @brief 将文件添加到错误列表（加载错误）
 * @param file 文件名
 * @param error 加载错误类型
 * @details 根据不同的文件加载错误类型生成相应的错误信息并添加到错误列表：
 *          - FNS: 文件不可扫描
 *          - TAGERR: 标签错误
 *          错误信息将包含操作类型和具体的错误原因
 */
void MusicSyncTool::addToErrorList(const QString& file, const LoadErrorType error) {
    switch (error) {
    case LoadErrorType::FNS:
        errorList.append(tr("加载") + file + tr("失败：文件不可扫描"));
        break;
    case LoadErrorType::TAGERR:
        errorList.append(tr("加载") + file + tr("失败：标签错误"));
        break;
    }
}

/**
 * @brief 获取重复的音乐文件并显示
 * @param path 路径类型（本地或远程）
 * @return 重复音乐文件的文件名列表
 * @details 检查指定路径下的音乐文件，识别标题和艺术家相同的重复文件。
 *          如果未选择路径，将显示错误提示。通过ShowDupe对话框展示
 *          重复文件列表，让用户选择要删除的重复文件
 */
QStringList MusicSyncTool::getDuplicatedMusic(const PathType path) {
    const QString selectedPath = path == PathType::LOCAL ? local.getPath() : remote.getPath();
    if (selectedPath == "") {
        Logger::Warn("No path selected");
        popError(PET::NPS);
        return {};
    }
    ShowDupe dp;
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    QList<QueryItem> items = ds.getAll();
    const QueryItem* slow = &items.first();
    const QueryItem* fast = nullptr;
    QStringList dupeList;
    for (auto it = items.begin(); it != items.end(); ++it) {
        fast = &*it;
        if (*slow == *fast) {
            dupeList.append(slow->getFileName());
            dupeList.append(fast->getFileName());
        }
        slow = fast;
    }
    for (const auto& i : dupeList) {
        Logger::Info("Found duplicated music named " + i + " at " + (path == PathType::LOCAL ? local.getPath() : remote.getPath()));
        dp.add(i);
    }
    dp.exec();
    return dupeList;
}

/**
 * @brief 获取选中的音乐文件
 * @param path 路径类型（本地或远程）
 * @return 选中音乐文件的文件名列表
 * @details 从指定的表格控件中获取用户选中的音乐文件行，
 *          提取选中行的标题、艺术家和专辑信息，然后通过数据源
 *          查询对应的文件名列表。如果没有选中任何行或表格为空，
 *          返回空列表
 */
QStringList MusicSyncTool::getSelectedMusic(const PathType path) {
    QSet<int> selectedRows;
    const QTableWidget* const table = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
    if (table->rowCount() == 0) {
        return {};
    }
    for (int i = 0; i < table->rowCount(); i++) {
        if (table->item(i, 0)->isSelected()) {
            selectedRows.insert(i);
        }
    }
    QStringList titleList;
    QStringList artistList;
    QStringList albumList;
    for (const int& i : selectedRows) {
        titleList.append(table->item(i, 0)->text());
        artistList.append(table->item(i, 1)->text());
        albumList.append(table->item(i, 2)->text());
    }
    if (selectedRows.empty()) {
        return {};
    }
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    QList<QueryItem> items;
    QueryItem item;
    for (int i = 0; i < titleList.count(); i++) {
        item.setTitle(titleList.at(i));
        item.setArtist(artistList.at(i));
        item.setAlbum(albumList.at(i));
        items.append(item);
    }
    return ds.getFileNameByMD(items);
}

/**
 * @brief 显示设置对话框
 * @details 创建并显示设置页面对话框，连接确认信号到保存设置的槽函数，
 *          允许用户修改应用程序的各种配置选项，包括语言、喜爱标签、
 *          排序方式和歌词忽略规则等
 */
void MusicSyncTool::showSettings() const {
    const auto page = new Settings();
    connect(page, SIGNAL(confirmPressed(set)), this, SLOT(saveSettings(set)));
    page->show();
}

/**
 * @brief 设置音乐文件的喜爱标签
 * @param path 路径类型（本地或远程）
 * @param key 喜爱标签的键值
 * @param dateTime 扫描时间
 * @details 如果用户在设置中指定了喜爱标签，则在对应的数据源中
 *          设置包含该标签的音乐文件为喜爱状态。这个操作通常在
 *          音乐文件扫描完成后执行
 */
void MusicSyncTool::setFavorite(const PathType path, const TagLib::String& key,
                                const QDateTime& dateTime) {
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    if (entity.favoriteTag != "") {
        ds.setFavorite(key.toCString(), dateTime);
    }
}

/**
 * @brief 设置规则命中状态
 * @param path 路径类型（本地或远程）
 * @param rules 歌词忽略规则列表
 * @param dateTime 扫描时间
 * @details 如果用户设置了歌词忽略规则，则在对应的数据源中
 *          检查音乐文件是否匹配这些规则，并设置相应的命中状态。
 *          这个操作通常在音乐文件扫描完成后执行
 */
void MusicSyncTool::setRuleHit(const PathType path, const QList<LyricIgnoreRule>& rules,
                               const QDateTime& dateTime) {
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    if (!rules.isEmpty()) {
        ds.setRuleHit(rules, dateTime);
    }
}

/**
 * @brief 从日志文件获取上次扫描时间
 * @param log 日志文件路径
 * @return 上次扫描的时间，如果没有找到则返回1970年1月1日
 * @details 读取指定的日志文件获取上次音乐文件扫描的时间戳。
 *          如果日志目录不存在则创建目录。如果日志文件不存在或
 *          时间格式无效，返回Unix时间戳起始时间，表示需要扫描所有文件
 */
QDateTime MusicSyncTool::getDateFromLog(const QString& log) {
    QFile file(log);
    QDateTime dateTime;
    const QString logDir = QCoreApplication::applicationDirPath() + "/log";
    const QDir dir(logDir);
    if (!dir.exists()) {
        // ReSharper disable once CppExpressionWithoutSideEffects
        dir.mkpath(logDir);
    }
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        dateTime = QDateTime::fromString(in.readLine());
        if (dateTime.isNull()) {
            Logger::Warn("No last scan log found, scanning all files");
            dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
        }
    } else {
        Logger::Warn(file.errorString());
        Logger::Warn("No last scan log found, scanning all files");
        // Set last scan time to 1970-01-01 00:00:00 if no log found
        dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
    }
    return dateTime;
}

/**
 * @brief 写入日志文件
 * @param log 日志文件路径
 * @param dateTime 要记录的时间
 * @details 将当前扫描完成的时间写入指定的日志文件，
 *          用于下次启动时判断哪些文件需要重新扫描
 */
void MusicSyncTool::writeLog(const QString& log, const QDateTime& dateTime) {
    QFile file(log);
    QTextStream out(&file);
    if (file.open(QIODevice::WriteOnly)) {
        out << dateTime.toString();
    }
    file.close();
}

/**
 * @brief 保存设置到设置文件
 * @param entityParam 设置实体对象
 * @details 将用户修改的设置保存到settings.json文件中，包括：
 *          - 歌词忽略设置
 *          - 排序方式和顺序
 *          - 界面语言
 *          - 喜爱标签
 *          - 歌词忽略规则列表
 *          如果排序设置发生变化，会触发界面刷新以应用新的排序
 */
void MusicSyncTool::saveSettings(const set& entityParam) {
    QFile file("settings.json");
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::Fatal("Error opening settings file");
        QMessageBox::critical(this, tr("错误"), tr("无法打开设置文件"));
        return;
    }
    QJsonObject obj;
    obj["ignoreLyric"] = entityParam.ignoreLyric;
    obj["sortBy"] = entityParam.sortBy;
    obj["orderBy"] = entityParam.orderBy;
    obj["language"] = entityParam.language;
    obj["favoriteTag"] = entityParam.favoriteTag;
    QJsonArray rulesArray;
    for (const auto& rule : entityParam.rules) {
        QJsonObject ruleObj;
        ruleObj["ruleType"] = rule.getRuleTypeStr();
        ruleObj["ruleField"] = rule.getRuleFieldStr();
        ruleObj["ruleName"] = rule.getRuleName();
        rulesArray.append(ruleObj);
    }
    obj["rules"] = rulesArray;
    const int tempSort = this->entity.sortBy;
    const int tempOrder = this->entity.orderBy;
    const QList<LyricIgnoreRule> tempRules = this->entity.rules;
    const QString tempTag = this->entity.favoriteTag;
    this->entity = entityParam;
    if (tempSort != entityParam.sortBy || tempOrder != entityParam.orderBy) {
        if (local.getPath() != "") {
            getMusic(PathType::LOCAL, 1);
        }
        if (remote.getPath() != "") {
            getMusic(PathType::REMOTE, 1);
        }
    }
    if (tempRules != entity.rules || entityParam.favoriteTag != tempTag) {
        cleanLog();
        if (local.getPath() != "") {
            getMusic(PathType::LOCAL, 1);
        }
        if (remote.getPath() != "") {
            getMusic(PathType::REMOTE, 1);
        }
    }
    loadLanguage();
    Logger::Info("IgnoreLyric: " + QString::number(this->entity.ignoreLyric));
    Logger::Info("SortBy: " + QString::number(this->entity.sortBy));
    Logger::Info("OrderBy: " + QString::number(this->entity.orderBy));
    Logger::Info("Language: " + this->entity.language);
    Logger::Info("FavoriteTag: " + this->entity.favoriteTag);
    int i = 1;
    for (const auto& rule : entityParam.rules) {
        Logger::Info("Rule No." + QString::number(i++) + ":");
        Logger::Info("RuleTypeConverter: " + rule.getRuleTypeStr());
        Logger::Info("RuleFieldConverter: " + rule.getRuleFieldStr());
        Logger::Info("Rule: " + rule.getRuleName());
    }
    QJsonDocument settings;
    settings.setObject(obj);
    file.write(settings.toJson());
    file.close();
}

/**
 * @brief 复制音乐文件从源路径到目标路径
 * @param source 源路径
 * @param fileList 要复制的文件列表
 * @param target 目标路径
 * @details 执行音乐文件的复制操作，包括以下功能：
 *          1. 创建目标目录（如果不存在）
 *          2. 逐个复制文件列表中的音乐文件
 *          3. 同时复制对应的歌词文件（.lrc）
 *          4. 检查磁盘空间，处理磁盘满等错误情况
 *          5. 根据设置决定是否忽略歌词文件
 *          6. 发出进度信号更新UI
 *          7. 处理文件已存在、找不到歌词等错误情况
 */
void MusicSyncTool::copyMusic(const QString& source, const QStringList& fileList, const QString& target) {
    const TagLib::String key = "LYRICS";
    emit started();
    emit total(fileList.size());
    const QDir dir(target);
    if (dir.isEmpty()) {
        if (!dir.mkpath(target)) {
            Logger::Fatal("Error creating directory: " + target);
            exit(EXIT_FAILURE);
        }
    }
    bool diskFull = false;
    for (const QString& file : fileList) {
        QStringList list = file.split(":");
        QString sourceFile = source + "/" + list.at(0);
        QString targetFile = target + "/" + list.at(0);
        if (diskFull) {
            addToErrorList(list.at(0), FileErrorType::DISKFULL);
            continue;
        }
        QString lyric;
        QString lyricTarget;
        for (const QString& format : MSTFileManager::supportedFormat) {
            if (sourceFile.contains(format)) {
                QString temp = sourceFile;
                QString tempTarget = targetFile;
                lyric = temp.replace(format, "lrc");
                lyricTarget = tempTarget.replace(format, "lrc");
                break;
            }
        }
        if (QFile::exists(targetFile)) {
            Logger::Warn("File existed, skipping " + targetFile);
            addToErrorList(list.at(0), FileErrorType::DUPLICATE);
            continue;
        }
        if (!entity.ignoreLyric && !static_cast<bool>(list.at(1).toInt())) {
            if (!QFile::exists(lyric)) {
                TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
                f = TagLib::FileRef(sourceFile.toStdWString().c_str());
#else
                f = TagLib::FileRef(sourceFile.toStdString().c_str());
#endif
                if (!f.isNull() && f.tag()) {
                    const TagLib::Tag* tag = f.tag();
                    if (!tag->properties().contains(key)) {
                        Logger::Warn("Lyric file not found, skipping " + lyric);
                        addToErrorList(list.at(0), FileErrorType::LNF);
                        continue;
                    }
                }
            } else {
                diskFull = MSTFileManager::isFull(sourceFile, target);
                if (diskFull) {
                    MSTFileManager::rollBackCopy(targetFile);
                    addToErrorList(list.at(0), FileErrorType::DISKFULL);
                    continue;
                }
                QFile::copy(lyric, lyricTarget);
            }
        }
        diskFull = MSTFileManager::isFull(sourceFile, target);
        if (diskFull) {
            MSTFileManager::rollBackCopy(targetFile);
            addToErrorList(list.at(0), FileErrorType::DISKFULL);
            continue;
        }
        QFile::copy(sourceFile, targetFile);
        emit current(fileList.indexOf(file));
    }
    emit finished();
    emit copyFinished(OperationType::COPY);
}

/**
 * @brief 显示操作结果对话框
 * @param type 操作类型（复制或加载）
 * @details 根据操作类型显示相应的结果对话框：
 *          - COPY: 显示复制操作的结果
 *          - LOAD: 显示加载操作的结果
 *          如果没有错误，直接刷新界面；如果有错误，显示错误详情。
 *          操作完成后清空错误列表并刷新音乐列表显示
 */
void MusicSyncTool::showOperationResult(const OperationType type) {
    const auto result = new OperationResult();
    switch (type) {
    case OperationType::COPY:
        result->setWindowTitle(tr("复制结果"));
        break;
    case OperationType::LOAD:
        result->setWindowTitle(tr("加载结果"));
        break;
    }
    QString errorString;
    if (errorList.isEmpty()) {
        delete result;
        if (type == OperationType::COPY) {
            if (favoriteOnly[0]) {
                getFavoriteMusic(PathType::LOCAL, currentPage[0]);
            } else {
                getMusic(PathType::LOCAL, currentPage[0]);
            }
            if (favoriteOnly[1]) {
                getFavoriteMusic(PathType::REMOTE, currentPage[1]);
            } else {
                getMusic(PathType::REMOTE, currentPage[1]);
            }
        }
        return;
    }
    for (QString& error : errorList) {
        errorString += error + "\n";
    }
    result->setText(errorString);
    result->exec();
    errorList.clear();
    if (type == OperationType::COPY) {
        if (favoriteOnly[0]) {
            getFavoriteMusic(PathType::LOCAL, currentPage[0]);
        } else {
            getMusic(PathType::LOCAL, currentPage[0]);
        }
        if (favoriteOnly[1]) {
            getFavoriteMusic(PathType::REMOTE, currentPage[1]);
        } else {
            getMusic(PathType::REMOTE, currentPage[1]);
        }
    }
}

/**
 * @brief 设置当前播放的音乐标题
 * @param file 文件名
 * @details 在用户界面上显示当前正在播放的音乐文件名，
 *          更新播放状态标签的文本内容
 */
void MusicSyncTool::setNowPlayingTitle(const QString& file) const { ui.nowPlaying->setText(tr("正在播放：") + file); }
/**
 * @brief 获取当前设置的语言
 * @return 语言字符串
 * @details 返回用户在设置中选择的界面语言，
 *          用于语言切换和界面本地化
 */
[[nodiscard]]
QString MusicSyncTool::getLanguage() const {
    return entity.language;
}

/**
 * @brief 远程打开操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 处理用户点击远程路径打开菜单的操作：
 *          1. 打开文件夹选择对话框
 *          2. 如果选择了路径，开始扫描音乐文件
 *          3. 设置可用空间显示
 *          4. 显示加载操作结果
 */
void MusicSyncTool::on_actionRemote_triggered(bool triggered) {
    openFolder(PathType::REMOTE);
    if (remote.getPath() == "") {
        return;
    }
    getMusic(PathType::REMOTE, 1);
    setAvailableSpace(PathType::REMOTE);
    showOperationResult(OperationType::LOAD);
}

/**
 * @brief 本地打开操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 处理用户点击本地路径打开菜单的操作：
 *          1. 打开文件夹选择对话框
 *          2. 如果选择了路径，开始扫描音乐文件
 *          3. 设置可用空间显示
 *          4. 显示加载操作结果
 */
void MusicSyncTool::on_actionLocal_triggered(bool triggered) {
    openFolder(PathType::LOCAL);
    if (local.getPath() == "") {
        return;
    }
    getMusic(PathType::LOCAL, 1);
    setAvailableSpace(PathType::LOCAL);
    showOperationResult(OperationType::LOAD);
}

/**
 * @brief 设置操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击设置菜单，打开设置对话框
 *          允许用户修改应用程序的各种配置选项
 */
void MusicSyncTool::on_actionSettings_triggered(bool triggered) const { showSettings(); }
/**
 * @brief 关于操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击关于菜单，显示应用程序的关于信息对话框，
 *          包括版本信息、开发者信息等内容
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void MusicSyncTool::on_actionAbout_triggered(bool triggered) {
    // NOLINT(*-convert-member-functions-to-static)
    AboutPage about;
    about.exec();
}

/**
 * @brief 复制到远程的槽函数
 * @details 处理用户点击复制到远程按钮的操作：
 *          1. 检查本地路径是否已选择
 *          2. 获取用户在本地表格中选中的音乐文件
 *          3. 检查是否有选中的文件
 *          4. 启动并发线程执行复制操作
 */
void MusicSyncTool::on_copyToRemote_clicked() {
    if (local.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    QStringList fileList = getSelectedMusic(PathType::LOCAL);
    if (fileList.isEmpty()) {
        popError(PET::NFS);
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, local.getPath(), fileList, remote.getPath());
}

/**
 * @brief 复制到本地的槽函数
 * @details 处理用户点击复制到本地按钮的操作：
 *          1. 检查远程路径是否已选择
 *          2. 获取用户在远程表格中选中的音乐文件
 *          3. 检查是否有选中的文件
 *          4. 启动并发线程执行复制操作
 */
void MusicSyncTool::on_copyToLocal_clicked() {
    if (remote.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    QStringList fileList = getSelectedMusic(PathType::REMOTE);
    if (fileList.isEmpty()) {
        popError(PET::NFS);
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, remote.getPath(), fileList, local.getPath());
}

/**
 * @brief 本地重复音乐扫描的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击本地重复音乐扫描菜单，
 *          在本地路径中查找重复的音乐文件并显示结果
 */
void MusicSyncTool::on_actionDupeLocal_triggered(bool triggered) { getDuplicatedMusic(PathType::LOCAL); }
/**
 * @brief 远程重复音乐扫描的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击远程重复音乐扫描菜单，
 *          在远程路径中查找重复的音乐文件并显示结果
 */
void MusicSyncTool::on_actionDupeRemote_triggered(bool triggered) { getDuplicatedMusic(PathType::REMOTE); }
/**
 * @brief 刷新本地音乐列表的槽函数
 * @details 响应用户点击本地刷新按钮，重新扫描本地路径
 *          并重新加载音乐列表到第一页
 */
void MusicSyncTool::on_refreshLocal_clicked() { getMusic(PathType::LOCAL, 1); }
/**
 * @brief 刷新远程音乐列表的槽函数
 * @details 响应用户点击远程刷新按钮，重新扫描远程路径
 *          并重新加载音乐列表到第一页
 */
void MusicSyncTool::on_refreshRemote_clicked() { getMusic(PathType::REMOTE, 1); }
/**
 * @brief 本地搜索的槽函数
 * @details 响应用户在本地搜索框中按下回车键，
 *          根据输入的文本在本地音乐列表中进行搜索
 */
void MusicSyncTool::on_searchLocal_returnPressed() { searchMusic(PathType::LOCAL, ui.searchLocal->text()); }
/**
 * @brief 远程搜索的槽函数
 * @details 响应用户在远程搜索框中按下回车键，
 *          根据输入的文本在远程音乐列表中进行搜索
 */
void MusicSyncTool::on_searchRemote_returnPressed() { searchMusic(PathType::REMOTE, ui.searchRemote->text()); }
/**
 * @brief 本地表格双击预览的槽函数
 * @param row 双击的行号
 * @param column 双击的列号（未使用）
 * @details 响应用户双击本地音乐表格中的某一行，
 *          设置该音乐文件的总时长并开始播放预览
 */
void MusicSyncTool::on_tableWidgetLocal_cellDoubleClicked(const int row, int column) {
    setTotalLength(PathType::LOCAL, row);
}

/**
 * @brief 远程表格双击预览的槽函数
 * @param row 双击的行号
 * @param column 双击的列号（未使用）
 * @details 响应用户双击远程音乐表格中的某一行，
 *          设置该音乐文件的总时长并开始播放预览
 */
void MusicSyncTool::on_tableWidgetRemote_cellDoubleClicked(const int row, int column) {
    setTotalLength(PathType::REMOTE, row);
}

/**
 * @brief 退出程序的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击退出菜单，正常退出应用程序
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void MusicSyncTool::on_actionExit_triggered(bool triggered) {
    exit(EXIT_SUCCESS);
} // NOLINT(*-convert-member-functions-to-static)

/**
 * @brief 弹出窗口询问用户是否要删除所有日志文件
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击清除日志文件菜单，显示确认对话框，
 *          如果用户确认则清除所有扫描日志文件，并显示完成提示
 */
void MusicSyncTool::on_actionClean_log_files_triggered(bool triggered) {
    const QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("提示"), tr("确定要清除所有日志文件吗？"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }
    cleanLog();
    QMessageBox::information(this, tr("提示"), tr("日志文件已清除"));
}

/**
 * @brief 播放或暂停音乐
 * @details 响应用户点击播放控制按钮：
 *          - 如果没有选定音频文件，显示错误提示
 *          - 如果正在播放，则暂停播放并更新界面状态
 *          - 如果已暂停或停止，则开始播放并更新界面状态
 */
void MusicSyncTool::on_playControl_clicked() {
    if (player->getNowPlaying().isEmpty()) {
        popError(PET::NOAUDIO);
        return;
    }
    if (player->isPlaying()) {
        player->pause();
        setMediaWidget(PlayState::PAUSED);
    } else {
        player->play();
        setMediaWidget(PlayState::PLAYING);
    }
}

/**
 * @brief 设置滑块位置
 * @param position 播放位置（毫秒）
 * @details 根据当前播放位置更新播放进度滑块和时间显示，
 *          将播放时间格式化为分:秒的形式，同时显示当前时间和总时长
 */
void MusicSyncTool::setSliderPosition(const qint64 position) const {
    ui.playSlider->setValue(static_cast<int>(position));
    QString progress;
    if (position % 60000 / 1000 < 10) {
        progress = QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000);
    } else {
        progress = QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000);
    }
    progress += "/" + QString::number(player->getDuration() / 60000) + ":";
    if (player->getDuration() % 60000 / 1000 < 10) {
        progress += "0" + QString::number(player->getDuration() % 60000 / 1000);
    } else {
        progress += QString::number(player->getDuration() % 60000 / 1000);
    }
    ui.playProgress->setText(progress);
}

/**
 * @brief 播放滑块移动的槽函数
 * @param position 滑块位置
 * @details 响应用户拖动播放进度滑块，设置播放器的播放位置
 *          并更新时间显示
 */
void MusicSyncTool::on_playSlider_sliderMoved(const int position) const {
    player->setPosition(position);
    if (position % 60000 / 1000 < 10) {
        ui.playProgress->setText(QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000));
    } else {
        ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
    }
}

/**
 * @brief 播放滑块按下的槽函数
 * @details 响应用户按下播放进度滑块，立即设置播放器位置到滑块当前值
 */
void MusicSyncTool::on_playSlider_sliderPressed() const { player->setPosition(ui.playSlider->value()); }
/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_sliderPressed() const {
    player->setVolume(static_cast<float>(ui.volumeSlider->value() / 100.0));
    const QString text = tr("音量：") + QString::number(ui.volumeSlider->value()) + "%";
    ui.volumeLabel->setText(text);
}

/**
 * @brief 本地收藏按钮的槽函数
 * @details 响应用户点击本地收藏按钮，显示第一页的收藏音乐列表
 */
void MusicSyncTool::on_favoriteOnlyLocal_clicked() { getFavoriteMusic(PathType::LOCAL, 1); }
/**
 * @brief 远程收藏按钮的槽函数
 * @details 响应用户点击远程收藏按钮，显示第一页的收藏音乐列表
 */
void MusicSyncTool::on_favoriteOnlyRemote_clicked() { getFavoriteMusic(PathType::REMOTE, 1); }
/*
 * @brief Slots for last page switch(local)
 */
void MusicSyncTool::on_lastPageLocal_clicked() {
    if (local.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (currentPage[0] == 1) {
        popError(PET::FIRST);
        return;
    }
    if (favoriteOnly[0]) {
        getFavoriteMusic(PathType::LOCAL, --currentPage[0]);
    } else {
        getMusic(PathType::LOCAL, --currentPage[0]);
    }
}

/*
 * @brief Slots for next page switch(local)
 */
void MusicSyncTool::on_nextPageLocal_clicked() {
    if (local.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (currentPage[0] == totalPage[0]) {
        popError(PET::LAST);
        return;
    }
    if (favoriteOnly[0]) {
        getFavoriteMusic(PathType::LOCAL, ++currentPage[0]);
    } else {
        getMusic(PathType::LOCAL, ++currentPage[0]);
    }
}

/*
 * @brief Slots for last page switch(remote)
 */
void MusicSyncTool::on_lastPageRemote_clicked() {
    if (remote.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (currentPage[1] == 1) {
        popError(PET::FIRST);
        return;
    }
    if (favoriteOnly[1]) {
        getFavoriteMusic(PathType::REMOTE, --currentPage[1]);
    } else {
        getMusic(PathType::REMOTE, --currentPage[1]);
    }
}

/*
 * @brief Slots for next page switch(remote)
 */
void MusicSyncTool::on_nextPageRemote_clicked() {
    if (remote.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (currentPage[1] == totalPage[1]) {
        popError(PET::LAST);
        return;
    }
    if (favoriteOnly[1]) {
        getFavoriteMusic(PathType::REMOTE, ++currentPage[1]);
    } else {
        getMusic(PathType::REMOTE, ++currentPage[1]);
    }
}

/*
 * @brief End media playback
 */
void MusicSyncTool::endMedia(const QMediaPlayer::PlaybackState state) const {
    if (state == QMediaPlayer::PlaybackState::StoppedState) {
        ui.playControl->setText(tr("播放"));
        ui.nowPlaying->setText(tr("播放已结束。"));
        ui.playSlider->setValue(0);
        ui.playProgress->setText("00:00");
    }
}

/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_sliderMoved(const int position) const {
    player->setVolume(static_cast<float>(position / 100.0));
    const QString text = tr("音量：") + QString::number(position) + "%";
    ui.volumeLabel->setText(text);
}

/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_valueChanged(const int position) const { on_volumeSlider_sliderMoved(position); }

void MusicSyncTool::on_copyFinished(OperationType op) const {
    if (local.getPath() != "") {
        setAvailableSpace(PathType::LOCAL);
    }
    if (remote.getPath() != "") {
        setAvailableSpace(PathType::REMOTE);
    }
}

/**
 * @brief 设置播放滑块的总长度
 * @param path 路径类型（本地或远程）
 * @param row 表格行号
 * @details 根据用户选择的音乐文件设置播放器的总时长，
 *          获取音频文件的时长信息并配置播放滑块的最大值，
 *          开始播放选定的音乐文件
 */
void MusicSyncTool::setTotalLength(const PathType path, const int row) {
    const QTableWidget& widget = path == PathType::LOCAL ? *ui.tableWidgetLocal : *ui.tableWidgetRemote;
    auto& ds = path == PathType::LOCAL ? local : remote;
    // QString sql = "SELECT fileName FROM musicInfo WHERE";
    // sql += " title = \"" + widget.item(row, 0)->text() + "\" AND artist = \"" + widget.item(row, 1)->text() +
    // 	"\" AND album = \"" + widget.item(row, 2)->text() + "\"";
    const auto item = std::make_shared<QueryItem>();
    item->setTitle(widget.item(row, 0)->text());
    item->setArtist(widget.item(row, 1)->text());
    item->setAlbum(widget.item(row, 2)->text());
    QList<QueryItem> file;
    file.append(*item);
    nowPlaying = ds.getFileNameByMD(file).at(0);
    const QString filePath = (path == PathType::LOCAL ? local.getPath() : remote.getPath()) + "/" + nowPlaying;
    player->setNowPlaying(filePath);
    TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
    f = TagLib::FileRef(filePath.toStdWString().c_str());
#else
    f = TagLib::FileRef(filePath.toStdString().c_str());
#endif
    if (!f.isNull() && f.audioProperties()) {
        const qint64 length = f.audioProperties()->lengthInMilliseconds();
        setNowPlayingTitle(nowPlaying);
        player->setPosition(0);
        player->play();
        setMediaWidget(PlayState::PLAYING);
        ui.playSlider->setMaximum(static_cast<int>(length));
        ui.playSlider->setValue(0);
        ui.playProgress->setText("00:00");
    }
}

/**
 * @brief 获取收藏的音乐文件
 * @param path 路径类型（本地或远程）
 * @param page 页码
 * @details 从指定的数据源获取标记为收藏的音乐文件列表，
 *          支持分页显示。如果没有设置收藏标签，显示错误提示。
 *          更新表格显示和分页信息
 */
void MusicSyncTool::getFavoriteMusic(const PathType path, const unsigned short page) {
    if (path == PathType::LOCAL && local.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (path == PathType::REMOTE && remote.getPath() == "") {
        popError(PET::NPS);
        return;
    }
    if (entity.favoriteTag == "") {
        popError(PET::NFT);
        return;
    }
    MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
    const auto fileList = ds.getFavorite(page, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
    const qsizetype totalSize = fileList.size();
    totalPage[(path == PathType::LOCAL ? 0 : 1)] = static_cast<short>(totalSize / PAGESIZE) + 1;
    const qsizetype lastPageSize = totalSize % PAGESIZE;
    // NOLINT(cppcoreguidelines-narrowing-conversions)
    QTableWidget* targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
    (path == PathType::LOCAL ? ui.pageLocal : ui.pageRemote)
        ->setText(QString::number(currentPage[(path == PathType::LOCAL ? 0 : 1)]) + "/" +
                  QString::number(totalPage[(path == PathType::LOCAL ? 0 : 1)]));
    targetTable->clearContents();
    qsizetype rowSize;
    if (currentPage[(path == PathType::LOCAL ? 0 : 1)] == totalPage[(path == PathType::LOCAL ? 0 : 1)]) {
        rowSize = lastPageSize;
    } else {
        rowSize = PAGESIZE;
    }
    targetTable->setRowCount(static_cast<int>(rowSize));
    favoriteOnly[(path == PathType::LOCAL ? 0 : 1)] = true;
    for (int i = (page - 1) * PAGESIZE; i < rowSize; i++) {
        targetTable->setItem(i, 0, new QTableWidgetItem(fileList.at(i).getTitle()));
        targetTable->setItem(i, 1, new QTableWidgetItem(fileList.at(i).getArtist()));
        targetTable->setItem(i, 2, new QTableWidgetItem(fileList.at(i).getAlbum()));
        targetTable->setItem(i, 3, new QTableWidgetItem(fileList.at(i).getGenre()));
        targetTable->setItem(i, 4, new QTableWidgetItem(fileList.at(i).getYear()));
        targetTable->setItem(i, 5, new QTableWidgetItem(fileList.at(i).getTrack()));
    }
}

/**
 * @brief 连接应用程序的信号槽
 * @details 建立各个组件之间的信号槽连接，包括：
 *          - 数据源与加载页面的进度连接
 *          - 复制操作和加载操作的完成信号
 *          - 媒体播放器的位置和状态变化信号
 *          - 错误处理的信号连接
 *          确保各组件能够正确响应事件和更新状态
 */
void MusicSyncTool::connectSlots() const {
    connect(&local, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
    connect(&remote, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
    connect(&local, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
    connect(&remote, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
    connect(&local, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
    connect(&remote, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
    connect(&local, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
    connect(&remote, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
    connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::showOperationResult);
    connect(&local, QOverload<OperationType>::of(&MSTDataSource::loadFinished), this, &MusicSyncTool::showOperationResult);
    connect(&remote, QOverload<OperationType>::of(&MSTDataSource::loadFinished), this, &MusicSyncTool::showOperationResult);
    connect(player->getMediaPlayer(), &QMediaPlayer::positionChanged, this, &MusicSyncTool::setSliderPosition);
    connect(player->getMediaPlayer(), &QMediaPlayer::playbackStateChanged, this, &MusicSyncTool::endMedia);
    connect(this, &MusicSyncTool::addToErrorListConcurrent, this,
            QOverload<const QString&, LoadErrorType>::of(&MusicSyncTool::addToErrorList));
    connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::on_copyFinished);
}

/**
 * @brief 获取可用空间并显示在顶部
 * @param path 路径类型（本地或远程）
 * @details 通过文件管理器获取指定路径的存储空间信息，
 *          并在用户界面上显示可用空间大小，帮助用户了解
 *          磁盘使用情况
 */
void MusicSyncTool::setAvailableSpace(const PathType path) const {
	const shared_ptr<MSTFileManager> manager = path == PathType::LOCAL ? localManager : remoteManager;
    const QStorageInfo storage(path == PathType::LOCAL ? local.getPath() : remote.getPath());
    const QString textBuilder = tr("可用空间：") + manager->getSpaceInfo();
    (path == PathType::LOCAL ? ui.availableSpaceLocal : ui.availableSpaceRemote)->setText(textBuilder);
}

/**
 * @brief 清理日志文件
 * @details 删除log目录下的所有扫描日志文件，清理历史记录。
 *          如果日志目录不存在则直接返回。这个操作通常在用户
 *          手动清理或需要重新完整扫描时执行
 */
void MusicSyncTool::cleanLog() {
    const QDir logDir("log");
    if (!logDir.exists()) {
        return;
    }
    QStringList logFiles = logDir.entryList(QDir::Files);
    for (QString& logFile : logFiles) {
        QFile file("log/" + logFile);
        file.remove();
    }
}




