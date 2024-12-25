#pragma warning(disable : 6031)
#include "MusicSyncTool.h"
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

/**
 * @brief 构造函数
 */
MusicSyncTool::MusicSyncTool(QWidget *parent) :
    QMainWindow(parent), translator(new QTranslator(this)), mediaPlayer(new QMediaPlayer(this)),
    audioOutput(new QAudioOutput(this)) {
    initDatabase();
    loadSettings();
    initUI();
    loadLanguage();
    initMediaPlayer();
    connectSlots();
    setMediaWidget(playState::STOPPED);
}
/**
 * @brief 析构函数
 */
MusicSyncTool::~MusicSyncTool() {
    if (dbLocal.isOpen()) {
        dbLocal.close();
    }
    if (dbRemote.isOpen()) {
        dbRemote.close();
    }
    delete mediaPlayer;
    delete audioOutput;
    delete loading;
}
/**
 * @brief 初始化数据库
 */
void MusicSyncTool::initDatabase() {
    dbLocal = QSqlDatabase::addDatabase("QSQLITE", "musicInfoLocal");
    dbRemote = QSqlDatabase::addDatabase("QSQLITE", "musicInfoRemote");
    queryLocal = QSqlQuery(dbLocal);
    queryRemote = QSqlQuery(dbRemote);
}
/**
 * @brief 加载设置
 */
void MusicSyncTool::loadSettings() {
    QFile file("settings.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[WARN] No settings file found, creating new setting file named settings.json";
        file.close();
        loadDefaultSettings();
        return;
    }
    QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
    if (settings.isNull()) {
        qDebug() << "[WARN] Invalid settings file, creating new setting file named settings.json";
        file.close();
        loadDefaultSettings();
        return;
    }
    QJsonObject obj = settings.object();
    entity.ignoreLyric = obj["ignoreLyric"].toBool();
    entity.sortBy = obj["sortBy"].toInt();
    entity.orderBy = obj["orderBy"].toInt();
    entity.language = obj["language"].toString();
    entity.favoriteTag = obj["favoriteTag"].toString();
    file.close();
}
void MusicSyncTool::loadDefaultSettings() {
    QFile file("settings.json");
    file.open(QIODevice::WriteOnly);
    QJsonObject obj;
    obj["ignoreLyric"] = false;
    obj["sortBy"] = sortByEnum::TITLE;
    obj["orderBy"] = orderByEnum::ASC;
    obj["language"] = "";
    obj["favoriteTag"] = "";
    file.write(QJsonDocument(obj).toJson());
    file.close();
}
/**
 * @brief 初始化媒体播放器
 */
void MusicSyncTool::initMediaPlayer() {
    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5);
}
/**
 * @brief 加载语言
 */
void MusicSyncTool::loadLanguage() {
    QFile file("translations/langinfo.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "[FATAL] Error opening langinfo.json:" << file.errorString();
        exit(EXIT_FAILURE);
    }
    QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonArray langArray = langinfo.array();
    for (QJsonValue lang : langArray) {
        if (entity.language == "") {
            entity.language = "中文";
        }
        QJsonObject langObj = lang.toObject();
        if (langObj["lang"].toString() == entity.language) {
            translator->load("translations/" + (langObj["fileName"]).toString());
            break;
        }
    }
    qApp->installTranslator(translator);
    ui.retranslateUi(this);
}
/**
 * @brief 初始化UI组件
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
 * @brief 弹出错误
 * @param type 错误类型
 */
void MusicSyncTool::popError(PET type) {
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
    }
}
/**
 * @brief 设置媒体控件
 * @param state 播放状态
 */
void MusicSyncTool::setMediaWidget(playState state) {
    if (state == playState::PLAYING) {
        ui.playControl->setText(tr("暂停"));
        setNowPlayingTitle(nowPlaying);
    } else if (state == playState::PAUSED) {
        ui.playControl->setText(tr("播放"));
    } else if (state == playState::STOPPED) {
        ui.playControl->setText(tr("播放"));
        ui.nowPlaying->setText(tr("播放已结束。"));
    }
    ui.volumeLabel->setText(tr("音量：") + QString::number(ui.volumeSlider->value()) + "%");
}
/**
 * @brief 打开文件夹
 * @param path 路径类型（本地或远程）
 */
