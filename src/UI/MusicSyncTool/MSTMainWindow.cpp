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

// LyricIgnoreRule 不再继承 QObject，原先经它传递到本翻译单元的 PROPERTIES 名字改为显式引入
using PROPERTIES::AccentColor;
using PROPERTIES::ThemeMode;
using PROPERTIES::toAccentColor;
using PROPERTIES::toOrderBy;
using PROPERTIES::toSortBy;
using PROPERTIES::toThemeMode;

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
	// 先等待后台任务结束，再销毁它们会访问的对象（表格、错误报告器、数据源）
	copyWatcher.waitForFinished();
	scanController->waitForFinished();
	if (local.isOpen()) {
		local.closeDB();
	}
	if (remote.isOpen()) {
		remote.closeDB();
	}
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
		// 语言配置缺失只影响翻译，提示用户后继续以默认语言运行
		errorReporter->popError(AppErrorType::NO_LANGUAGE);
		Logger::Error("Error opening langinfo.json:" + file.errorString());
		return;
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
			// 翻译文件路径锚定程序目录，避免依赖进程工作目录
			const QString translationPath = QApplication::applicationDirPath() + "/translations/"
				+ langObj["fileName"].toString();
			if (!translator->load(translationPath)) {
				Logger::Warn("Failed to load translation: " + translationPath);
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
	// 扫描结束展示加载错误：此前 loadFinished 无人发射，加载错误只会静默堆积
	connect(scanController, &MSTScanController::scanFinished, this, [this] {
		errorReporter->showOperationResult(OperationType::LOAD);
	});
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
	// 扫描任务在工作线程取数，表格刷新统一回到 GUI 线程
	connect(scanController, &MSTScanController::tablePageReady, this, &MSTMainWindow::applyTablePage);
	// 复制任务的文件错误经排队连接回到 GUI 线程登记，避免跨线程写错误列表
	connect(this, &MSTMainWindow::fileCopyError, errorReporter,
	        QOverload<const QString&, FileErrorType>::of(&MSTErrorReporter::addToErrorList));
	connect(&copyWatcher, &QFutureWatcher<void>::finished, this, [this] { copyOperationInProgress.store(false); });
	// 扫描控制器加载页面管理
	connect(scanController, &MSTScanController::scanStart, loading, &LoadingPage::showPage);
	connect(scanController, &MSTScanController::scanFinished, loading, &LoadingPage::stopPage);
	connect(scanController, &MSTScanController::scanTotal, loading, &LoadingPage::setTotal);
	connect(scanController, &MSTScanController::scanCurrent, loading, &LoadingPage::setProgress);
	// 复制任务的进度与扫描共用加载页，此前这几个回调没有任何接收者
	connect(this, &MSTMainWindow::started, loading, &LoadingPage::showPage);
	connect(this, &MSTMainWindow::total, loading, &LoadingPage::setTotal);
	connect(this, &MSTMainWindow::current, loading, &LoadingPage::setProgress);
	connect(this, &MSTMainWindow::finished, loading, &LoadingPage::stopPage);
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
			// 数据库打不开只影响该目录，提示用户而不是直接终止进程
			Logger::Error("Failed to open local database at " + dir);
			errorReporter->popError(AppErrorType::DB_ERROR);
		}
	}
	else {
		remote.setPath(dir);
		remoteManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
		if (!remote.openDB()) {
			Logger::Error("Failed to open remote database at " + dir);
			errorReporter->popError(AppErrorType::DB_ERROR);
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
	const QStringList dupeList = QueryItem::findDuplicatesInSorted(items);
	for (const auto& i : dupeList) {
		Logger::Info(
			"Found duplicated music named " + i + " at " + (
				path == PathType::LOCAL ? local.getPath() : remote.getPath()));
		duplicateDialog.add(i);
	}
	duplicateDialog.exec();
	return dupeList;
}

void MSTMainWindow::showSettings() {
	// 挂到主窗口下并设置关闭即销毁，避免标题栏关闭时窗口泄漏
	const auto page = new Settings(entity, this);
	page->setAttribute(Qt::WA_DeleteOnClose);
	connect(page, &Settings::confirmPressed, this, &MSTMainWindow::saveSettings);
	page->show();
}

/**
 * @details base-style.qss 定义基础样式，mode 决定明暗，accent 决定强调色；
 *          文件缺失时跳过该段，保证界面仍可用
 */
void MSTMainWindow::applyTheme(const SettingsData& entity) {
	QString modeStylePath = ":/MusicSyncTool/style-light.qss";
	if (toThemeMode(entity.themeMode) == ThemeMode::DARK) {
		modeStylePath = ":/MusicSyncTool/style-dark-mode.qss";
	}

	QString accentStylePath = ":/MusicSyncTool/style.qss";
	switch (toAccentColor(entity.accentColor)) {
	case AccentColor::GREEN:
		accentStylePath = ":/MusicSyncTool/style-green.qss";
		break;
	case AccentColor::PURPLE:
		accentStylePath = ":/MusicSyncTool/style-purple.qss";
		break;
	case AccentColor::DARK:
		accentStylePath = ":/MusicSyncTool/style-dark.qss";
		break;
	case AccentColor::BLUE:
	default:
		break;
	}

	QString styleSheet;
	const QStringList stylePaths = {":/MusicSyncTool/base-style.qss", modeStylePath, accentStylePath};
	for (const QString& stylePath : stylePaths) {
		QFile styleFile(stylePath);
		if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
			styleSheet += styleFile.readAll();
			styleSheet += "\n";
			styleFile.close();
		}
	}
	qApp->setStyleSheet(styleSheet);
}

