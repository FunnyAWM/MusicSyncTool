// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#pragma warning(disable : 6031)
#include "MusicSyncTool.h"

#include <algorithm>
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

/*
 * @brief Default constructor
 */
MusicSyncTool::MusicSyncTool(QWidget* parent) :
	QMainWindow(parent), translator(new QTranslator(this)), mediaPlayer(new QMediaPlayer(this)),
	audioOutput(new QAudioOutput(this)) {
	initDatabase();
	loadSettings();
	initUI();
	loadLanguage();
	initMediaPlayer();
	connectSlots();
	setMediaWidget(PlayState::STOPPED);
}

/*
 * @brief Destructor
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

/*
 * @brief Initialize database
 */
void MusicSyncTool::initDatabase() {
	dbLocal = QSqlDatabase::addDatabase("QSQLITE", "musicInfoLocal");
	dbRemote = QSqlDatabase::addDatabase("QSQLITE", "musicInfoRemote");
	queryLocal = QSqlQuery(dbLocal);
	queryRemote = QSqlQuery(dbRemote);
}

/*
 * @brief Load settings from settings file
 */
void MusicSyncTool::loadSettings() {
	QFile file("settings.json");
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "[WARN] No settings file found, creating new setting file named settings.json";
		file.close();
		loadDefaultSettings();
		return;
	}
	const QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
	if (settings.isNull()) {
		qWarning() << "[WARN] Invalid settings file, creating new setting file named settings.json";
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
	QList<LyricIgnoreRuleSingleton> rules;
	QJsonArray rulesArray = obj["rules"].toArray();
	for (QJsonValue rule : rulesArray) {
		QJsonObject ruleObj = rule.toObject();
		rules.append(LyricIgnoreRuleSingleton(
			LyricIgnoreRuleSingleton::stringToIgnoreRules(ruleObj["ruleType"].toString()),
			LyricIgnoreRuleSingleton::stringToLyricRules(ruleObj["ruleField"].toString()),
			ruleObj["ruleName"].toString()));
	}
	entity.rules = rules;
	file.close();
}

/*
 * @brief Load default settings(if settings.json don't exists)
 */
void MusicSyncTool::loadDefaultSettings() {
	QFile file("settings.json");
	file.open(QIODevice::WriteOnly);
	QJsonObject obj;
	obj["ignoreLyric"] = false;
	obj["sortBy"] = TITLE;
	obj["orderBy"] = ASC;
	obj["language"] = "";
	obj["favoriteTag"] = "";
	obj["rules"] = QJsonArray();
	file.write(QJsonDocument(obj).toJson());
	file.close();
}

/*
 * @brief Initialize media player module
 */
void MusicSyncTool::initMediaPlayer() const {
	mediaPlayer->setAudioOutput(audioOutput);
	audioOutput->setVolume(0.5);
}

/*
 * @brief Load language file by settings
 */
void MusicSyncTool::loadLanguage() {
	QFile file(QApplication::applicationDirPath() + "/translations/langinfo.json");
	if (!file.open(QIODevice::ReadOnly)) {
		qFatal() << "[FATAL] Error opening langinfo.json:" << file.errorString();
		exit(EXIT_FAILURE);
	}
	const QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
	file.close();
	QJsonArray langArray = langinfo.array();
	for (QJsonValue lang : langArray) {
		if (entity.language == "") {
			entity.language = "中文";
		}
		if (QJsonObject langObj = lang.toObject(); langObj["lang"].toString() == entity.language) {
			// ReSharper disable once CppNoDiscardExpression
			translator->load("translations/" + langObj["fileName"].toString());
			break;
		}
	}
	qApp->installTranslator(translator);
	ui.retranslateUi(this);
}

/*
 * @brief Initialize UI components
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

/*
 * @brief Pop up error windows according to error type
 * @param Error type
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
	}
}

/*
 * @brief Set up media player
 * @param Play state
 */
void MusicSyncTool::setMediaWidget(const PlayState state) const {
	if (state == PlayState::PLAYING) {
		ui.playControl->setText(tr("暂停"));
		setNowPlayingTitle(nowPlaying);
	}
	else if (state == PlayState::PAUSED) {
		ui.playControl->setText(tr("播放"));
	}
	else if (state == PlayState::STOPPED) {
		ui.playControl->setText(tr("播放"));
		ui.nowPlaying->setText(tr("播放已结束。"));
	}
	ui.volumeLabel->setText(tr("音量：") + QString::number(ui.volumeSlider->value()) + "%");
}

/*
 * @brief Pop up open folder dialog and load path to database
 * @param Path type
 */