void MusicSyncTool::openFolder(pathType path) {
    QFileDialog fileDialog;
    fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
    fileDialog.setFileMode(QFileDialog::Directory);
    QString dir = fileDialog.getExistingDirectory();
    if (dir == "") {
        return;
    }
    (path == pathType::LOCAL ? localPath : remotePath) = dir;
    if (path == pathType::LOCAL) {
        dbLocal.setDatabaseName(localPath + "/musicInfo.db");
    } else {
        dbRemote.setDatabaseName(remotePath + "/musicInfo.db");
    }
    QSqlDatabase &db = path == pathType::LOCAL ? dbLocal : dbRemote;
    if (!db.open()) {
        qDebug() << "[FATAL] Error opening database:" << db.lastError().text().toStdString();
        exit(EXIT_FAILURE);
    }
}
/**
 * @brief 获取音乐信息
 * @param path 路径类型（本地或远程）
 */
void MusicSyncTool::getMusic(pathType path, short page) {
    qDebug() << "[INFO] Scanning started";
    if (localPath == "" && remotePath == "") {
        qDebug() << "[WARN] No path selected";
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::getMusicConcurrent, this, path, page);
}
/**
 * @brief 获取音乐信息（并发）
 * @param path 路径类型（本地或远程）
 */
void MusicSyncTool::getMusicConcurrent(pathType path, short page) {
    clock_t start = clock();
    emit started();
    favoriteOnly[path == pathType::LOCAL ? 0 : 1] = false;
    QStringList temp = (path == pathType::LOCAL ? localPath : remotePath).split("/");
    QString logFile = "lastScan";
    temp[0].remove(":");
    for (QString tempStr : temp) {
        logFile += " - " + tempStr;
    }
    logFile += ".log";
    QFile file("log/" + logFile);
    QDateTime dateTime;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        dateTime = QDateTime::fromString(in.readLine());
    } else {
        qDebug() << "[WARN] No last scan log found, scanning all files";
        dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
    }
    QDir dir(path == pathType::LOCAL ? localPath : remotePath);
    QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
                  "track INT, favorite BOOL, fileName TEXT)";
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    query.exec(sql);
    QStringList newFileList = dir.entryList(QDir::Files);
    sql = "SELECT fileName FROM musicInfo";
    query.exec(sql);
    QStringList oldFileList;
    while (query.next()) {
        oldFileList.append(query.value(0).toString());
    }
    newFileList.sort();
    oldFileList.sort();
    for (int i = 0; i < oldFileList.size(); i++) {
        if (!newFileList.contains(oldFileList.at(i))) {
            sql = "DELETE FROM musicInfo WHERE fileName = \"" + oldFileList.at(i) + "\"";
            query.exec(sql);
            oldFileList.removeAt(i);
            i = -1;
        }
    }
    for (int i = 0; i < newFileList.size(); i++) {
        if (oldFileList.contains(newFileList.at(i)) || !supportedFormat.contains(newFileList.at(i).right(4))) {
            newFileList.removeAt(i);
            i = -1;
        }
    }
    newFileList.removeOne("musicInfo.db");
    emit total(newFileList.size() + oldFileList.size());
    emit current(0);
    TagLib::String key(entity.favoriteTag.toStdString());
    for (int i = 0; i < newFileList.size(); i++) {
        QString file = (path == pathType::LOCAL ? localPath : remotePath) + "/" + newFileList.at(i).toUtf8();
        TagLib::FileRef f(file.toStdWString().c_str());
        if (!f.isNull() && f.tag()) {
            TagLib::Tag *tag = f.tag();
            sql = "INSERT INTO musicInfo (title, artist, album, genre, year, track, favorite, fileName) VALUES (\"" +
                QString::fromStdString(tag->title().to8Bit(true)) + "\", \"" +
                QString::fromStdString(tag->artist().to8Bit(true)) + "\", \"" +
                QString::fromStdString(tag->album().to8Bit(true)) + "\", \"" +
                QString::fromStdString(tag->genre().to8Bit(true)) + "\", " + QString::number(tag->year()) + ", " +
                (tag->properties().contains(key) ? "1" : "0") + ", " + QString::number(tag->track()) + ", \"" +
                newFileList.at(i) + "\")";
            query.exec(sql);
            emit current(i);
        } else {
            emit addToErrorListConcurrent(newFileList.at(i), loadErrorType::TAGERR);
            continue;
        }
    }
    if (entity.favoriteTag != "") {
        for (int i = 0; i < oldFileList.size(); i++) {
            QString file = (path == pathType::LOCAL ? localPath : remotePath) + "/" + oldFileList.at(i).toUtf8();
            if (QFile(file).fileTime(QFileDevice::FileModificationTime) <= dateTime) {
                continue;
            }
            TagLib::FileRef f(file.toStdWString().c_str());
            if (!f.isNull() && f.tag()) {
                TagLib::Tag *tag = f.tag();
                sql = QString("UPDATE musicInfo SET favorite = ") + (tag->properties().contains(key) ? "1" : "0") +
                    " WHERE fileName = \"" + oldFileList.at(i) + "\"";
                query.exec(sql);
                emit current(i + newFileList.size());
            } else {
                continue;
            }
        }
    }
    sql = "SELECT COUNT(*) FROM musicInfo";
    query.exec(sql);
    query.next();
    totalPage[path == pathType::LOCAL ? 0 : 1] = query.value(0).toInt() / PAGESIZE + 1;
    short lastPageSize = query.value(0).toInt() % PAGESIZE;
    emit total(PAGESIZE);
    sql = "SELECT title, artist, album, genre, year, track FROM musicInfo ORDER BY";
    switch (entity.sortBy) {
    case sortByEnum::TITLE:
        sql += " title";
        break;
    case sortByEnum::ARTIST:
        sql += " artist";
        break;
    case sortByEnum::ALBUM:
        sql += " album";
        break;
    default:
        sql += " title";
        break;
    }
    switch (entity.orderBy) {
    case orderByEnum::ASC:
        sql += " ASC";
        break;
    case orderByEnum::DESC:
        sql += " DESC";
        break;
    default:
        sql += " ASC";
        break;
    }
    sql += " LIMIT " + QString::number(PAGESIZE) + " OFFSET " +
        QString::number((currentPage[(path == pathType::LOCAL ? 0 : 1)] - 1) * PAGESIZE);
    query.exec(sql);
    QTableWidget *targetTable = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
    targetTable->clearContents();
    if (currentPage[(path == pathType::LOCAL ? 0 : 1)] == totalPage[(path == pathType::LOCAL ? 0 : 1)]) {
        targetTable->setRowCount(lastPageSize);
    } else {
        targetTable->setRowCount(PAGESIZE);
    }
    for (int j = 0; query.next(); j++) {
        emit current(j);
        for (int i = 0; i < 6; i++) {
            if (query.value(i) != 0) {
                targetTable->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
            }
        }
    }
    clock_t end = clock();
    qDebug() << "[INFO] Scanning finished in" << (double)(end - start) / CLOCKS_PER_SEC << "seconds";
    QDir logDir("log");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    QFile timeFile("log/" + logFile);
    if (!timeFile.open(QIODevice::WriteOnly)) {
        qDebug() << "[WARN] Cannot write to lastScan.log";
        return;
    }
    (path == pathType::LOCAL ? ui.pageLocal : ui.pageRemote)
        ->setText(QString::number(currentPage[(path == pathType::LOCAL ? 0 : 1)]) + "/" +
                  QString::number(totalPage[(path == pathType::LOCAL ? 0 : 1)]));
    QTextStream out(&timeFile);
    out << QDateTime::currentDateTime().toString() << "\n";
    timeFile.close();
    emit finished();
}
/**
 * @brief 搜索音乐
 * @param path 路径类型（本地或远程）
 * @param text 搜索文本
 */
