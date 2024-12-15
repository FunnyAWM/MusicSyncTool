#pragma warning(disable:6031)
#include "MusicSyncTool.h"
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>
#include <iostream>

MusicSyncTool::MusicSyncTool(QWidget* parent)
	: QMainWindow(parent), ignoreLyric(false), sortBy(Settings::TITLE), mediaPlayer(new QMediaPlayer(this)), audioOutput(new QAudioOutput(this)) {
	ui.setupUi(this);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
	dbLocal = QSqlDatabase::addDatabase("QSQLITE", "musicInfoLocal");
	dbRemote = QSqlDatabase::addDatabase("QSQLITE", "musicInfoRemote");
	queryLocal = QSqlQuery(dbLocal);
	queryRemote = QSqlQuery(dbRemote);
	ui.tableWidgetLocal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetRemote->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QFile file("settings.json");
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "[WARN] No settings file found, creating new setting file named settings.json";
		return;
	}
	settings = QJsonDocument::fromJson(file.readAll());
	if (settings.isNull()) {
		qDebug() << "[WARN] Invalid settings file, creating new setting file named settings.json";
		return;
	}
	QJsonObject obj = settings.object();
	ignoreLyric = obj["ignoreLyric"].toBool();
	sortBy = obj["sortBy"].toInt();
	language = obj["language"].toString();
	QTranslator translator;
	if (language == "Chinese") {
		qDebug() << translator.load(":/translations/Translation_zh_Hans.qm");
	} else {
		qDebug() << translator.load(":/translations/Translation_en_US.qm");
	}
	qApp->installTranslator(&translator);
	ui.retranslateUi(this);
	file.close();
    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5);
    ui.volumeSlider->setValue(50);
    ui.volumeLabel->setText(tr("音量：") + "50%");
    connect(mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(setSliderPosition(qint64)));
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
	QSqlDatabase& db = path == pathType::LOCAL ? dbLocal : dbRemote;
	if (!db.open()) {
		qDebug() << "[FATAL] Error opening database:" << db.lastError().text().toStdString();
		exit(EXIT_FAILURE);
	}
}
/**
* @brief 获取音乐信息
* @param path 路径类型（本地或远程）
*/
void MusicSyncTool::getMusic(pathType path) {
	qDebug() << "[INFO] Scanning started";
	clock_t start = clock();
	if (localPath == "" && remotePath == "") {
		qDebug() << "[WARN] No path selected";
		return;
	}
	emit showLoading();
	QDir dir(path == pathType::LOCAL ? localPath : remotePath);
	QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, track INT, fileName TEXT)";
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
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
		if (oldFileList.contains(newFileList.at(i)) || newFileList.at(i).contains(".lrc")) {
			newFileList.removeAt(i);
			i = -1;
		}
	}
	newFileList.removeOne("musicInfo.db");
	
	for (int i = 0; i < newFileList.size(); i++) {
		QString file = (path == pathType::LOCAL ? localPath : remotePath) + "/" + newFileList.at(i).toUtf8();
		TagLib::FileRef f(file.toStdWString().c_str());
		if (!f.isNull() && f.tag()) {
			TagLib::Tag* tag = f.tag();
			sql = "INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (\""
				+ QString::fromStdString(tag->title().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->artist().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->album().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->genre().to8Bit(true))
				+ "\", " + QString::number(tag->year())
				+ ", " + QString::number(tag->track())
				+ ", \"" + newFileList.at(i)
				+ "\")";
			query.exec(sql);
		} else {
			continue;
		}
	}
	sql = "SELECT COUNT(*) FROM musicInfo";
	query.exec(sql);
	query.next();
	int tableSize = query.value(0).toInt();
	sql = "SELECT title, artist, album, genre, year, track FROM musicInfo ORDER BY";
	switch (sortBy) {
	case Settings::TITLE:
		sql += " title";
		break;
	case Settings::ARTIST:
		sql += " artist";
		break;
	case Settings::ALBUM:
		sql += " album";
		break;
	default:
		sql += " title";
		break;
	}
	query.exec(sql);
	QTableWidget* targetTable = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
	targetTable->clearContents();
	targetTable->setRowCount(tableSize);
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				targetTable->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
	}
	clock_t end = clock();
	emit stopLoading();
	qDebug() << "[INFO] Scanning finished in" << double(end - start) / CLOCKS_PER_SEC << "seconds";
}
/**
* @brief 搜索音乐
* @param path 路径类型（本地或远程）
* @param text 搜索文本
*/
void MusicSyncTool::searchMusic(pathType path, QString text)
{
	if (text == "") {
		getMusic(path);
		return;
	}
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->clearContents();
	QString sql = "SELECT COUNT(*) FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text + "%\" OR album LIKE \"%" + text + "%\"";
	query.exec(sql);
	query.next();
	int tableSize = query.value(0).toInt();
	(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setRowCount(tableSize);
	sql = "SELECT * FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text + "%\" OR album LIKE \"%" + text + "%\"";
	query.exec(sql);
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
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
		QMessageBox::critical(this, "错误", "请先选择路径！");
		return QStringList();
	}
	ShowDupe dp;
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	QString sql = "SELECT * FROM musicInfo ORDER BY title";
	query.exec(sql);
	QString fast;
	QString fastFileName;
	QString slow;
	QString slowFileName;
	QStringList dupeList;
	query.next();
	slow = query.value(0).toString() + query.value(1).toString();
	slowFileName = query.value(6).toString();
	while (query.next()) {
		fast = query.value(0).toString() + query.value(1).toString();
		fastFileName = query.value(6).toString();
		if (fast == slow) {
			dupeList.append(fastFileName);
			dupeList.append(slowFileName);
		}
		slow = fast;
		slowFileName = fastFileName;
	}
	for (int i = 0; i < dupeList.size(); i++) {
		qDebug() << "[INFO] Found duplicated music named" << dupeList.at(i) << "at" << (path == pathType::LOCAL ? localPath : remotePath);
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
	const QTableWidget* const table = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
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
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	QString titleListString;
	QString artistListString;
	for (int i = 0; i < selectedRows.size() - 1; i++) {
		titleListString += "\"" + titleList.at(i) + "\", ";
		artistListString += "\"" + artistList.at(i) + "\", ";
	}
	titleListString += "\"" + titleList.at(selectedRows.size() - 1) + "\"";
	artistListString += "\"" + artistList.at(selectedRows.size() - 1) + "\"";
	QString sql = "SELECT fileName FROM musicInfo WHERE title IN("+ titleListString + ") AND artist IN(" + artistListString + ")";
	QStringList fileList;
	query.exec(sql);
	while (query.next()) {
		fileList.append(query.value(0).toString());
	}
	return fileList;
}
/**
* @brief 显示设置页面
*/
void MusicSyncTool::showSettings()
{
	Settings* page = new Settings();
	connect(page, SIGNAL(confirmPressed(Settings::set)), this, SLOT(saveSettings(Settings::set)));
	page->show();
}
/**
* @brief 保存设置
* @param entity 配置实体
*/
void MusicSyncTool::saveSettings(Settings::set entity)
{
	QFile file("settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		qDebug() << "[FATAL] Error opening settings file";
		QMessageBox::critical(this, "错误", "无法打开设置文件！");
		exit(EXIT_FAILURE);
	}
	QJsonObject obj;
	obj["ignoreLyric"] = entity.ignoreLyric;
	obj["sortBy"] = entity.sortBy;
	obj["language"] = entity.language;
	int temp = sortBy;
	QString templang = entity.language;
	ignoreLyric = entity.ignoreLyric;
	sortBy = entity.sortBy;
	if (temp != sortBy) {
		getMusic(pathType::LOCAL);
		getMusic(pathType::REMOTE);
	}
	if (templang != entity.language) {
		qDebug() << "[INFO] Language changed to" << entity.language;
		ui.retranslateUi(this);
	}
	qDebug() << "[INFO] IgnoreLyric:" << entity.ignoreLyric;
	qDebug() << "[INFO] SortBy:" << entity.sortBy;
	settings.setObject(obj);
	file.write(settings.toJson());
}
/**
* @brief 复制音乐
* @param source 源路径
* @param fileList 音乐列表
* @param target 目标路径
*/
void MusicSyncTool::copyMusic(const QString source, QStringList fileList, const QString target) const {
	TagLib::String key = "LYRICS";
	if (source == "" || fileList.isEmpty() || target == "") {
		return;
	}
	QDir dir(target);
	if (dir.isEmpty()) {
		dir.mkpath(target);
	}
	QStringList supportedFormat = { ".mp3", ".flac", ".ape" };
	QStringList errorList;
	for (QString& file : fileList) {
		QString sourceFile = source + "/" + file;
		QString targetFile = target + "/" + file;
		QString lyric;
		QString lyricTarget;
		for (QString& format : supportedFormat) {
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
			errorList.append("复制" + file + "失败：文件已存在");
			continue;
		}
		else if(!ignoreLyric) {
			if (!QFile::exists(lyric)) {
				TagLib::FileRef f(sourceFile.toStdWString().c_str());
				if (!f.isNull() && f.tag()) {
					TagLib::Tag* tag = f.tag();
					if (!tag->properties().contains(key)) {
						qDebug() << "[WARN] Lyric file not found, skipping" << lyric;
						errorList.append("复制" + file + "失败：找不到歌词文件");
						continue;
					}
				}
			}
			else {
				QFile::copy(lyric, lyricTarget);
			}
		}
		QFile::copy(sourceFile, targetFile);
	}
	if (!errorList.isEmpty()) {
		CopyResult* result = new CopyResult();
		QString errorString;
		for (QString& error : errorList) {
			errorString += error + "\n";
		}
		result->setText(errorString);
		result->exec();
	}
}
void MusicSyncTool::setNowPlayingTitle(QString file) { ui.nowPlaying->setText(tr("正在播放：") + file); }
/**
* @brief 获取语言
* @return 语言
*/
QString MusicSyncTool::getLanguage()
{
	return language;
}
/**
* @brief 显示加载页面
*/
void MusicSyncTool::showLoading() {
	loading.show();
}
/**
* @brief 关闭加载页面
*/
void MusicSyncTool::stopLoading() {
	loading.close();
}
/**
* @brief 本地音乐按钮触发
* @param triggered 是否触发
*/
void MusicSyncTool::on_actionRemote_triggered(bool triggered) {
	openFolder(pathType::REMOTE);
	if (remotePath == "") {
		return;
	}
	getMusic(pathType::REMOTE);
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
	getMusic(pathType::LOCAL);
}
/**
* @brief 设置按钮触发
* @param triggered 是否触发
*/
void MusicSyncTool::on_actionSettings_triggered(bool)
{
	showSettings();
}
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
		QMessageBox::critical(this, "错误", "请先选择路径！");
		return;
	}
	QStringList fileList = getSelectedMusic(pathType::LOCAL);
	copyMusic(localPath, fileList, remotePath);
	getMusic(pathType::REMOTE);
}
/**
* @brief 远程复制到本地按钮触发
*/
void MusicSyncTool::on_copyToLocal_clicked() {
	if (remotePath == "") {
		QMessageBox::critical(this, "错误", "请先选择路径！");
		return;
	}
	QStringList fileList = getSelectedMusic(pathType::REMOTE);
	copyMusic(remotePath, fileList, localPath);
	getMusic(pathType::LOCAL);
}
/**
* @brief 检测本地重复音乐
*/
void MusicSyncTool::on_actionDupeLocal_triggered(bool triggered) {
	getDuplicatedMusic(pathType::LOCAL);
}
/**
* @brief 检测远程重复音乐
*/
void MusicSyncTool::on_actionDupeRemote_triggered(bool triggered) {
	getDuplicatedMusic(pathType::REMOTE);
}
/**
* @brief 刷新本地音乐列表
*/
void MusicSyncTool::on_refreshLocal_clicked() {
	getMusic(pathType::LOCAL);
}
/**
* @brief 刷新远程音乐列表
*/
void MusicSyncTool::on_refreshRemote_clicked() {
	getMusic(pathType::REMOTE);
}
/**
* @brief 本地搜索框回车触发
*/
void MusicSyncTool::on_searchLocal_returnPressed()
{
	searchMusic(pathType::LOCAL, ui.searchLocal->text());
}
/**
* @brief 远程搜索框回车触发
*/
void MusicSyncTool::on_searchRemote_returnPressed()
{
	searchMusic(pathType::REMOTE, ui.searchRemote->text()); }