void MusicSyncTool::openFolder(const PathType path) {
	const QString dir = QFileDialog::getExistingDirectory();
	if (dir == "") {
		return;
	}
	(path == PathType::LOCAL ? localPath : remotePath) = dir;
	if (path == PathType::LOCAL) {
		dbLocal.setDatabaseName(localPath + "/musicInfo.db");
	}
	else {
		dbRemote.setDatabaseName(remotePath + "/musicInfo.db");
	}
	if (QSqlDatabase& db = path == PathType::LOCAL ? dbLocal : dbRemote; !db.open()) {
		qFatal() << "[FATAL] Error opening database:" << db.lastError().text().toStdString();
		exit(EXIT_FAILURE);
	}
}

/*
 * @brief Get music files from selected path(multi-thread caller)
 * @param Path type
 * @param Page number
 */
void MusicSyncTool::getMusic(PathType path, unsigned short page) {
	qDebug() << "[INFO] Scanning started";
	if (localPath == "" && remotePath == "") {
		qWarning() << "[WARN] No path selected";
		return;
	}
	QFuture<void> future = QtConcurrent::run(&MusicSyncTool::getMusicConcurrent, this, path, page);
}

/*
 * @brief Get music files from selected path and load it to tables(main operation, concurrent)
 * @param Path type
 * @param Page number
 */