void MusicSyncTool::searchMusic(pathType path, QString text) {
    if (text == "") {
        getMusic(path, 1);
        return;
    }
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->clearContents();
    QString sql = "SELECT COUNT(*) FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text +
        "%\" OR album LIKE \"%" + text + "%\"";
    query.exec(sql);
    query.next();
    int tableSize = query.value(0).toInt();
    (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setRowCount(tableSize);
    sql = "SELECT * FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text +
        "%\" OR album LIKE \"%" + text + "%\"";
    query.exec(sql);
    while (query.next()) {
        for (int i = 0; i < 6; i++) {
            if (query.value(i) != 0) {
                (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)
                    ->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
            }
        }
    }
}
/**
 * @brief 添加到错误列表
 * @param file 文件名
 * @param error 错误类型
 */
void MusicSyncTool::addToErrorList(QString file, fileErrorType error) {
    switch (error) {
    case fileErrorType::DUPLICATE:
        errorList.append(tr("复制") + file + tr("失败：文件已存在"));
        break;
    case fileErrorType::LNF:
        errorList.append(tr("复制") + file + tr("失败：找不到歌词文件"));
        break;
    case fileErrorType::DISKFULL:
        errorList.append(tr("复制") + file + tr("失败：磁盘已满"));
        break;
    default:
        break;
    }
}

void MusicSyncTool::addToErrorList(QString file, loadErrorType error) { 
    switch (error) {
    case loadErrorType::FNS:
        errorList.append(tr("加载") + file + tr("失败：文件不可扫描"));
        break;
    case loadErrorType::TAGERR:
        errorList.append(tr("加载") + file + tr("失败：标签错误"));
        break;
    default:
        break;
    }
}
 /**
 * @brief 获取重复音乐
 * @param path 路径类型（本地或远程）
 * @return 重复音乐列表
 */

QStringList MusicSyncTool::getDuplicatedMusic(pathType path) {
    QString selectedPath = (path == pathType::LOCAL ? localPath : remotePath);
    if (selectedPath == "") {
        qDebug() << "[WARN] No path selected";
        popError(PET::NPS);
        return QStringList();
    }
    ShowDupe dp;
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    QString sql = "SELECT title, artist, album, year, fileName FROM musicInfo ORDER BY title";
    query.exec(sql);
    QString fast;
    QString fastFileName;
    QString slow;
    QString slowFileName;
    QStringList dupeList;
    query.next();
    slow = query.value(0).toString() + 
        ":" + query.value(1).toString() +
        ":" + query.value(2).toString() + 
        ":" + query.value(3).toString();
    slowFileName = query.value(4).toString();
    while (query.next()) {
        fast = query.value(0).toString() +
            ":" + query.value(1).toString() +
            ":" + query.value(2).toString() + 
            ":" + query.value(3).toString();
        fastFileName = query.value(4).toString();
        auto slowList = slow.split(":");
        auto fastList = fast.split(":");
        for (int i = 0, dupeCount = 0; i < 4; i++) {
            if (slowList.at(i) == fastList.at(i)) {
                dupeCount++;
            }
            if (dupeCount >= 3 && slowList.at(0) == fastList.at(0)) {
                dupeList.append(slowFileName);
                dupeList.append(fastFileName);
            }
        }
        slow = fast;
        slowFileName = fastFileName;
    }
    for (int i = 0; i < dupeList.size(); i++) {
        qDebug() << "[INFO] Found duplicated music named" << dupeList.at(i) << "at"
                 << (path == pathType::LOCAL ? localPath : remotePath);
    }
    for (int i = 0; i < dupeList.size(); i++) {
        dp.add(dupeList.at(i));
    }
    dp.exec();
    return dupeList;
}
/**
 * @brief 获取表格中选中的音乐
 * @param path 路径类型（本地或远程）
 * @return 选中音乐列表
 */

QStringList MusicSyncTool::getSelectedMusic(pathType path) {
    QSet<int> selectedRows;
    const QTableWidget *const table = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
    if (table->rowCount() == 0) {
        return QStringList();
    }
    for (int i = 0; i < table->rowCount(); i++) {
        if (table->item(i, 0)->isSelected()) {
            selectedRows.insert(i);
        }
    }
    QStringList titleList;
    QStringList artistList;
    for (int i : selectedRows) {
        titleList.append(table->item(i, 0)->text());
        artistList.append(table->item(i, 1)->text());
    }
    if (selectedRows.size() == 0) {
        return QStringList();
    }
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    QString titleListString;
    QString artistListString;
    for (int i = 0; i < selectedRows.size() - 1; i++) {
        titleListString += "\"" + titleList.at(i) + "\", ";
        artistListString += "\"" + artistList.at(i) + "\", ";
    }
    titleListString += "\"" + titleList.at(selectedRows.size() - 1) + "\"";
    artistListString += "\"" + artistList.at(selectedRows.size() - 1) + "\"";
    QString sql = "SELECT fileName FROM musicInfo WHERE title IN(" + titleListString + ") AND artist IN(" +
        artistListString + ")";
    QStringList fileList = QStringList();
    query.exec(sql);
    while (query.next()) {
        fileList.append(query.value(0).toString());
    }
    return fileList;
}
/**
 * @brief 显示设置页面
 */
void MusicSyncTool::showSettings() {
    Settings *page = new Settings();
    connect(page, SIGNAL(confirmPressed(set)), this, SLOT(saveSettings(set)));
    page->show();
}
/**
 * @brief 保存设置
 * @param entity 配置实体
 */
void MusicSyncTool::saveSettings(set entity) {
    QFile file("settings.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "[FATAL] Error opening settings file";
        QMessageBox::critical(this, tr("错误"), tr("无法打开设置文件"));
        return;
    }
    QJsonObject obj;
    obj["ignoreLyric"] = entity.ignoreLyric;
    obj["sortBy"] = entity.sortBy;
    obj["orderBy"] = entity.orderBy;
    obj["language"] = entity.language;
    obj["favoriteTag"] = entity.favoriteTag;
    int tempSort = this->entity.sortBy;
    int tempOrder = this->entity.orderBy;
    this->entity = entity;
    if (tempSort != entity.sortBy || tempOrder != entity.orderBy) {
        if (localPath != "") {
            getMusic(pathType::LOCAL, 1);
        }
        if (remotePath != "") {
            getMusic(pathType::REMOTE, 1);
        }
    }
    loadLanguage();
    qDebug() << "[INFO] IgnoreLyric:" << this->entity.ignoreLyric;
    qDebug() << "[INFO] SortBy:" << this->entity.sortBy;
    qDebug() << "[INFO] OrderBy:" << this->entity.orderBy;
    qDebug() << "[INFO] Language:" << this->entity.language;
    qDebug() << "[INFO] FavoriteTag:" << this->entity.favoriteTag;
    QJsonDocument settings;
    settings.setObject(obj);
    file.write(settings.toJson());
    file.close();
}
/**
 * @brief 复制音乐
 * @param source 源路径
 * @param fileList 音乐列表
 * @param target 目标路径
 */
void MusicSyncTool::copyMusic(const QString source, QStringList fileList, const QString target) {
    TagLib::String key = "LYRICS";
    emit started();
    emit total(fileList.size());
    QDir dir(target);
    if (dir.isEmpty()) {
        dir.mkpath(target);
    }
    QStringList errorList;
    for (QString &file : fileList) {
        QString sourceFile = source + "/" + file;
        QString targetFile = target + "/" + file;
        QString lyric;
        QString lyricTarget;
        for (const QString &format : supportedFormat) {
            if (sourceFile.contains(format)) {
                QString temp = sourceFile;
                QString tempTarget = targetFile;
                lyric = temp.replace(format, ".lrc");
                lyricTarget = tempTarget.replace(format, ".lrc");
                break;
            }
        }
        if (QFile::exists(targetFile)) {
            qDebug() << "[WARN] File existed, skipping" << targetFile;
            addToErrorList(file, fileErrorType::DUPLICATE);
            continue;
        } else if (!entity.ignoreLyric) {
            if (!QFile::exists(lyric)) {
                TagLib::FileRef f(sourceFile.toStdWString().c_str());
                if (!f.isNull() && f.tag()) {
                    TagLib::Tag *tag = f.tag();
                    if (!tag->properties().contains(key)) {
                        qDebug() << "[WARN] Lyric file not found, skipping" << lyric;
                        addToErrorList(file, fileErrorType::LNF);
                        continue;
                    }
                }
            } else {
                if (isFull(lyric, target)) {
                    continue;
                }
                QFile::copy(lyric, lyricTarget);
            }
        }
        if (isFull(sourceFile, target)) {
            QFile::remove(lyricTarget);
            addToErrorList(file, fileErrorType::DISKFULL);
            continue;
        }
        QFile::copy(sourceFile, targetFile);
        emit current(fileList.indexOf(file));
    }
    emit finished();
    emit copyFinished(operationType::COPY);
}
/**
 * @brief 显示复制结果
 */
void MusicSyncTool::showOperationResult(operationType type) {
    OperationResult *result = new OperationResult();
    switch (type) {
    case operationType::COPY:
        result->setWindowTitle(tr("复制结果"));
        break;
    case operationType::LOAD:
        result->setWindowTitle(tr("加载结果"));
        break;
    default:
        break;
    }
    QString errorString;
    if (errorList.isEmpty()) {
        delete result;
        if (type == operationType::COPY) {
            getMusic(pathType::LOCAL, 1);
            getMusic(pathType::REMOTE, 1);
        }
        return;
    }
    for (QString &error : errorList) {
        errorString += error + "\n";
    }
    result->setText(errorString);
    result->exec();
    errorList.clear();
    if (type == operationType::COPY) {
        getMusic(pathType::LOCAL, 1);
        getMusic(pathType::REMOTE, 1);
    }
}
/**
 * @brief 设置当前播放音乐
 * @param file 文件名
 */
void MusicSyncTool::setNowPlayingTitle(QString file) { ui.nowPlaying->setText(tr("正在播放：") + file); }
/**
 * @brief 获取语言
 * @return 语言
 */
QString MusicSyncTool::getLanguage() const { return entity.language; }
/**
 * @brief 本地音乐按钮触发
 * @param triggered 是否触发
 */
void MusicSyncTool::on_actionRemote_triggered(bool triggered) {
    openFolder(pathType::REMOTE);
    if (remotePath == "") {
        return;
    }
    getMusic(pathType::REMOTE, 1);
    showOperationResult(operationType::LOAD);
}
/**
 * @brief 远程音乐按钮触发
 * @param triggered 是否触发
 */
void MusicSyncTool::on_actionLocal_triggered(bool triggered) {
    openFolder(pathType::LOCAL);
    if (localPath == "") {
        return;
    }
    getMusic(pathType::LOCAL, 1);
    showOperationResult(operationType::LOAD);
}
/**
 * @brief 设置按钮触发
 * @param triggered 是否触发
 */
void MusicSyncTool::on_actionSettings_triggered(bool) { showSettings(); }
/**
 * @brief 关于按钮触发
 * @param triggered 是否触发
 */
void MusicSyncTool::on_actionAbout_triggered(bool triggered) {
    AboutPage about;
    about.exec();
}
/**
 * @brief 本地复制到远程按钮触发
 */
void MusicSyncTool::on_copyToRemote_clicked() {
    if (localPath == "") {
        popError(PET::NPS);
        return;
    }
    QStringList fileList = getSelectedMusic(pathType::LOCAL);
    if (fileList.isEmpty()) {
        popError(PET::NFS);
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, localPath, fileList, remotePath);
}
/**
 * @brief 远程复制到本地按钮触发
 */
void MusicSyncTool::on_copyToLocal_clicked() {
    if (remotePath == "") {
        popError(PET::NPS);
        return;
    }
    QStringList fileList = getSelectedMusic(pathType::REMOTE);
    if (fileList.isEmpty()) {
        popError(PET::NFS);
        return;
    }
    QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, remotePath, fileList, localPath);
}
/**
 * @brief 检测本地重复音乐
 */
