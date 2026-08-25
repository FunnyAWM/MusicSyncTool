// ReSharper disable CppClangTidyConcurrencyMtUnsafe
#pragma warning(disable : 6031)
#include "MSTMainWindow.h"
#include <iostream>
#include <QJsonArray>
#include <taglib/tag.h>
#include "MSTErrorReporter.h"
#include "MSTMediaController.h"
#include "MSTTableManager.h"
#include "../../Services/Logger.h"
#include "../../Services/MSTScanController.h"
#include "../../Services/MSTSettingsManager.h"

#if defined(__linux)
#include <unistd.h>
#endif

MSTMainWindow::MSTMainWindow(QWidget* parent) : // NOLINT(*-pro-type-member-init)
	QMainWindow(parent), translator(new QTranslator(this)), loading(new LoadingPage()),
	player(new MSTMediaPlayer(this)) {
	initDatabase();
	loadSettings();
	initUI();

	// 初始化表格管理器
	tableManager = new MSTTableManager(
		ui.tableWidgetLocal, ui.tableWidgetRemote,
		ui.pageLocal, ui.pageRemote,
		local, remote, this);

	// 初始化扫描控制器
	scanController = new MSTScanController(local, remote, tableManager, this);

	// 初始化错误报告器
	errorReporter = new MSTErrorReporter(this, tableManager, this);

	// 初始化媒体播放控制器
	mediaController = new MSTMediaController(
		player.get(),
		ui.playControl, ui.playSlider, ui.playProgress,
		ui.volumeSlider, ui.volumeLabel, ui.nowPlaying,
		ui.tableWidgetLocal, ui.tableWidgetRemote,
		local, remote, this);

	loadLanguage();
	mediaController->initMediaPlayer();
	connectSlots();
	mediaController->setMediaWidget(PlayState::STOPPED);
}

MSTMainWindow::~MSTMainWindow() {
	if (local.isOpen()) {
		local.closeDB();
	}
	if (remote.isOpen()) {
		remote.closeDB();
	}
	delete copyOperationInProgress;
	delete scanController;
	delete mediaController;
	delete errorReporter;
	player.reset();
	delete loading;
}

void MSTMainWindow::initDatabase() {
	local.setConnectionName("local");
	remote.setConnectionName("remote");
}

void MSTMainWindow::initUI() {
	ui.setupUi(this);
	this->setWindowIcon(QIcon(":/MSTMainWindow.ico"));
	ui.tableWidgetLocal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetRemote->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.volumeSlider->blockSignals(true);
	ui.volumeSlider->setValue(50);
	ui.volumeSlider->blockSignals(false);
	ui.volumeLabel->setText(tr("音量：") + "50%");
	ui.nowPlaying->setText(tr("播放已结束。"));
}

void MSTMainWindow::loadSettings() {
	MSTSettingsManager::loadSettings(entity);
}

void MSTMainWindow::loadLanguage() {
	QFile file(QApplication::applicationDirPath() + "/translations/langinfo.json");
	if (!file.open(QIODevice::ReadOnly)) {
		errorReporter->popError(AppErrorType::NO_LANGUAGE);
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
			if (!translator->load("translations/" + langObj["fileName"].toString())) {
				Logger::Warn("Failed to load translation: " + langObj["fileName"].toString());
			}
			break;
		}
	}
	qApp->installTranslator(translator);
	ui.retranslateUi(this);
}

