// ReSharper disable CppClangTidyConcurrencyMtUnsafe
// #pragma warning(disable : 6031)
#include "MusicSyncTool.h"

#include <algorithm>
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "Logger.h"
#if defined(__linux)
#include <unistd.h>
#endif

/*
 * @brief Default constructor
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

/*
 * @brief Destructor
 */
MusicSyncTool::~MusicSyncTool() {
	if (local.isOpen()) {
		local.closeDB();
	}
	if (remote.isOpen()) {
		remote.closeDB();
	}
	if (dbLocal.isOpen()) {
		dbLocal.close();
	}
	if (dbRemote.isOpen()) {
		dbRemote.close();
	}
	delete loading;
}

/*
 * @brief Initialize database
 */
void MusicSyncTool::initDatabase() {
	local.setConnectionName("local");
	remote.setConnectionName("remote");
}

/*
 * @brief Load settings from settings file
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
void MusicSyncTool::initMediaPlayer() const { player->setVolume(0.5); }

/*
 * @brief Load language file by settings
 */
void MusicSyncTool::loadLanguage() {
	QFile file(QApplication::applicationDirPath() + "/translations/langinfo.json");
	if (!file.open(QIODevice::ReadOnly)) {
		popError(PET::NOLANG);
		Logger::Fatal("Error loading language info:" + file.errorString());
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
	case PET::NOLANG:
		QMessageBox::critical(this, tr("错误"), tr("找不到程序语言配置文件，程序即将退出！"));
		break;
	case PET::DBERROR:
		QMessageBox::critical(this, tr("错误"), tr("操作数据库中数据时出现严重错误，程序即将退出！"));
		break;
	case PET::DOF:
		QMessageBox::critical(this, tr("错误"), tr("打开或创建目录失败，请检查程序是否有相关权限！"));
	}
}

/*
 * @brief Set up media player
 * @param Play state
 */
void MusicSyncTool::setMediaWidget(const PlayState state) const {
	if (state == PlayState::PLAYING) {
		ui.playControl->setText(tr("暂停"));
		setNowPlayingTitle(player->getNowPlaying());
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
		local.setPath(dir);
		if (!local.openDB()) {
			exit(EXIT_FAILURE);
		}
	}
	else {
		remote.setPath(dir);
		if (!remote.openDB()) {
			exit(EXIT_FAILURE);
		}
	}
}

/*
 * @brief Get music files from selected path(multi-thread caller)
 * @param Path type
 * @param Page number
 */
void MusicSyncTool::getMusic(PathType path, unsigned short page) {
	Logger::Info("Scanning started");
	if (localPath == "" && remotePath == "") {
		Logger::Warn("No path selected");
		return;
	}
	QFuture<void> future = QtConcurrent::run(&MusicSyncTool::getMusicConcurrent, this, path, page);
}

/*
 * @brief Get music files from selected path and load it to tables(main operation, concurrent)
 * @param Path type
 * @param Page number
 */