void MusicSyncTool::getMusicConcurrent(PathType path, unsigned short page) {
	clock_t start = clock();
	emit started();
	favoriteOnly[path == PathType::LOCAL ? 0 : 1] = false;
	QStringList pathForLog = (path == PathType::LOCAL ? localPath : remotePath).split("/");
	QString logFileNameBuilder = "lastScan";
	pathForLog[0].remove(":");
	for (const QString& tempStr : pathForLog) {
		logFileNameBuilder += " - " + tempStr;
	}
	logFileNameBuilder += ".log";
	QFile logFile("log/" + logFileNameBuilder);
	QDateTime dateTime;
	if (logFile.open(QIODevice::ReadOnly)) {
		QTextStream in(&logFile);
		dateTime = QDateTime::fromString(in.readLine());
	}
	else {
		qWarning() << "[WARN] No last scan log found, scanning all files";
		// Set last scan time to 1970-01-01 00:00:00 if no log found
		dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
	}
	QDir dir(path == PathType::LOCAL ? localPath : remotePath);
	QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
		"track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)";
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
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
			query.prepare("DELETE FROM musicInfo WHERE fileName = :fileName");
			query.bindValue(":fileName", oldFileList.at(i));
			query.exec();
			oldFileList.removeAt(i);
			i = i - 2 < -1 ? -1 : i - 2;
		}
	}
	for (int i = 0; i < newFileList.size(); i++) {
		if (oldFileList.contains(newFileList.at(i)) || !isFormatSupported(newFileList.at(i))) {
			newFileList.removeAt(i);
			i = i - 2 < 0 ? -1 : i - 2;
		}
	}
	newFileList.removeOne("musicInfo.db");
	emit total(newFileList.size() + oldFileList.size());
	emit current(0);
	TagLib::String key(entity.favoriteTag.toStdString());
	for (int i = 0; i < newFileList.size(); i++) {
		QString file = (path == PathType::LOCAL ? localPath : remotePath) + "/" + newFileList.at(i).toUtf8();
		TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
		f = TagLib::FileRef(file.toStdWString().c_str());
#else
		f = TagLib::FileRef(file.toStdString().c_str());
#endif
		if (!f.isNull() && f.tag()) {
			TagLib::Tag* tag = f.tag();
			query.prepare("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
				":artist, :album, :genre, :year, :track, :fileName)");
			query.bindValue(":title", tag->title().toCString(true));
			query.bindValue(":artist", tag->artist().toCString(true));
			query.bindValue(":album", tag->album().toCString(true));
			query.bindValue(":genre", tag->genre().toCString(true));
			query.bindValue(":year", tag->year());
			query.bindValue(":track", tag->track());
			query.bindValue(":fileName", newFileList.at(i));
			if (!query.exec()) {
				qWarning() << "[WARN] Error inserting data to database:" << query.lastError().text();
			}
			emit current(i);
		}
	}
	query.clear();
	if (entity.favoriteTag != "") {
		for (int i = 0; i < oldFileList.size(); i++) {
			QString file = (path == PathType::LOCAL ? localPath : remotePath) + "/" + oldFileList.at(i).toUtf8();
			if (QFile(file).fileTime(QFileDevice::FileModificationTime) <= dateTime) {
				continue;
			}
			TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
			f = TagLib::FileRef(file.toStdWString().c_str());
#else
			f = TagLib::FileRef(file.toStdString().c_str());
#endif
			if (!f.isNull() && f.tag()) {
				TagLib::Tag* tag = f.tag();
				query.prepare("UPDATE musicInfo SET favorite = :favorite WHERE fileName = :fileName");
				query.bindValue(":favorite", tag->properties().contains(key));
				query.bindValue(":fileName", oldFileList.at(i));
				query.exec();
				emit current(i + newFileList.size());
			}
		}
	}
	if (!entity.rules.isEmpty()) {
		for (int i = 0; i < oldFileList.size(); i++) {
			QString file = (path == PathType::LOCAL ? localPath : remotePath) + "/" + oldFileList.at(i).toUtf8();
			if (QFile(file).fileTime(QFileDevice::FileModificationTime) <= dateTime) {
				continue;
			}
			TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
			f = TagLib::FileRef(file.toStdWString().c_str());
#else
			f = TagLib::FileRef(file.toStdString().c_str());
#endif
			if (!f.isNull() && f.tag()) {
				emit current(i + newFileList.size());
				TagLib::Tag* tag = f.tag();
				for (const LyricIgnoreRuleSingleton& rule : entity.rules) {
					if (isRuleHit(rule, tag)) {
						query.prepare("UPDATE musicInfo SET ruleHit = 1 WHERE fileName = :fileName");
						query.bindValue(":fileName", oldFileList.at(i));
						break;
					}
				}
			}
		}
	}
	sql = "SELECT COUNT(*) FROM musicInfo";
	query.exec(sql);
	query.next();
	totalPage[path == PathType::LOCAL ? 0 : 1] = static_cast<short>(query.value(0).toInt() / PAGESIZE + 1);
	short lastPageSize = static_cast<short>(query.value(0).toInt() % PAGESIZE);
	emit total(PAGESIZE);
	sql = "SELECT title, artist, album, genre, year, track FROM musicInfo ORDER BY";
	switch (entity.sortBy) {
	case TITLE:
		sql += " title";
		break;
	case ARTIST:
		sql += " artist";
		break;
	case ALBUM:
		sql += " album";
		break;
	default:
		sql += " title";
		break;
	}
	switch (entity.orderBy) {
	case ASC:
		sql += " ASC";
		break;
	case DESC:
		sql += " DESC";
		break;
	default:
		sql += " ASC";
		break;
	}
	sql += " LIMIT " + QString::number(PAGESIZE) + " OFFSET " +
		QString::number((page - 1) * PAGESIZE);
	query.exec(sql);
	QTableWidget* targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
	targetTable->clearContents();
	if (currentPage[(path == PathType::LOCAL ? 0 : 1)] == totalPage[(path == PathType::LOCAL ? 0 : 1)]) {
		targetTable->setRowCount(lastPageSize);
	}
	else {
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
	qDebug() << "[INFO] Scanning finished in" << static_cast<double>(end - start) / CLOCKS_PER_SEC << "seconds";
	if (QDir logDir("log"); !logDir.exists()) {
		if (!logDir.mkpath(".")) {
			qFatal() << "[FATAL] Cannot create log directory";
			exit(EXIT_FAILURE);
		}
	}
	QFile timeFile("log/" + logFileNameBuilder);
	if (!timeFile.open(QIODevice::WriteOnly)) {
		qWarning() << "[WARN] Cannot write to lastScan.log";
		return;
	}
	(path == PathType::LOCAL ? ui.pageLocal : ui.pageRemote)
		->setText(QString::number(currentPage[(path == PathType::LOCAL ? 0 : 1)]) + "/" +
			QString::number(totalPage[(path == PathType::LOCAL ? 0 : 1)]));
	QTextStream out(&timeFile);
	out << QDateTime::currentDateTime().toString() << "\n";
	timeFile.close();
	emit finished();
}

/*
 * @brief Search music files by text
 * @param Path type
 * @param Search text
 */