void MusicSyncTool::on_actionDupeLocal_triggered(bool triggered) { getDuplicatedMusic(pathType::LOCAL); }
/**
 * @brief 检测远程重复音乐
 */
void MusicSyncTool::on_actionDupeRemote_triggered(bool triggered) { getDuplicatedMusic(pathType::REMOTE); }
/**
 * @brief 刷新本地音乐列表
 */
void MusicSyncTool::on_refreshLocal_clicked() { getMusic(pathType::LOCAL, 1); }
/**
 * @brief 刷新远程音乐列表
 */
void MusicSyncTool::on_refreshRemote_clicked() { getMusic(pathType::REMOTE, 1); }
/**
 * @brief 本地搜索框回车触发
 */
void MusicSyncTool::on_searchLocal_returnPressed() { searchMusic(pathType::LOCAL, ui.searchLocal->text()); }
/**
 * @brief 远程搜索框回车触发
 */
void MusicSyncTool::on_searchRemote_returnPressed() { searchMusic(pathType::REMOTE, ui.searchRemote->text()); }
/**
 * @brief 本地表格双击触发
 * @param row 行
 * @param column 列
 */
void MusicSyncTool::on_tableWidgetLocal_cellDoubleClicked(int row, int column) { setTotalLength(pathType::LOCAL, row); }
/**
 * @brief 远程表格双击触发
 * @param row 行
 * @param column 列
 */