void MSTMainWindow::saveSettings(const SettingsData& entityParam) {
	if (!MSTSettingsManager::saveSettings(entityParam)) {
		// 不终止进程：设置写入失败只影响本次保存
		Logger::Error("Error opening settings file");
		QMessageBox::critical(this, tr("错误"), tr("无法打开设置文件"));
		return;
	}
	const SettingsData previous = this->entity;
	this->entity = entityParam;

	if (previous.themeMode != entityParam.themeMode || previous.accentColor != entityParam.accentColor) {
		MSTMainWindow::applyTheme(entityParam);
	}
	if (previous.logToFile != entityParam.logToFile) {
		Logger::setLogToFile(entityParam.logToFile);
	}
	if (previous.sortBy != entityParam.sortBy || previous.orderBy != entityParam.orderBy) {
		if (local.getPath() != "") {
			scanController->resetScan(PathType::LOCAL);
			getMusic(PathType::LOCAL, 1);
		}
		if (remote.getPath() != "") {
			scanController->resetScan(PathType::REMOTE);
			getMusic(PathType::REMOTE, 1);
		}
	}
	// 规则、收藏标签与递归开关都会改变扫描结果，需要重新扫描
	if (previous.rules != entity.rules || entityParam.favoriteTag != previous.favoriteTag
	    || entityParam.recursiveScan != previous.recursiveScan) {
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
		// 错误回调（运行于工作线程，只发信号，登记由 GUI 线程完成）
		[this](const QString& fileName, const int errorType) {
			switch (errorType) {
			case 0: // DUPLICATE
				emit fileCopyError(fileName, FileErrorType::DUPLICATE_FILE);
				break;
			case 1: // LNF
				emit fileCopyError(fileName, FileErrorType::LYRIC_NOT_FOUND);
				break;
			case 2: // DISKFULL
				emit fileCopyError(fileName, FileErrorType::DISKFULL);
				break;
			default:
				Logger::Warn("Unknown copy error type: " + QString::number(errorType) + " for " + fileName);
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

void MSTMainWindow::on_actionSettings_triggered(bool triggered) { showSettings(); }
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
	startCopy(local.getPath(), fileList, remote.getPath());
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
	startCopy(remote.getPath(), fileList, local.getPath());
}

/**
 * @details 复制任务由 copyWatcher 持有，窗口析构时会等待其结束；
 *          同一时间只允许一个复制任务，避免进度与错误列表互相干扰
 */
void MSTMainWindow::startCopy(const QString& source, const QStringList& fileList, const QString& target) {
	if (copyOperationInProgress.exchange(true)) {
		Logger::Warn("Copy operation already in progress, request ignored");
		return;
	}
	copyWatcher.setFuture(QtConcurrent::run(&MSTMainWindow::copyMusic, this, source, fileList, target));
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
	// 走正常关闭流程：触发析构，等待后台任务结束后再释放资源
	close();
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

void MSTMainWindow::on_copyFinished(OperationType op) {
	copyOperationInProgress.store(false);
	if (local.getPath() != "") {
		setAvailableSpace(PathType::LOCAL);
	}
	if (remote.getPath() != "") {
		setAvailableSpace(PathType::REMOTE);
	}
}

/**
 * @details 扫描任务在工作线程完成取数后发出 tablePageReady，
 *          由本槽在 GUI 线程更新表格内容、页码标签与滚动位置
 */
void MSTMainWindow::applyTablePage(const PathType path, const QList<QueryItem> items,
                                   const unsigned short page, const unsigned short totalPages) {
	tableManager->populateTable(path, items, page, totalPages);
	tableManager->scrollToTop(path);
}