void MusicSyncTool::searchMusic(const PathType path, const QString& text) {
	if (text == "") {
		getMusic(path, 1);
		return;
	}
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
	(path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->clearContents();
	query.prepare("SELECT COUNT(*) FROM musicInfo WHERE title LIKE :text OR artist LIKE :text OR album LIKE :text");
	query.bindValue(":text", "%" + text + "%");
	query.exec();
	query.next();
	const int tableSize = query.value(0).toInt();
	(path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setRowCount(tableSize);
	query.prepare(
		"SELECT title, artist, album, genre, year, track FROM musicInfo WHERE title LIKE :text OR artist LIKE "
		":text OR album LIKE :text");
	query.bindValue(":text", "%" + text + "%");
	query.exec();
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				(path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)
					->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
	}
}

/*
 * @brief Add file to error list
 * @param File name
 * @param Error type
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

/*
 * @brief Add file to error list
 * @param File name
 * @param Error type
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

/*
 * @brief Get duplicated music and show it
 * @param Path type(local or remote)
 */
QStringList MusicSyncTool::getDuplicatedMusic(const PathType path) {
	if (const QString selectedPath = path == PathType::LOCAL ? localPath : remotePath; selectedPath == "") {
		qWarning() << "[WARN] No path selected";
		popError(PET::NPS);
		return {};
	}
	ShowDupe dp;
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
	const QString sql = "SELECT title, artist, album, year, fileName FROM musicInfo ORDER BY title";
	query.exec(sql);
	QStringList dupeList;
	query.next();
	QString slow = query.value(0).toString() + ":" + query.value(1).toString() + ":" + query.value(2).toString() + ":" +
		query.value(3).toString();
	QString slowFileName = query.value(4).toString();
	while (query.next()) {
		QString fast = query.value(0).toString() + ":" + query.value(1).toString() + ":" + query.value(2).toString() +
			":" +
			query.value(3).toString();
		QString fastFileName = query.value(4).toString();
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
	for (const auto& i : dupeList) {
		qDebug() << "[INFO] Found duplicated music named" << i << "at"
			<< (path == PathType::LOCAL ? localPath : remotePath);
	}
	for (const auto& i : dupeList) {
		dp.add(i);
	}
	dp.exec();
	return dupeList;
}

/*
 * @brief Get selected music files
 * @param Path type
 * @return Selected music files
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
	for (const int& i : selectedRows) {
		titleList.append(table->item(i, 0)->text());
		artistList.append(table->item(i, 1)->text());
	}
	if (selectedRows.empty()) {
		return {};
	}
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
	QString titleListString;
	QString artistListString;
	for (int i = 0; i < selectedRows.size() - 1; i++) {
		titleListString += "\"" + titleList.at(i) + "\", ";
		artistListString += "\"" + artistList.at(i) + "\", ";
	}
	titleListString += "\"" + titleList.at(selectedRows.size() - 1) + "\"";
	artistListString += "\"" + artistList.at(selectedRows.size() - 1) + "\"";
	query.prepare("SELECT fileName, ruleHit FROM musicInfo WHERE title IN ( :title ) AND artist IN ( :artist )");
	query.bindValue(":title", titleListString);
	query.bindValue(":artist", artistListString);
	auto fileList = QStringList();
	query.exec();
	while (query.next()) {
		fileList.append(query.value(0).toString() + ":" + QString::number(query.value(1).toInt()));
	}
	return fileList;
}

/*
 * @brief Show settings dialog
 */
void MusicSyncTool::showSettings() const {
	const auto page = new Settings();
	connect(page, SIGNAL(confirmPressed(set)), this, SLOT(saveSettings(set)));
	page->show();
}

/*
 * @brief Save settings to settings file
 * @param Entity
 */
void MusicSyncTool::saveSettings(const set& entityParam) {
	QFile file("settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		qFatal() << "[FATAL] Error opening settings file";
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
	const QList<LyricIgnoreRuleSingleton> tempRules = this->entity.rules;
	const QString tempTag = this->entity.favoriteTag;
	this->entity = entityParam;
	if (tempSort != entityParam.sortBy || tempOrder != entityParam.orderBy) {
		if (localPath != "") {
			getMusic(PathType::LOCAL, 1);
		}
		if (remotePath != "") {
			getMusic(PathType::REMOTE, 1);
		}
	}
	if (tempRules != entity.rules || entityParam.favoriteTag != tempTag) {
		cleanLog();
		if (localPath != "") {
			getMusic(PathType::LOCAL, 1);
		}
		if (remotePath != "") {
			getMusic(PathType::REMOTE, 1);
		}
	}
	loadLanguage();
	qDebug() << "[INFO] IgnoreLyric:" << this->entity.ignoreLyric;
	qDebug() << "[INFO] SortBy:" << this->entity.sortBy;
	qDebug() << "[INFO] OrderBy:" << this->entity.orderBy;
	qDebug() << "[INFO] Language:" << this->entity.language;
	qDebug() << "[INFO] FavoriteTag:" << this->entity.favoriteTag;
	int i = 1;
	for (const auto& rule : entityParam.rules) {
		qDebug() << "[INFO] Rule No." << i++ << ":";
		qDebug() << "[INFO] RuleTypeConverter:" << rule.getRuleTypeStr();
		qDebug() << "[INFO] RuleFieldConverter:" << rule.getRuleFieldStr();
		qDebug() << "[INFO] Rule:" << rule.getRuleName();
	}
	QJsonDocument settings;
	settings.setObject(obj);
	file.write(settings.toJson());
	file.close();
}

/*
 * @brief Copy music files from source to target
 * @param Source path
 * @param File list
 * @param Target path
 */
void MusicSyncTool::copyMusic(const QString& source, const QStringList& fileList, const QString& target) {
	const TagLib::String key = "LYRICS";
	emit started();
	emit total(fileList.size());
	if (const QDir dir(target); dir.isEmpty()) {
		if (!dir.mkpath(target)) {
			qFatal() << "[FATAL] Error creating directory:" << target;
			exit(EXIT_FAILURE);
		}
	}
	for (const QString& file : fileList) {
		QStringList list = file.split(":");
		QString sourceFile = source + "/" + list.at(0);
		QString targetFile = target + "/" + list.at(0);
		QString lyric;
		QString lyricTarget;
		for (const QString& format : supportedFormat) {
			if (sourceFile.contains(format)) {
				QString temp = sourceFile;
				QString tempTarget = targetFile;
				lyric = temp.replace(format, ".lrc");
				lyricTarget = tempTarget.replace(format, ".lrc");
				break;
			}
		}
		if (QFile::exists(targetFile)) {
			qWarning() << "[WARN] File existed, skipping" << targetFile;
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
					if (const TagLib::Tag* tag = f.tag(); !tag->properties().contains(key)) {
						qWarning() << "[WARN] Lyric file not found, skipping" << lyric;
						addToErrorList(list.at(0), FileErrorType::LNF);
						continue;
					}
				}
			}
			else {
				if (isFull(lyric, target)) {
					addToErrorList(list.at(0), FileErrorType::DISKFULL);
					continue;
				}
				QFile::copy(lyric, lyricTarget);
			}
		}
		if (isFull(sourceFile, target)) {
			QFile::remove(lyricTarget);
			addToErrorList(list.at(0), FileErrorType::DISKFULL);
			continue;
		}
		QFile::copy(sourceFile, targetFile);
		emit current(fileList.indexOf(file));
	}
	emit finished();
	emit copyFinished(OperationType::COPY);
}

/*
 * @brief Show operation result
 * @param Operation type
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
			getMusic(PathType::LOCAL, 1);
			getMusic(PathType::REMOTE, 1);
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
		getMusic(PathType::LOCAL, 1);
		getMusic(PathType::REMOTE, 1);
	}
}

/*
 * @brief Set now playing title
 * @param File name
 */
void MusicSyncTool::setNowPlayingTitle(const QString& file) const { ui.nowPlaying->setText(tr("正在播放：") + file); }
/*
 * @brief Get language
 * @return Language
 */
[[nodiscard]]
QString MusicSyncTool::getLanguage() const { return entity.language; }

/*
 * @brief Slots for remote open action
 */
void MusicSyncTool::on_actionRemote_triggered(bool triggered) {
	openFolder(PathType::REMOTE);
	if (remotePath == "") {
		return;
	}
	getMusic(PathType::REMOTE, 1);
	setAvailableSpace(PathType::REMOTE);
	showOperationResult(OperationType::LOAD);
}

/*
 * @brief Slots for local open action
 */
void MusicSyncTool::on_actionLocal_triggered(bool triggered) {
	openFolder(PathType::LOCAL);
	if (localPath == "") {
		return;
	}
	getMusic(PathType::LOCAL, 1);
	setAvailableSpace(PathType::LOCAL);
	showOperationResult(OperationType::LOAD);
}

/*
 * @brief Slots for settings action
 */
void MusicSyncTool::on_actionSettings_triggered(bool triggered) const { showSettings(); }
/*
 * @brief Slots for about action
 */
void MusicSyncTool::on_actionAbout_triggered(bool triggered) {
	AboutPage about;
	about.exec();
}

/*
 * @brief Slots for copy to remote action
 */
void MusicSyncTool::on_copyToRemote_clicked() {
	if (localPath == "") {
		popError(PET::NPS);
		return;
	}
	QStringList fileList = getSelectedMusic(PathType::LOCAL);
	if (fileList.isEmpty()) {
		popError(PET::NFS);
		return;
	}
	QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, localPath, fileList, remotePath);
}