void MusicSyncTool::on_tableWidgetRemote_cellDoubleClicked(int row, int column) {
    setTotalLength(pathType::REMOTE, row);
}
/**
 * @brief 退出按钮触发
 * @param triggered 是否触发
 */
void MusicSyncTool::on_actionExit_triggered(bool triggered) { exit(EXIT_SUCCESS); }
void MusicSyncTool::on_actionClean_log_files_triggered(bool triggered) {
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, tr("提示"), tr("确定要清除所有日志文件吗？"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }
    QDir logDir("log");
    if (!logDir.exists()) {
        return;
    }
    QStringList logFiles = logDir.entryList(QDir::Files);
    for (QString &logFile : logFiles) {
        QFile file("log/" + logFile);
        file.remove();
    }
    QMessageBox::information(this, tr("提示"), tr("日志文件已清除"));
}
/**
 * @brief 播放控制按钮触发
 */
void MusicSyncTool::on_playControl_clicked() {
    if (!mediaPlayer->hasAudio()) {
        popError(PET::NOAUDIO);
        return;
    } else {
        if (mediaPlayer->isPlaying()) {
            mediaPlayer->pause();
            setMediaWidget(playState::PAUSED);
        } else {
            mediaPlayer->play();
            setMediaWidget(playState::PLAYING);
        }
    }
}