void MusicSyncTool::on_tableWidgetLocal_cellDoubleClicked(int row, int column) { setTotalLength(pathType::LOCAL, row); }

/**
* @brief 关于按钮触发
* @param triggered 是否触发
*/
void MusicSyncTool::on_actionExit_triggered(bool triggered) { exit(EXIT_SUCCESS); }

void MusicSyncTool::on_playControl_clicked() {
    if (!mediaPlayer->hasAudio()) {
        QMessageBox::critical(this, tr("错误"), tr("没有选定音频！"));
        return;
    } else {
        if (mediaPlayer->isPlaying()) {
            mediaPlayer->pause();
            ui.playControl->setText(tr("播放"));
        } else {
            mediaPlayer->play();
            ui.playControl->setText(tr("暂停"));
        }
    }
}

void MusicSyncTool::setSliderPosition(qint64 position) { 
	ui.playSlider->setValue(position);
    ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
}

void MusicSyncTool::on_playSlider_sliderMoved(int position) { 
	mediaPlayer->setPosition(position); 
	ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
}

void MusicSyncTool::on_playSlider_sliderPressed() { mediaPlayer->setPosition(ui.playSlider->value()); }

void MusicSyncTool::on_volumeSlider_sliderPressed() { 
	audioOutput->setVolume(ui.volumeSlider->value() / 100.0); 
	ui.volumeLabel->setText(tr("音量：") + QString::number(ui.volumeSlider->value()) + "%");
}