/*
 * @brief Slots for copy to local action
 */
void MusicSyncTool::on_copyToLocal_clicked() {
	if (remotePath == "") {
		popError(PET::NPS);
		return;
	}
	QStringList fileList = getSelectedMusic(PathType::REMOTE);
	if (fileList.isEmpty()) {
		popError(PET::NFS);
		return;
	}
	QFuture<void> future = QtConcurrent::run(&MusicSyncTool::copyMusic, this, remotePath, fileList, localPath);
}

/*
 * @brief Slots for duplicated music scanning(local)
 */
void MusicSyncTool::on_actionDupeLocal_triggered(bool triggered) { getDuplicatedMusic(PathType::LOCAL); }
/*
 * @brief Slots for duplicated music scanning(remote)
 */
void MusicSyncTool::on_actionDupeRemote_triggered(bool triggered) { getDuplicatedMusic(PathType::REMOTE); }
/*
 * @brief Slots for refresh local action
 */
void MusicSyncTool::on_refreshLocal_clicked() { getMusic(PathType::LOCAL, 1); }
/*
 * @brief Slots for refresh remote action
 */
void MusicSyncTool::on_refreshRemote_clicked() { getMusic(PathType::REMOTE, 1); }
/*
 * @brief Slots for search local action
 */
void MusicSyncTool::on_searchLocal_returnPressed() { searchMusic(PathType::LOCAL, ui.searchLocal->text()); }
/*
 * @brief Slots for search remote action
 */