void MusicSyncTool::setSliderPosition(qint64 position) {
    ui.playSlider->setValue(position);
    if (position % 60000 / 1000 < 10) {
        ui.playProgress->setText(QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000));
    } else {
        ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
    }
}

void MusicSyncTool::on_playSlider_sliderMoved(int position) {
    mediaPlayer->setPosition(position);
    if (position % 60000 / 1000 < 10) {
        ui.playProgress->setText(QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000));
    } else {
        ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
    }
}

void MusicSyncTool::on_playSlider_sliderPressed() { mediaPlayer->setPosition(ui.playSlider->value()); }

void MusicSyncTool::on_volumeSlider_sliderPressed() {
    audioOutput->setVolume(ui.volumeSlider->value() / 100.0);
    QString text = tr("音量：") + QString::number(ui.volumeSlider->value()) + "%";
    ui.volumeLabel->setText(text);
}

void MusicSyncTool::on_favoriteOnlyLocal_clicked() { getFavoriteMusic(pathType::LOCAL, 1); }

void MusicSyncTool::on_favoriteOnlyRemote_clicked() { getFavoriteMusic(pathType::REMOTE, 1); }

void MusicSyncTool::on_lastPageLocal_clicked() {
    if (localPath == "") {
        popError(PET::NPS);
        return;
    } else if (currentPage[0] == 1) {
        popError(PET::FIRST);
        return;
    }
    if (favoriteOnly[0]) {
        getFavoriteMusic(pathType::LOCAL, --currentPage[0]);
    } else {
        getMusic(pathType::LOCAL, --currentPage[0]);
    }
}
void MusicSyncTool::on_nextPageLocal_clicked() {
    if (localPath == "") {
        popError(PET::NPS);
        return;
    } else if (currentPage[0] == totalPage[0]) {
        popError(PET::LAST);
        return;
    }
    if (favoriteOnly[0]) {
        getFavoriteMusic(pathType::LOCAL, ++currentPage[0]);
    } else {
        getMusic(pathType::LOCAL, ++currentPage[0]);
    }
}