void MusicSyncTool::getMusicConcurrent(const PathType path, const unsigned short page) {
	const clock_t start = clock();
	MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
	favoriteOnly[path == PathType::LOCAL ? 0 : 1] = false;
	QStringList pathForLog = ds.getPath().split("/");
	QString logFile = "lastScan";
	if (!pathForLog[0].isEmpty()) {
		pathForLog[0].remove(":");
	}
	else {
		pathForLog.removeFirst();
	}
	for (const QString& tempStr : pathForLog) {
		logFile += " - " + tempStr;
	}
	logFile += ".log";
	logFile = QCoreApplication::applicationDirPath() + "/log/" + logFile;
	QDateTime dateTime = getDateFromLog(logFile);
	const QDir dir(ds.getPath());
	ds.initTable();
	QStringList filters;
	for (const QString& format : supportedFormat) {
		filters.append("*." + format);
	}
	QStringList newFileList = dir.entryList(filters, QDir::Files);
	QList<QueryItem> tempList = ds.getAll({QueryRows::FILENAME}, ASC, TITLE);
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
		if (oldFileList.contains(newFileList.at(i)) || !isFormatSupported(newFileList.at(i))) {
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
	QStringList errList = ds.addMusic(newFileList);
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
	}
	else {
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
	Logger::Info(
		"Scanning finished in" + QString::number(static_cast<double>(end - start) / CLOCKS_PER_SEC) + "seconds");
	(path == PathType::LOCAL ? ui.pageLocal : ui.pageRemote)
		->setText(QString::number(currentPage[(path == PathType::LOCAL ? 0 : 1)]) + "/" +
			QString::number(totalPage[(path == PathType::LOCAL ? 0 : 1)]));
	dateTime = QDateTime::currentDateTime();
	writeLog(logFile, dateTime);
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
	auto& ds = path == PathType::LOCAL ? local : remote;
	const auto [fst, snd] = ds.searchMusic(text);
	const auto& targetTable = path == PathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote;
	targetTable->clearContents();
	targetTable->setRowCount(fst);
	int row = 0;
	for (const auto& item : snd) {
		targetTable->setItem(row, 0, new QTableWidgetItem(item.getTitle()));
		targetTable->setItem(row, 1, new QTableWidgetItem(item.getArtist()));
		targetTable->setItem(row, 2, new QTableWidgetItem(item.getAlbum()));
		targetTable->setItem(row, 3, new QTableWidgetItem(item.getGenre()));
		targetTable->setItem(row, 4, new QTableWidgetItem(QString::number(item.getYear())));
		targetTable->setItem(row, 5, new QTableWidgetItem(QString::number(item.getTrack())));
		row++;
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
	auto& ds = path == PathType::LOCAL ? local : remote;
	if (ds.getPath().isEmpty()) {
		Logger::Warn("No path selected");
		return {};
	}
	constexpr auto sortKey = TITLE;
	auto result = ds.getAll({
		                        QueryRows::TITLE, QueryRows::ARTIST,
		                        QueryRows::ALBUM, QueryRows::YEAR,
		                        QueryRows::FILENAME
	                        }, ASC, sortKey);

	ShowDupe dp;
	QStringList dupeList;
	for (int i = 0; i < result.size() - 1; i++) {
		auto& slow = result[i];
		auto& fast = result[i + 1];
		if (slow == fast) {
			dupeList.append(slow.getFileName());
			dp.add(slow.getFileName());
			dupeList.append(fast.getFileName());
			dp.add(fast.getFileName());
		}
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
	auto& ds = path == PathType::LOCAL ? local : remote;
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
	auto fileList = QStringList();
	for (const int& i : selectedRows) {
		QString title = table->item(i, 0)->text();
		QString artist = table->item(i, 1)->text();
		QString album = table->item(i, 2)->text();
		auto item = QueryItem();
		item.setTitle(title);
		item.setArtist(artist);
		item.setAlbum(album);
		item.setFileName(ds.getFileNameByMD(item));
		fileList.append(item.getFileName());
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

void MusicSyncTool::setFavorite(const PathType path, const TagLib::String& key,
                                const QDateTime& dateTime) {
	MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
	if (entity.favoriteTag != "") {
		ds.setFavorite(key, dateTime);
	}
}

void MusicSyncTool::setRuleHit(const PathType path, const QList<LyricIgnoreRule>& rules,
                               const QDateTime& dateTime) {
	MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
	if (!rules.isEmpty()) {
		ds.setRuleHit(rules, dateTime);
	}
}

QDateTime MusicSyncTool::getDateFromLog(const QString& log) {
	QFile file(log);
	QDateTime dateTime;
	const QString logDir = QCoreApplication::applicationDirPath() + "/log";
	if (const QDir dir(logDir); !dir.exists()) {
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
	}
	else {
		Logger::Warn("Failed to open last scan log:" + file.errorString());
		Logger::Warn("No last scan log found, scanning all files");
		// Set last scan time to 1970-01-01 00:00:00 if no log found
		dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
	}
	return dateTime;
}

void MusicSyncTool::writeLog(const QString& log, const QDateTime& dateTime) {
	QFile file(log);
	QTextStream out(&file);
	if (file.open(QIODevice::WriteOnly)) {
		out << dateTime.toString();
	}
	file.close();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MusicSyncTool::rollBackCopy(const QString& fileName) {
	const auto list = fileName.split(".");
	const auto lyric = list.at(0) + ".lrc";
	if (QFile::exists(lyric)) {
		QFile::remove(lyric);
	}
	if (QFile::exists(fileName)) {
		QFile::remove(fileName);
	}
}

/*
 * @brief Save settings to settings file
 * @param Entity
 */
void MusicSyncTool::saveSettings(const set& entityParam) {
	QFile file("settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		Logger::Fatal("Failed to open settings file:" + file.errorString());
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
	Logger::Info(QString("IgnoreLyric:") + QString::number(this->entity.ignoreLyric));
	Logger::Info("SortBy:" + QString::number(this->entity.sortBy));
	Logger::Info("OrderBy:" + QString::number(this->entity.orderBy));
	Logger::Info("Language:" + this->entity.language);
	Logger::Info("FavoriteTag:" + this->entity.favoriteTag);
	int i = 1;
	for (const auto& rule : entityParam.rules) {
		Logger::Info("Rule No." + QString::number(i++) + ":");
		Logger::Info("RuleTypeConverter:" + rule.getRuleTypeStr());
		Logger::Info("RuleFieldConverter:" + rule.getRuleFieldStr());
		Logger::Info("Rule:" + rule.getRuleName());
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
			Logger::Fatal("Failed to create target directory");
			popError(PET::DOF);
			return;
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
		for (const QString& format : supportedFormat) {
			if (sourceFile.contains(format)) {
				QString temp = sourceFile;
				QString tempTarget = targetFile;
				lyric = temp.replace(format, "lrc");
				lyricTarget = tempTarget.replace(format, "lrc");
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
				diskFull = isFull(sourceFile, target);
				if (diskFull) {
					rollBackCopy(targetFile);
					addToErrorList(list.at(0), FileErrorType::DISKFULL);
					continue;
				}
				QFile::copy(lyric, lyricTarget);
			}
		}
		diskFull = isFull(sourceFile, target);
		if (diskFull) {
			rollBackCopy(targetFile);
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
			if (favoriteOnly[0]) {
				getFavoriteMusic(PathType::LOCAL, currentPage[0]);
			}
			else {
				getMusic(PathType::LOCAL, currentPage[0]);
			}
			if (favoriteOnly[1]) {
				getFavoriteMusic(PathType::REMOTE, currentPage[1]);
			}
			else {
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
		}
		else {
			getMusic(PathType::LOCAL, currentPage[0]);
		}
		if (favoriteOnly[1]) {
			getFavoriteMusic(PathType::REMOTE, currentPage[1]);
		}
		else {
			getMusic(PathType::REMOTE, currentPage[1]);
		}
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
QString MusicSyncTool::getLanguage() const {
	return entity.language;
}

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
// ReSharper disable once CppMemberFunctionMayBeStatic
void MusicSyncTool::on_actionAbout_triggered(bool triggered) {
	// NOLINT(*-convert-member-functions-to-static)
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
// ReSharper disable once CppMemberFunctionMayBeStatic
void MusicSyncTool::on_actionExit_triggered(bool triggered) {
	exit(EXIT_SUCCESS);
} // NOLINT(*-convert-member-functions-to-static)

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
	if (player->getNowPlaying().isEmpty()) {
		popError(PET::NOAUDIO);
		return;
	}
	if (player->isPlaying()) {
		player->pause();
		setMediaWidget(PlayState::PAUSED);
	}
	else {
		player->play();
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
	progress += "/" + QString::number(player->getDuration() / 60000) + ":";
	if (player->getDuration() % 60000 / 1000 < 10) {
		progress += "0" + QString::number(player->getDuration() % 60000 / 1000);
	}
	else {
		progress += QString::number(player->getDuration() % 60000 / 1000);
	}
	ui.playProgress->setText(progress);
}

/*
 * @brief Slots for play slider
 */
void MusicSyncTool::on_playSlider_sliderMoved(const int position) const {
	player->setPosition(position);
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
void MusicSyncTool::on_playSlider_sliderPressed() const { player->setPosition(ui.playSlider->value()); }
/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_sliderPressed() const {
	player->setVolume(static_cast<float>(ui.volumeSlider->value() / 100.0));
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
	player->setVolume(static_cast<float>(position / 100.0));
	const QString text = tr("音量：") + QString::number(position) + "%";
	ui.volumeLabel->setText(text);
}

/*
 * @brief Slots for volume slider
 */
void MusicSyncTool::on_volumeSlider_valueChanged(const int position) const { on_volumeSlider_sliderMoved(position); }

void MusicSyncTool::on_copyFinished(OperationType op) const {
	if (localPath != "") {
		setAvailableSpace(PathType::LOCAL);
	}
	if (remotePath != "") {
		setAvailableSpace(PathType::REMOTE);
	}
}

/*
 * @brief Set total length of play slider
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
	nowPlaying = ds.getFileNameByMD(*item);
	const QString filePath = (path == PathType::LOCAL ? localPath : remotePath) + "/" + nowPlaying;
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
	const int lastPageSize = totalSize % PAGESIZE;
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
		->setText(QString::number(currentPage[(path == PathType::LOCAL ? 0 : 1)]) + "/" +
			QString::number(totalPage[(path == PathType::LOCAL ? 0 : 1)]));
	targetTable->clearContents();
	if (currentPage[(path == PathType::LOCAL ? 0 : 1)] == totalPage[(path == PathType::LOCAL ? 0 : 1)]) {
		targetTable->setRowCount(lastPageSize);
	}
	else {
		targetTable->setRowCount(PAGESIZE);
	}
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
	connect(&local, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&remote, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&local, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(&remote, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(&local, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
	connect(&remote, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
	connect(&local, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
	connect(&remote, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
	connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::showOperationResult);
	connect(this, &MusicSyncTool::loadFinished, this, &MusicSyncTool::showOperationResult);
	connect(player->getMediaPlayer(), &QMediaPlayer::positionChanged, this, &MusicSyncTool::setSliderPosition);
	connect(player->getMediaPlayer(), &QMediaPlayer::playbackStateChanged, this, &MusicSyncTool::endMedia);
	connect(this, &MusicSyncTool::addToErrorListConcurrent, this,
	        QOverload<const QString&, LoadErrorType>::of(&MusicSyncTool::addToErrorList));
	connect(this, &MusicSyncTool::copyFinished, this, &MusicSyncTool::on_copyFinished);
}

/*
 * @brief Get available space and show it on the top
 * @param Path type(local or remote)
 */
void MusicSyncTool::setAvailableSpace(const PathType path) const {
	const QStorageInfo storage(path == PathType::LOCAL ? localPath : remotePath);
	const QString textBuilder = tr("可用空间：") +
		QString::number(static_cast<double>(storage.bytesAvailable()) / 1024.0 / 1024.0 / 1024.0, 10, 2) + "GB" +
		" / " + QString::number(static_cast<double>(storage.bytesTotal()) / 1024.0 / 1024.0 / 1024.0, 10, 2) + "GB";
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

bool MusicSyncTool::isRuleHit(const LyricIgnoreRule& singleton, const TagLib::Tag* target) {
	QString fieldStr;
	switch (singleton.getRuleField()) {
	case RuleField::TITLE:
		fieldStr = QString::fromStdString(target->title().to8Bit(true));
		break;
	case RuleField::ARTIST:
		fieldStr = QString::fromStdString(target->artist().to8Bit(true));
		break;
	case RuleField::ALBUM:
		fieldStr = QString::fromStdString(target->album().to8Bit(true));
		break;
	}
	const QRegularExpression regExp(singleton.getRuleName());
	const bool hasMatch = regExp.match(fieldStr).hasMatch();
	return (singleton.getRuleType() == RuleType::INCLUDES) ? hasMatch : !hasMatch;
}

bool MusicSyncTool::isFormatSupported(const QString& fileName) const {
	return std::any_of(supportedFormat.begin(), supportedFormat.end(), [&fileName](const QString& format) {
		const QString extension = fileName.section('.', -1);
		return extension.compare(format, Qt::CaseInsensitive) == 0;
	});
}