void MusicSyncTool::on_searchRemote_returnPressed() { searchMusic(PathType::REMOTE, ui.searchRemote->text()); }
/*
 * @brief Slots for preview(local)
 */
void MusicSyncTool::on_tableWidgetLocal_cellDoubleClicked(const int row, int column) {
	setTotalLength(PathType::LOCAL, row);
}

/*
 * @brief Slots for preview(remote)
 */
void MusicSyncTool::on_tableWidgetRemote_cellDoubleClicked(const int row, int column) {
	setTotalLength(PathType::REMOTE, row);
}

/*
 * @brief Slots for exiting the program
 */
void MusicSyncTool::on_actionExit_triggered(bool triggered) { exit(EXIT_SUCCESS); }

/*
 * @brief Pop up window to ask if user want to delete all the log files
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

/*
 * @brief Play or pause the music
 */
void MusicSyncTool::on_playControl_clicked() {
	if (!mediaPlayer->hasAudio()) {
		popError(PET::NOAUDIO);
		return;
	}
	if (mediaPlayer->isPlaying()) {
		mediaPlayer->pause();
		setMediaWidget(PlayState::PAUSED);
	}
	else {
		mediaPlayer->play();
		setMediaWidget(PlayState::PLAYING);
	}
}

/*
 * @brief Set slider position
 */
void MusicSyncTool::setSliderPosition(const qint64 position) const {
	ui.playSlider->setValue(static_cast<int>(position));
	QString progress;
	if (position % 60000 / 1000 < 10) {
		progress = QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000);
	}
	else {
		progress = QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000);
	}
	progress += "/" + QString::number(mediaPlayer->duration() / 60000) + ":";
	if (mediaPlayer->duration() % 60000 / 1000 < 10) {
		progress += "0" + QString::number(mediaPlayer->duration() % 60000 / 1000);
	}
	else {
		progress += QString::number(mediaPlayer->duration() % 60000 / 1000);
	}
	ui.playProgress->setText(progress);
}

/*
 * @brief Slots for play slider
 */
void MusicSyncTool::on_playSlider_sliderMoved(const int position) const {
	mediaPlayer->setPosition(position);
	if (position % 60000 / 1000 < 10) {
		ui.playProgress->setText(QString::number(position / 60000) + ":0" + QString::number(position % 60000 / 1000));
	}
	else {
		ui.playProgress->setText(QString::number(position / 60000) + ":" + QString::number(position % 60000 / 1000));
	}
}

/*
 * @brief Slots for play slider
 */
void MusicSyncTool::on_playSlider_sliderPressed() const { mediaPlayer->setPosition(ui.playSlider->value()); }
/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_sliderPressed() const {
	audioOutput->setVolume(static_cast<float>(ui.volumeSlider->value() / 100.0));
	const QString text = tr("音量：") + QString::number(ui.volumeSlider->value()) + "%";
	ui.volumeLabel->setText(text);
}

/*
 * @brief Slots for favorite button(local)
 */
void MusicSyncTool::on_favoriteOnlyLocal_clicked() { getFavoriteMusic(PathType::LOCAL, 1); }
/*
 * @brief Slots for favorite button(remote)
 */
void MusicSyncTool::on_favoriteOnlyRemote_clicked() { getFavoriteMusic(PathType::REMOTE, 1); }
/*
 * @brief Slots for last page switch(local)
 */