void MusicSyncTool::on_lastPageRemote_clicked() {
    if (remotePath == "") {
        popError(PET::NPS);
        return;
    } else if (currentPage[1] == 1) {
        popError(PET::FIRST);
        return;
    }
    if (favoriteOnly[1]) {
        getFavoriteMusic(pathType::REMOTE, --currentPage[1]);
    } else {
        getMusic(pathType::REMOTE, --currentPage[1]);
    }
}

void MusicSyncTool::on_nextPageRemote_clicked() {
    if (remotePath == "") {
        popError(PET::NPS);
        return;
    } else if (currentPage[1] == totalPage[1]) {
        popError(PET::LAST);
        return;
    }
    if (favoriteOnly[1]) {
        getFavoriteMusic(pathType::REMOTE, ++currentPage[1]);
    } else {
        getMusic(pathType::REMOTE, ++currentPage[1]);
    }
}

void MusicSyncTool::endMedia(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::PlaybackState::StoppedState) {
        ui.playControl->setText(tr("播放"));
        ui.nowPlaying->setText(tr("播放已结束。"));
        ui.playSlider->setValue(0);
        ui.playProgress->setText("00:00");
    }
}

void MusicSyncTool::on_volumeSlider_sliderMoved(int position) {
    audioOutput->setVolume(position / 100.0);
    QString text = tr("音量：") + QString::number(position) + "%";
    ui.volumeLabel->setText(text);
}

void MusicSyncTool::on_volumeSlider_valueChanged(int position) { on_volumeSlider_sliderMoved(position); }