void MSTMainWindow::connectSlots() {
	connect(&local, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&remote, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&local, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(&remote, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(this, &MSTMainWindow::copyFinished, errorReporter, &MSTErrorReporter::showOperationResult);
	connect(&local, QOverload<OperationType>::of(&MSTDataSource::loadFinished), errorReporter,
	        &MSTErrorReporter::showOperationResult);
	connect(&remote, QOverload<OperationType>::of(&MSTDataSource::loadFinished), errorReporter,
	        &MSTErrorReporter::showOperationResult);
	connect(this, &MSTMainWindow::addToErrorListConcurrent, errorReporter,
	        QOverload<const QString&, LoadErrorType>::of(&MSTErrorReporter::addToErrorList));
	connect(this, &MSTMainWindow::copyFinished, this, &MSTMainWindow::on_copyFinished);
	connect(tableManager, &MSTTableManager::errorOccurred, errorReporter, &MSTErrorReporter::popError);
	connect(tableManager, &MSTTableManager::requestLoadMusic, this, &MSTMainWindow::getMusic);
	connect(tableManager, &MSTTableManager::requestLoadFavorite, this, [this](PathType path, unsigned short page) {
		tableManager->getFavoriteMusic(path, page, entity.favoriteTag, toSortBy(entity.sortBy),
		                               toOrderBy(entity.orderBy));
	});
	connect(scanController, &MSTScanController::scanTotal, this, &MSTMainWindow::total);
	connect(scanController, &MSTScanController::scanCurrent, this, &MSTMainWindow::current);
	connect(scanController, &MSTScanController::errorOccurred, errorReporter, &MSTErrorReporter::popError);
	connect(scanController, &MSTScanController::loadErrorOccurred, errorReporter,
	        QOverload<const QString&, LoadErrorType>::of(&MSTErrorReporter::addToErrorList));
	// 扫描控制器加载页面管理
	connect(scanController, &MSTScanController::scanStart, loading, &LoadingPage::showPage);
	connect(scanController, &MSTScanController::scanFinished, loading, &LoadingPage::stopPage);
	connect(scanController, &MSTScanController::scanTotal, loading, &LoadingPage::setTotal);
	connect(scanController, &MSTScanController::scanCurrent, loading, &LoadingPage::setProgress);
	connect(errorReporter, &MSTErrorReporter::requestRefreshMusic, this, [this](PathType path, unsigned short page) {
		if (tableManager->isFavoriteOnly(path)) {
			tableManager->getFavoriteMusic(path, page, entity.favoriteTag, toSortBy(entity.sortBy),
			                               toOrderBy(entity.orderBy));
		}
		else {
			scanController->resetScan(path);
			getMusic(path, page);
		}
	});
	mediaController->connectSignals();
	connect(mediaController, &MSTMediaController::errorOccurred, errorReporter, &MSTErrorReporter::popError);
}

void MSTMainWindow::openFolder(const PathType path) {
	const QString dir = QFileDialog::getExistingDirectory();
	if (dir == "") {
		return;
	}
	if (path == PathType::LOCAL) {
		local.setPath(dir);
		localManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
		if (!local.openDB()) {
			exit(EXIT_FAILURE);
		}
	}
	else {
		remote.setPath(dir);
		remoteManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
		if (!remote.openDB()) {
			exit(EXIT_FAILURE);
		}
	}
}

void MSTMainWindow::getMusic(const PathType path, const unsigned short page) const {
	scanController->startScan(path, page, entity);
}

void MSTMainWindow::popError(const AppErrorType type) const {
	errorReporter->popError(type);
}

QStringList MSTMainWindow::getDuplicatedMusic(const PathType path) {
	const QString selectedPath = path == PathType::LOCAL ? local.getPath() : remote.getPath();
	if (selectedPath == "") {
		Logger::Warn("No path selected");
		errorReporter->popError(AppErrorType::NO_PATH);
		return {};
	}
	ShowDupe duplicateDialog;
	MSTDataSource& ds = path == PathType::LOCAL ? local : remote;
	QList<QueryItem> items = ds.getAll();
	const QueryItem* slow = &items.first();
	QStringList dupeList;
	for (const auto& fast : items) {
		if (slow != nullptr && *slow == fast) {
			dupeList.append(slow->getFileName());
			dupeList.append(fast.getFileName());
		}
		slow = &fast;
	}
	for (const auto& i : dupeList) {
		Logger::Info(
			"Found duplicated music named " + i + " at " + (
				path == PathType::LOCAL ? local.getPath() : remote.getPath()));
		duplicateDialog.add(i);
	}
	duplicateDialog.exec();
	return dupeList;
}

void MSTMainWindow::showSettings() const {
	const auto page = new Settings(entity);
	connect(page, SIGNAL(confirmPressed(SettingsData)), this, SLOT(saveSettings(SettingsData)));
	page->show();
}

void MSTMainWindow::saveSettings(const SettingsData& entityParam) {
	if (!MSTSettingsManager::saveSettings(entityParam)) {
		Logger::Fatal("Error opening settings file");
		QMessageBox::critical(this, tr("错误"), tr("无法打开设置文件"));
		return;
	}
	const int previousSortBy = this->entity.sortBy;
	const int previousOrderBy = this->entity.orderBy;
	const QList<LyricIgnoreRule> previousRules = this->entity.rules;
	const QString previousFavoriteTag = this->entity.favoriteTag;
	this->entity = entityParam;
	if (previousSortBy != entityParam.sortBy || previousOrderBy != entityParam.orderBy) {
		if (local.getPath() != "") {
			scanController->resetScan(PathType::LOCAL);
			getMusic(PathType::LOCAL, 1);
		}
		if (remote.getPath() != "") {
			scanController->resetScan(PathType::REMOTE);
			getMusic(PathType::REMOTE, 1);
		}
	}
	if (previousRules != entity.rules || entityParam.favoriteTag != previousFavoriteTag) {
		MSTSettingsManager::cleanLog();
		if (local.getPath() != "") {
			scanController->resetScan(PathType::LOCAL);
			getMusic(PathType::LOCAL, 1);
		}
		if (remote.getPath() != "") {
			scanController->resetScan(PathType::REMOTE);
			getMusic(PathType::REMOTE, 1);
		}
	}
	loadLanguage();
	Logger::Info("IgnoreLyric: " + QString::number(this->entity.ignoreLyric));
	Logger::Info("SortBy: " + QString::number(this->entity.sortBy));
	Logger::Info("OrderBy: " + QString::number(this->entity.orderBy));
	Logger::Info("Language: " + this->entity.language);
	Logger::Info("FavoriteTag: " + this->entity.favoriteTag);
	Logger::Info("Recursive scan: " + QString::fromStdString(this->entity.recursiveScan ? "true" : "false"));
	int i = 1;
	for (const auto& rule : entityParam.rules) {
		Logger::Info("Rule No." + QString::number(i++) + ":");
		Logger::Info("RuleTypeConverter: " + rule.getRuleTypeStr());
		Logger::Info("RuleFieldConverter: " + rule.getRuleFieldStr());
		Logger::Info("Rule: " + rule.getRuleName());
	}
}

void MSTMainWindow::copyMusic(const QString& source, const QStringList& fileList, const QString& target) {
	// 使用MSTFileManager的新方法进行复制，保持原有的信号发射
	MSTFileManager::copyMusicFiles(
		source, fileList, target, entity.ignoreLyric,
		// 进度回调
		[this](const int index) {
			emit current(index);
		},
		// 错误回调
		[this](const QString& fileName, const int errorType) {
			switch (errorType) {
			case 0: // DUPLICATE
				errorReporter->addToErrorList(fileName, FileErrorType::DUPLICATE);
				break;
			case 1: // LNF
				errorReporter->addToErrorList(fileName, FileErrorType::LYRIC_NOT_FOUND);
				break;
			case 2: // DISKFULL
				errorReporter->addToErrorList(fileName, FileErrorType::DISKFULL);
				break;
			}
		},
		// 开始回调
		[this] {
			emit started();
		},
		// 完成回调
		[this] {
			emit finished();
			emit copyFinished(OperationType::COPY);
		},
		// 总数回调
		[this](const int total) {
			emit this->total(total);
		}
	);
}

[[nodiscard]]
QString MSTMainWindow::getLanguage() const {
	return entity.language;
}

void MSTMainWindow::setAvailableSpace(const PathType path) const {
	const shared_ptr<MSTFileManager> manager = path == PathType::LOCAL ? localManager : remoteManager;
	const QStorageInfo storage(path == PathType::LOCAL ? local.getPath() : remote.getPath());
	const QString spaceInfoText = tr("可用空间：") + manager->getSpaceInfo();
	(path == PathType::LOCAL ? ui.availableSpaceLocal : ui.availableSpaceRemote)->setText(spaceInfoText);
}

void MSTMainWindow::on_actionRemote_triggered(bool triggered) {
	openFolder(PathType::REMOTE);
	if (remote.getPath() == "") {
		return;
	}
	scanController->resetScan(PathType::REMOTE);
	getMusic(PathType::REMOTE, 1);
	setAvailableSpace(PathType::REMOTE);
	errorReporter->showOperationResult(OperationType::LOAD);
}

void MSTMainWindow::on_actionLocal_triggered(bool triggered) {
	openFolder(PathType::LOCAL);
	if (local.getPath() == "") {
		return;
	}
	scanController->resetScan(PathType::LOCAL);
	getMusic(PathType::LOCAL, 1);
	setAvailableSpace(PathType::LOCAL);
	errorReporter->showOperationResult(OperationType::LOAD);
}

void MSTMainWindow::on_actionSettings_triggered(bool triggered) const { showSettings(); }
// ReSharper disable once CppMemberFunctionMayBeStatic
void MSTMainWindow::on_actionAbout_triggered(bool triggered) {
	// NOLINT(*-convert-member-functions-to-static)
	AboutPage about;
	about.exec();
}

void MSTMainWindow::on_copyToRemote_clicked() {
	if (local.getPath() == "") {
		errorReporter->popError(AppErrorType::NO_PATH);
		return;
	}
	if (remote.getPath() == "") {
		errorReporter->popError(AppErrorType::NO_DEST_PATH);
		return;
	}
	QStringList fileList = tableManager->getSelectedMusic(PathType::LOCAL);
	if (fileList.isEmpty()) {
		errorReporter->popError(AppErrorType::NO_FILE);
		return;
	}
	for (QString& file : fileList) {
		file = file + ":" + (local.getRuleHit(file) ? "1" : "0"); // 添加规则命中状态
	}
	QFuture<void> future = QtConcurrent::run(&MSTMainWindow::copyMusic, this, local.getPath(), fileList,
	                                         remote.getPath());
}

void MSTMainWindow::on_copyToLocal_clicked() {
	if (remote.getPath() == "") {
		errorReporter->popError(AppErrorType::NO_PATH);
		return;
	}
	if (local.getPath() == "") {
		errorReporter->popError(AppErrorType::NO_DEST_PATH);
		return;
	}
	QStringList fileList = tableManager->getSelectedMusic(PathType::REMOTE);
	if (fileList.isEmpty()) {
		errorReporter->popError(AppErrorType::NO_FILE);
		return;
	}
	for (QString& file : fileList) {
		file = file + ":" + (remote.getRuleHit(file) ? "1" : "0"); // 添加规则命中状态
	}
	QFuture<void> future = QtConcurrent::run(&MSTMainWindow::copyMusic, this, remote.getPath(), fileList,
	                                         local.getPath());
}

void MSTMainWindow::on_actionDupeLocal_triggered(bool triggered) { getDuplicatedMusic(PathType::LOCAL); }
void MSTMainWindow::on_actionDupeRemote_triggered(bool triggered) { getDuplicatedMusic(PathType::REMOTE); }
void MSTMainWindow::on_refreshLocal_clicked() {
	scanController->resetScan(PathType::LOCAL);
	getMusic(PathType::LOCAL, 1);
}

void MSTMainWindow::on_refreshRemote_clicked() {
	scanController->resetScan(PathType::REMOTE);
	getMusic(PathType::REMOTE, 1);
}

void MSTMainWindow::on_searchLocal_returnPressed() const {
	tableManager->searchMusic(PathType::LOCAL, ui.searchLocal->text());
}

void MSTMainWindow::on_searchRemote_returnPressed() const {
	tableManager->searchMusic(PathType::REMOTE, ui.searchRemote->text());
}

void MSTMainWindow::on_tableWidgetLocal_cellDoubleClicked(const int row, int column) const {
	mediaController->playSelectedTrack(PathType::LOCAL, row);
}

void MSTMainWindow::on_tableWidgetRemote_cellDoubleClicked(const int row, int column) const {
	mediaController->playSelectedTrack(PathType::REMOTE, row);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void MSTMainWindow::on_actionExit_triggered(bool triggered) {
	exit(EXIT_SUCCESS);
} // NOLINT(*-convert-member-functions-to-static)

void MSTMainWindow::on_actionClean_log_files_triggered(bool triggered) {
	const QMessageBox::StandardButton reply =
		QMessageBox::warning(this, tr("提示"), tr("确定要清除所有日志文件吗？"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::No) {
		return;
	}
	MSTSettingsManager::cleanLog();
	QMessageBox::information(this, tr("提示"), tr("日志文件已清除"));
}

void MSTMainWindow::on_playControl_clicked() const {
	mediaController->togglePlayPause();
}

void MSTMainWindow::on_playSlider_sliderMoved(const int position) const {
	mediaController->seekToPosition(position);
}

void MSTMainWindow::on_playSlider_sliderPressed() const { mediaController->onPlaySliderPressed(); }

void MSTMainWindow::on_volumeSlider_sliderPressed() const {
	mediaController->onVolumeSliderPressed();
}

void MSTMainWindow::on_favoriteOnlyLocal_clicked() const {
	tableManager->getFavoriteMusic(PathType::LOCAL, 1, entity.favoriteTag, toSortBy(entity.sortBy),
	                               toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_favoriteOnlyRemote_clicked() const {
	tableManager->getFavoriteMusic(PathType::REMOTE, 1, entity.favoriteTag, toSortBy(entity.sortBy),
	                               toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_lastPageLocal_clicked() const {
	tableManager->goToPrevPage(PathType::LOCAL, local.getPath() != "",
	                           entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_nextPageLocal_clicked() const {
	tableManager->goToNextPage(PathType::LOCAL, local.getPath() != "",
	                           entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_lastPageRemote_clicked() const {
	tableManager->goToPrevPage(PathType::REMOTE, remote.getPath() != "",
	                           entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_nextPageRemote_clicked() const {
	tableManager->goToNextPage(PathType::REMOTE, remote.getPath() != "",
	                           entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

void MSTMainWindow::on_volumeSlider_sliderMoved(const int position) const {
	mediaController->setVolumeFromSlider(position);
}

void MSTMainWindow::on_volumeSlider_valueChanged(const int position) const { on_volumeSlider_sliderMoved(position); }

void MSTMainWindow::on_copyFinished(OperationType op) const {
	if (local.getPath() != "") {
		setAvailableSpace(PathType::LOCAL);
	}
	if (remote.getPath() != "") {
		setAvailableSpace(PathType::REMOTE);
	}
}