void MusicSyncTool::on_lastPageLocal_clicked() {
	if (localPath == "") {
		popError(PET::NPS);
		return;
	}
	if (currentPage[0] == 1) {
		popError(PET::FIRST);
		return;
	}
	if (favoriteOnly[0]) {
		getFavoriteMusic(PathType::LOCAL, --currentPage[0]);
	}
	else {
		getMusic(PathType::LOCAL, --currentPage[0]);
	}
}

/*
 * @brief Slots for next page switch(local)
 */
void MusicSyncTool::on_nextPageLocal_clicked() {
	if (localPath == "") {
		popError(PET::NPS);
		return;
	}
	if (currentPage[0] == totalPage[0]) {
		popError(PET::LAST);
		return;
	}
	if (favoriteOnly[0]) {
		getFavoriteMusic(PathType::LOCAL, ++currentPage[0]);
	}
	else {
		getMusic(PathType::LOCAL, ++currentPage[0]);
	}
}

/*
 * @brief Slots for last page switch(remote)
 */
void MusicSyncTool::on_lastPageRemote_clicked() {
	if (remotePath == "") {
		popError(PET::NPS);
		return;
	}
	if (currentPage[1] == 1) {
		popError(PET::FIRST);
		return;
	}
	if (favoriteOnly[1]) {
		getFavoriteMusic(PathType::REMOTE, --currentPage[1]);
	}
	else {
		getMusic(PathType::REMOTE, --currentPage[1]);
	}
}

/*
 * @brief Slots for next page switch(remote)
 */
void MusicSyncTool::on_nextPageRemote_clicked() {
	if (remotePath == "") {
		popError(PET::NPS);
		return;
	}
	if (currentPage[1] == totalPage[1]) {
		popError(PET::LAST);
		return;
	}
	if (favoriteOnly[1]) {
		getFavoriteMusic(PathType::REMOTE, ++currentPage[1]);
	}
	else {
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
	audioOutput->setVolume(static_cast<float>(position / 100.0));
	const QString text = tr("音量：") + QString::number(position) + "%";
	ui.volumeLabel->setText(text);
}

/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_valueChanged(const int position) const { on_volumeSlider_sliderMoved(position); }
/*
 * @brief Set total length of play slider
 */
void MusicSyncTool::setTotalLength(const PathType path, const int row) {
	const QTableWidget& widget = path == PathType::LOCAL ? *ui.tableWidgetLocal : *ui.tableWidgetRemote;
	// QString sql = "SELECT fileName FROM musicInfo WHERE";
	// sql += " title = \"" + widget.item(row, 0)->text() + "\" AND artist = \"" + widget.item(row, 1)->text() +
	// 	"\" AND album = \"" + widget.item(row, 2)->text() + "\"";
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
	query.prepare("SELECT fileName FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album");
	query.bindValue(":title", widget.item(row, 0)->text());
	query.bindValue(":artist", widget.item(row, 1)->text());
	query.bindValue(":album", widget.item(row, 2)->text());
	query.exec();
	query.next();
	nowPlaying = query.value(0).toString();
	const QString filePath = (path == PathType::LOCAL ? localPath : remotePath) + "/" + nowPlaying;
	mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
	TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
	f = TagLib::FileRef(filePath.toStdWString().c_str());
#else
	f = TagLib::FileRef(filePath.toStdString().c_str());
#endif
	if (!f.isNull() && f.audioProperties()) {
		const qint64 length = f.audioProperties()->lengthInMilliseconds();
		setNowPlayingTitle(nowPlaying);
		mediaPlayer->setPlaybackRate(1.0);
		mediaPlayer->setPosition(0);
		mediaPlayer->play();
		setMediaWidget(PlayState::PLAYING);
		ui.playSlider->setMaximum(static_cast<int>(length));
		ui.playSlider->setValue(0);
		ui.playProgress->setText("00:00");
	}
}

/*
 * @brief Get favorite music files
 * @param Path type(local or remote)
 * @param Page number
 */
void MusicSyncTool::getFavoriteMusic(const PathType path, unsigned short page) {
	if (path == PathType::LOCAL && localPath == "") {
		popError(PET::NPS);
		return;
	}
	if (path == PathType::REMOTE && remotePath == "") {
		popError(PET::NPS);
		return;
	}
	if (entity.favoriteTag == "") {
		popError(PET::NFT);
		return;
	}
	emit started();
	QSqlQuery& query = path == PathType::LOCAL ? queryLocal : queryRemote;
	QString sql = "SELECT COUNT(*) FROM musicInfo WHERE favorite = 1";
	query.exec(sql);
	query.next();
	const int totalSize = query.value(0).toInt();
	totalPage[(path == PathType::LOCAL ? 0 : 1)] = static_cast<short>(totalSize / PAGESIZE) + 1;
	// NOLINT(cppcoreguidelines-narrowing-conversions)
	sql = "SELECT * FROM musicInfo WHERE favorite = 1";
	switch (entity.sortBy) {
	case TITLE:
		sql += " ORDER BY title";
		break;
	case ARTIST:
		sql += " ORDER BY artist";
		break;
	case ALBUM:
		sql += " ORDER BY album";
		break;
	default:
		sql += " ORDER BY title";
		break;
	}
	switch (entity.orderBy) {
	case ASC:
		sql += " ASC";
		break;
	case DESC:
		sql += " DESC";
		break;
	default:
		sql += " ASC";
		break;
	}
	sql += " LIMIT " + QString::number(PAGESIZE) + " OFFSET " +
		QString::number((currentPage[(path == PathType::LOCAL ? 0 : 1)] - 1) * PAGESIZE);
	query.exec(sql);
	QTableWidget* targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
	(path == PathType::LOCAL ? ui.pageLocal : ui.pageRemote)
		->setText(QString::number(currentPage[0]) + "/" + QString::number(totalPage[0]));
	targetTable->clearContents();
	targetTable->setRowCount(PAGESIZE);
	favoriteOnly[(path == PathType::LOCAL ? 0 : 1)] = true;
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

/*
 * @brief Connect slots for application
 */
void MusicSyncTool::connectSlots() const {
	connect(this, &MusicSyncTool::total, loading, &LoadingPage::setTotal);
	connect(this, &MusicSyncTool::current, loading, &LoadingPage::setProgress);
	connect(this, &MusicSyncTool::started, loading, &LoadingPage::showPage);
	connect(this, &MusicSyncTool::finished, loading, &LoadingPage::stopPage);
	connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::showOperationResult);
	connect(this, &MusicSyncTool::loadFinished, this, &MusicSyncTool::showOperationResult);
	connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MusicSyncTool::setSliderPosition);
	connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &MusicSyncTool::endMedia);
	connect(this, &MusicSyncTool::addToErrorListConcurrent, this,
	        QOverload<const QString&, LoadErrorType>::of(&MusicSyncTool::addToErrorList));
}