void MusicSyncTool::setTotalLength(pathType path, int row) {
    const QTableWidget &widget = (path == pathType::LOCAL ? *ui.tableWidgetLocal : *ui.tableWidgetRemote);
    QString sql = "SELECT fileName FROM musicInfo WHERE";
    sql += " title = \"" + widget.item(row, 0)->text() + "\" AND artist = \"" + widget.item(row, 1)->text() +
        "\" AND album = \"" + widget.item(row, 2)->text() + "\"";
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    query.exec(sql);
    query.next();
    nowPlaying = query.value(0).toString();
    QString filePath = (path == pathType::LOCAL ? localPath : remotePath) + "/" + nowPlaying;
    mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    TagLib::FileRef f(filePath.toStdWString().c_str());
    if (!f.isNull() && f.audioProperties()) {
        qint64 length = f.audioProperties()->lengthInMilliseconds();
        setNowPlayingTitle(nowPlaying);
        mediaPlayer->setPlaybackRate(1.0);
        mediaPlayer->setPosition(0);
        mediaPlayer->play();
        setMediaWidget(playState::PLAYING);
        ui.playSlider->setMaximum(length);
        ui.playSlider->setValue(0);
        ui.playProgress->setText("00:00");
    }
}

void MusicSyncTool::getFavoriteMusic(pathType path, short page) {
    if (path == pathType::LOCAL && localPath == "") {
        popError(PET::NPS);
        return;
    } else if (path == pathType::REMOTE && remotePath == "") {
        popError(PET::NPS);
        return;
    }
    if (entity.favoriteTag == "") {
        popError(PET::NFT);
        return;
    }
    emit started();
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    QString sql = "SELECT COUNT(*) FROM musicInfo WHERE favorite = 1";
    query.exec(sql);
    query.next();
    int totalSize = query.value(0).toInt();
    totalPage[(path == pathType::LOCAL ? 0 : 1)] = totalSize / PAGESIZE + 1;
    sql = "SELECT * FROM musicInfo WHERE favorite = 1";
    switch (entity.sortBy) {
    case sortByEnum::TITLE:
        sql += " ORDER BY title";
        break;
    case sortByEnum::ARTIST:
        sql += " ORDER BY artist";
        break;
    case sortByEnum::ALBUM:
        sql += " ORDER BY album";
        break;
    default:
        sql += " ORDER BY title";
        break;
    }
    switch (entity.orderBy) {
    case orderByEnum::ASC:
        sql += " ASC";
        break;
    case orderByEnum::DESC:
        sql += " DESC";
        break;
    default:
        sql += " ASC";
        break;
    }
    sql += " LIMIT " + QString::number(PAGESIZE) + " OFFSET " +
        QString::number((currentPage[(path == pathType::LOCAL ? 0 : 1)] - 1) * PAGESIZE);
    query.exec(sql);
    QTableWidget *targetTable = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
    (path == pathType::LOCAL ? ui.pageLocal : ui.pageRemote)
        ->setText(QString::number(currentPage[0]) + "/" + QString::number(totalPage[0]));
    targetTable->clearContents();
    targetTable->setRowCount(PAGESIZE);
    favoriteOnly[(path == pathType::LOCAL ? 0 : 1)] = true;
    emit total(totalSize);
    for (int i = 0; query.next(); i++) {
        for (int j = 0; j < 6; j++) {
            if (query.value(j) != 0) {
                targetTable->setItem(query.at(), j, new QTableWidgetItem(query.value(j).toString()));
                emit current(i);
            }
        }
    }
    emit finished();
}

void MusicSyncTool::connectSlots() const {
    connect(this, &MusicSyncTool::total, loading, &LoadingPage::setTotal);
    connect(this, &MusicSyncTool::current, loading, &LoadingPage::setProgress);
    connect(this, &MusicSyncTool::started, loading, &LoadingPage::showPage);
    connect(this, &MusicSyncTool::finished, loading, &LoadingPage::stopPage);
    connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::showOperationResult);
    connect(this, &MusicSyncTool::loadFinished, this, &MusicSyncTool::showOperationResult);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MusicSyncTool::setSliderPosition);
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &MusicSyncTool::endMedia);
    connect(this, &MusicSyncTool::addToErrorListConcurrent, this, QOverload<QString, loadErrorType>::of(&MusicSyncTool::addToErrorList));
}

bool MusicSyncTool::isFull(QString filePath, QString target) { 
    QFileInfo musicInfo(filePath);
    QDir targetInfo(target);
    QStorageInfo storage(targetInfo);
    if (musicInfo.size() > storage.bytesAvailable()) {
        return true;
    }
    return false;
}