void MusicSyncTool::on_volumeSlider_sliderMoved(int position) {
    audioOutput->setVolume(position / 100.0);
    ui.volumeLabel->setText(tr("音量：") + QString::number(position) + "%");
}


void MusicSyncTool::setTotalLength(pathType path, int row) {
    const QTableWidget &widget = (path == pathType::LOCAL ? *ui.tableWidgetLocal : *ui.tableWidgetRemote);
    QString sql = "SELECT fileName FROM musicInfo WHERE";
    sql += " title = \"" + widget.item(row, 0)->text() + "\" AND artist = \"" + widget.item(row, 1)->text() +
    "\" AND album = \"" + widget.item(row, 2)->text() + "\"";
    QSqlQuery &query = (path == pathType::LOCAL ? queryLocal : queryRemote);
    query.exec(sql);
    query.next();
    QString fileName = query.value(0).toString();
    QString filePath = (path == pathType::LOCAL ? localPath : remotePath) + "/" + fileName;
    mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    TagLib::FileRef f(filePath.toStdWString().c_str());
    if (!f.isNull() && f.audioProperties()) {
        qint64 length = f.audioProperties()->lengthInMilliseconds();
        setNowPlayingTitle(fileName);
        mediaPlayer->setPlaybackRate(1.0);
        mediaPlayer->setPosition(0);
        mediaPlayer->play();
        ui.playControl->setText(tr("暂停"));
        ui.playSlider->setMaximum(length);
        ui.playSlider->setValue(0);
        ui.playProgress->setText("00:00");
    }
}