/*
 * @brief Get available space and show it on the top
 * @param Path type(local or remote)
 */
void MusicSyncTool::setAvailableSpace(const PathType path) const {
	const QStorageInfo storage(path == PathType::LOCAL ? localPath : remotePath);
	const QString textBuilder = tr("可用空间：") + QString::number(
			static_cast<double>(storage.bytesAvailable()) / 1024.0 / 1024.0 / 1024.0, 10, 2) + "GB"
		+ " / " + QString::number(static_cast<double>(storage.bytesTotal()) / 1024.0 / 1024.0 / 1024.0, 10, 2)
		+ "GB";
	(path == PathType::LOCAL ? ui.availableSpaceLocal : ui.availableSpaceRemote)->setText(textBuilder);
}

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

/*
 * @brief Check if disk is full
 * @param File path
 * @param Target path
 * @return True if disk is full
 */
bool MusicSyncTool::isFull(const QString& filePath, const QString& target) {
	const QFileInfo musicInfo(filePath);
	const QDir targetInfo(target);
	if (const QStorageInfo storage(targetInfo); musicInfo.size() > storage.bytesAvailable()) {
		return true;
	}
	return false;
}

bool MusicSyncTool::isRuleHit(const LyricIgnoreRuleSingleton& singleton, const TagLib::Tag* target) {
	const QRegularExpression regExp(singleton.getRuleName());
	switch (singleton.getRuleField()) {
	case RuleField::TITLE:
		if (singleton.getRuleType() == RuleType::INCLUDES) {
			return regExp.match(QString::fromStdString(target->title().to8Bit(true))).hasMatch();
		}
		return !regExp.match(QString::fromStdString(target->title().to8Bit(true))).hasMatch();
	case RuleField::ARTIST:
		if (singleton.getRuleType() == RuleType::INCLUDES) {
			return regExp.match(QString::fromStdString(target->artist().to8Bit(true))).hasMatch();
		}
		return !regExp.match(QString::fromStdString(target->artist().to8Bit(true))).hasMatch();
	case RuleField::ALBUM:
		if (singleton.getRuleType() == RuleType::INCLUDES) {
			return regExp.match(QString::fromStdString(target->album().to8Bit(true))).hasMatch();
		}
		return !regExp.match(QString::fromStdString(target->album().to8Bit(true))).hasMatch();
	}
	return false;
}

bool MusicSyncTool::isFormatSupported(const QString& fileName) const {
	return std::any_of(supportedFormat.begin(), supportedFormat.end(), [&fileName](const QString& format) {
		return fileName.contains(format);
	});
}
