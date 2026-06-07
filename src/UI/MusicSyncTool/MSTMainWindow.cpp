/**
 * @file MSTMainWindow.cpp
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
#include "MSTMainWindow.h"
#include <algorithm>
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include "../../Services/Logger.h"
#include "../../Services/MSTSettingsManager.h"
#include "MSTTableManager.h"
#include "MSTMediaController.h"
#include "MSTErrorReporter.h"
#include "../../Services/MSTScanController.h"

#if defined(__linux)
#include <unistd.h>
#endif

/**
 * @brief 音乐同步工具主窗口构造函数
 * @param parent 父窗口部件指针，默认为nullptr
 * @details 初始化音乐同步工具主窗口，包括数据库连接、设置加载、
 *          用户界面初始化、语言加载、媒体播放器初始化和信号槽连接
 */
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

/**
 * @brief 音乐同步工具主窗口析构函数
 * @details 清理资源，包括关闭数据库连接、删除动态分配的对象，
 *          确保程序正常退出时没有内存泄漏
 */
MSTMainWindow::~MSTMainWindow() {
	if (local.isOpen()) {
		local.closeDB();
	}
	if (remote.isOpen()) {
		remote.closeDB();
	}
	delete copyStats;
	delete scanController;
	delete mediaController;
	delete errorReporter;
	player.reset();
	delete loading;
}

/**
 * @brief 初始化数据库连接
 * @details 为本地和远程数据源设置不同的连接名称，用于区分不同的数据库实例
 *          本地数据库用于存储本地路径的音乐文件信息，远程数据库用于存储远程路径的音乐文件信息
 */
void MSTMainWindow::initDatabase() {
	local.setConnectionName("local");
	remote.setConnectionName("remote");
}

/**
 * @brief 从设置文件加载用户配置
 * @details 委托MSTSettingsManager从settings.json文件加载设置实体，
 *          如果文件不存在或格式错误，MSTSettingsManager将创建默认设置
 */
void MSTMainWindow::loadSettings() {
	MSTSettingsManager::loadSettings(entity);
}

/**
 * @brief 根据设置加载语言文件
 * @details 读取translations/langinfo.json配置文件，根据用户设置的语言加载相应的翻译文件。
 *          如果用户未设置语言，默认使用中文。加载翻译文件后，安装翻译器并重新翻译界面文本。
 *          支持的语言包括中文、英文等多种语言。
 */
void MSTMainWindow::loadLanguage() {
	QFile file(QApplication::applicationDirPath() + "/translations/langinfo.json");
	if (!file.open(QIODevice::ReadOnly)) {
		errorReporter->popError(PET::NOLANG);
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

/**
 * @brief 初始化用户界面组件
 * @details 设置主窗口的用户界面，包括：
 *          - 加载UI布局文件
 *          - 设置窗口图标
 *          - 配置表格控件的列宽自适应模式
 *          - 初始化音量滑块和标签的默认值（50%）
 *          - 设置播放状态显示文本
 */
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

/**
 * @brief 弹出文件夹选择对话框并将路径加载到数据库
 * @param path 路径类型（本地或远程）
 * @details 打开文件夹选择对话框让用户选择音乐文件夹路径。
 *          根据路径类型创建相应的文件管理器和数据库连接：
 *          - LOCAL: 设置本地路径，创建本地文件管理器，打开本地数据库
 *          - REMOTE: 设置远程路径，创建远程文件管理器，打开远程数据库
 *          如果数据库打开失败，程序将退出
 */
void MSTMainWindow::openFolder(const PathType path) {
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
	}
	else {
		remote.setPath(dir);
		remoteManager = std::make_shared<MSTFileManager>(MSTFileManager(dir));
		if (!remote.openDB()) {
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * @brief 启动音乐扫描
 * @param path 路径类型（本地或远程）
 * @param page 页码
 * @details 委托给scanController执行扫描操作
 */
void MSTMainWindow::getMusic(const PathType path, const unsigned short page) {
	scanController->startScan(path, page, entity);
}

/**
 * @brief 弹出错误对话框（委托给errorReporter）
 */
void MSTMainWindow::popError(const PET type) {
	errorReporter->popError(type);
}

/**
 * @brief 获取重复的音乐文件并显示
 * @param path 路径类型（本地或远程）
 * @return 重复音乐文件的文件名列表
 * @details 检查指定路径下的音乐文件，识别标题和艺术家相同的重复文件。
 *          如果未选择路径，将显示错误提示。通过ShowDupe对话框展示
 *          重复文件列表，让用户选择要删除的重复文件
 */
QStringList MSTMainWindow::getDuplicatedMusic(const PathType path) {
	const QString selectedPath = path == PathType::LOCAL ? local.getPath() : remote.getPath();
	if (selectedPath == "") {
		Logger::Warn("No path selected");
		errorReporter->popError(PET::NPS);
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
		Logger::Info(
			"Found duplicated music named " + i + " at " + (
				path == PathType::LOCAL ? local.getPath() : remote.getPath()));
		dp.add(i);
	}
	dp.exec();
	return dupeList;
}

/**
 * @brief 显示设置对话框
 * @details 创建并显示设置页面对话框，连接确认信号到保存设置的槽函数，
 *          允许用户修改应用程序的各种配置选项，包括语言、喜爱标签、
 *          排序方式和歌词忽略规则等
 */
void MSTMainWindow::showSettings() const {
	const auto page = new Settings(entity);
	connect(page, SIGNAL(confirmPressed(set)), this, SLOT(saveSettings(set)));
	page->show();
}

/**
 * @brief 保存设置到设置文件
 * @param entityParam 设置实体对象
 * @details 委托MSTSettingsManager将用户修改的设置保存到settings.json文件中。
 *          如果排序设置发生变化，会触发界面刷新以应用新的排序。
 *          如果规则或喜爱标签发生变化，会清除日志并重新扫描。
 */
void MSTMainWindow::saveSettings(const set& entityParam) {
	if (!MSTSettingsManager::saveSettings(entityParam)) {
		Logger::Fatal("Error opening settings file");
		QMessageBox::critical(this, tr("错误"), tr("无法打开设置文件"));
		return;
	}
	const int tempSort = this->entity.sortBy;
	const int tempOrder = this->entity.orderBy;
	const QList<LyricIgnoreRule> tempRules = this->entity.rules;
	const QString tempTag = this->entity.favoriteTag;
	this->entity = entityParam;
	if (tempSort != entityParam.sortBy || tempOrder != entityParam.orderBy) {
		if (local.getPath() != "") {
			scanController->resetScan(PathType::LOCAL);
			getMusic(PathType::LOCAL, 1);
		}
		if (remote.getPath() != "") {
			scanController->resetScan(PathType::REMOTE);
			getMusic(PathType::REMOTE, 1);
		}
	}
	if (tempRules != entity.rules || entityParam.favoriteTag != tempTag) {
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
	Logger::Info("Recursive scan: " + this->entity.recursiveScan);
	int i = 1;
	for (const auto& rule : entityParam.rules) {
		Logger::Info("Rule No." + QString::number(i++) + ":");
		Logger::Info("RuleTypeConverter: " + rule.getRuleTypeStr());
		Logger::Info("RuleFieldConverter: " + rule.getRuleFieldStr());
		Logger::Info("Rule: " + rule.getRuleName());
	}
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
				errorReporter->addToErrorList(fileName, FileErrorType::LNF);
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

/**
 * @brief 获取当前设置的语言
 * @return 语言字符串
 * @details 返回用户在设置中选择的界面语言，
 *          用于语言切换和界面本地化
 */
[[nodiscard]]
QString MSTMainWindow::getLanguage() const {
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

/**
 * @brief 本地打开操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 处理用户点击本地路径打开菜单的操作：
 *          1. 打开文件夹选择对话框
 *          2. 如果选择了路径，开始扫描音乐文件
 *          3. 设置可用空间显示
 *          4. 显示加载操作结果
 */
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

/**
 * @brief 设置操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击设置菜单，打开设置对话框
 *          允许用户修改应用程序的各种配置选项
 */
void MSTMainWindow::on_actionSettings_triggered(bool triggered) const { showSettings(); }
/**
 * @brief 关于操作的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击关于菜单，显示应用程序的关于信息对话框，
 *          包括版本信息、开发者信息等内容
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void MSTMainWindow::on_actionAbout_triggered(bool triggered) {
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
void MSTMainWindow::on_copyToRemote_clicked() {
	if (local.getPath() == "") {
		errorReporter->popError(PET::NPS);
		return;
	}
	if (remote.getPath() == "") {
		errorReporter->popError(PET::NDP);
		return;
	}
	QStringList fileList = tableManager->getSelectedMusic(PathType::LOCAL);
	if (fileList.isEmpty()) {
		errorReporter->popError(PET::NFS);
		return;
	}
	for (QString& file : fileList) {
		file = file + ":" + (local.getRuleHit(file) ? "1" : "0"); // 添加规则命中状态
	}
	QFuture<void> future = QtConcurrent::run(&MSTMainWindow::copyMusic, this, local.getPath(), fileList,
	                                         remote.getPath());
}

/**
 * @brief 复制到本地的槽函数
 * @details 处理用户点击复制到本地按钮的操作：
 *          1. 检查远程路径是否已选择
 *          2. 获取用户在远程表格中选中的音乐文件
 *          3. 检查是否有选中的文件
 *          4. 启动并发线程执行复制操作
 */
void MSTMainWindow::on_copyToLocal_clicked() {
	if (remote.getPath() == "") {
		errorReporter->popError(PET::NPS);
		return;
	}
	if (local.getPath() == "") {
		errorReporter->popError(PET::NDP);
		return;
	}
	QStringList fileList = tableManager->getSelectedMusic(PathType::REMOTE);
	if (fileList.isEmpty()) {
		errorReporter->popError(PET::NFS);
		return;
	}
	for (QString& file : fileList) {
		file = file + ":" + (remote.getRuleHit(file) ? "1" : "0"); // 添加规则命中状态
	}
	QFuture<void> future = QtConcurrent::run(&MSTMainWindow::copyMusic, this, remote.getPath(), fileList,
	                                         local.getPath());
}

/**
 * @brief 本地重复音乐扫描的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击本地重复音乐扫描菜单，
 *          在本地路径中查找重复的音乐文件并显示结果
 */
void MSTMainWindow::on_actionDupeLocal_triggered(bool triggered) { getDuplicatedMusic(PathType::LOCAL); }
/**
 * @brief 远程重复音乐扫描的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击远程重复音乐扫描菜单，
 *          在远程路径中查找重复的音乐文件并显示结果
 */
void MSTMainWindow::on_actionDupeRemote_triggered(bool triggered) { getDuplicatedMusic(PathType::REMOTE); }
/**
 * @brief 刷新本地音乐列表的槽函数
 * @details 响应用户点击本地刷新按钮，重新扫描本地路径
 *          并重新加载音乐列表到第一页
 */
void MSTMainWindow::on_refreshLocal_clicked() {
	scanController->resetScan(PathType::LOCAL);
	getMusic(PathType::LOCAL, 1);
}
/**
 * @brief 刷新远程音乐列表的槽函数
 * @details 响应用户点击远程刷新按钮，重新扫描远程路径
 *          并重新加载音乐列表到第一页
 */
void MSTMainWindow::on_refreshRemote_clicked() {
	scanController->resetScan(PathType::REMOTE);
	getMusic(PathType::REMOTE, 1);
}
/**
 * @brief 本地搜索的槽函数
 * @details 响应用户在本地搜索框中按下回车键，
 *          根据输入的文本在本地音乐列表中进行搜索
 */
void MSTMainWindow::on_searchLocal_returnPressed() { tableManager->searchMusic(PathType::LOCAL, ui.searchLocal->text()); }
/**
 * @brief 远程搜索的槽函数
 * @details 响应用户在远程搜索框中按下回车键，
 *          根据输入的文本在远程音乐列表中进行搜索
 */
void MSTMainWindow::on_searchRemote_returnPressed() { tableManager->searchMusic(PathType::REMOTE, ui.searchRemote->text()); }
/**
 * @brief 本地表格双击预览的槽函数
 * @param row 双击的行号
 * @param column 双击的列号（未使用）
 */
void MSTMainWindow::on_tableWidgetLocal_cellDoubleClicked(const int row, int column) {
	mediaController->playSelectedTrack(PathType::LOCAL, row);
}

/**
 * @brief 远程表格双击预览的槽函数
 * @param row 双击的行号
 * @param column 双击的列号（未使用）
 */
void MSTMainWindow::on_tableWidgetRemote_cellDoubleClicked(const int row, int column) {
	mediaController->playSelectedTrack(PathType::REMOTE, row);
}

/**
 * @brief 退出程序的槽函数
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击退出菜单，正常退出应用程序
 */
// ReSharper disable once CppMemberFunctionMayBeStatic
void MSTMainWindow::on_actionExit_triggered(bool triggered) {
	exit(EXIT_SUCCESS);
} // NOLINT(*-convert-member-functions-to-static)

/**
 * @brief 弹出窗口询问用户是否要删除所有日志文件
 * @param triggered 触发状态（未使用）
 * @details 响应用户点击清除日志文件菜单，显示确认对话框，
 *          如果用户确认则清除所有扫描日志文件，并显示完成提示
 */
void MSTMainWindow::on_actionClean_log_files_triggered(bool triggered) {
	const QMessageBox::StandardButton reply =
		QMessageBox::warning(this, tr("提示"), tr("确定要清除所有日志文件吗？"), QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::No) {
		return;
	}
	MSTSettingsManager::cleanLog();
	QMessageBox::information(this, tr("提示"), tr("日志文件已清除"));
}

/**
 * @brief 播放或暂停音乐
 * @details 响应用户点击播放控制按钮：
 *          - 如果没有选定音频文件，显示错误提示
 *          - 如果正在播放，则暂停播放并更新界面状态
 *          - 如果已暂停或停止，则开始播放并更新界面状态
 */
void MSTMainWindow::on_playControl_clicked() {
	mediaController->togglePlayPause();
}

/**
 * @brief 播放滑块移动的槽函数
 * @param position 滑块位置
 */
void MSTMainWindow::on_playSlider_sliderMoved(const int position) const {
	mediaController->seekToPosition(position);
}

/**
 * @brief 播放滑块按下的槽函数
 */
void MSTMainWindow::on_playSlider_sliderPressed() const { mediaController->onPlaySliderPressed(); }
void MSTMainWindow::on_volumeSlider_sliderPressed() const {
	mediaController->onVolumeSliderPressed();
}

/**
 * @brief 本地收藏按钮的槽函数
 * @details 响应用户点击本地收藏按钮，显示第一页的收藏音乐列表
 */
void MSTMainWindow::on_favoriteOnlyLocal_clicked() {
	tableManager->getFavoriteMusic(PathType::LOCAL, 1, entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}
/**
 * @brief 远程收藏按钮的槽函数
 * @details 响应用户点击远程收藏按钮，显示第一页的收藏音乐列表
 */
void MSTMainWindow::on_favoriteOnlyRemote_clicked() {
	tableManager->getFavoriteMusic(PathType::REMOTE, 1, entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}
/*
 * @brief 上一页（本地）的槽函数
 */
void MSTMainWindow::on_lastPageLocal_clicked() {
	tableManager->goToPrevPage(PathType::LOCAL, local.getPath() != "",
		entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

/*
 * @brief 下一页（本地）的槽函数
 */
void MSTMainWindow::on_nextPageLocal_clicked() {
	tableManager->goToNextPage(PathType::LOCAL, local.getPath() != "",
		entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

/*
 * @brief 上一页（远程）的槽函数
 */
void MSTMainWindow::on_lastPageRemote_clicked() {
	tableManager->goToPrevPage(PathType::REMOTE, remote.getPath() != "",
		entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

/*
 * @brief 下一页（远程）的槽函数
 */
void MSTMainWindow::on_nextPageRemote_clicked() {
	tableManager->goToNextPage(PathType::REMOTE, remote.getPath() != "",
		entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
}

/*
 * @brief 音量滑块移动的槽函数
 */
void MSTMainWindow::on_volumeSlider_sliderMoved(const int position) const {
	mediaController->setVolumeFromSlider(position);
}

/*
 * @brief Slots for volume slider
 */
void MSTMainWindow::on_volumeSlider_valueChanged(const int position) const { on_volumeSlider_sliderMoved(position); }

void MSTMainWindow::on_copyFinished(OperationType op) const {
	if (local.getPath() != "") {
		setAvailableSpace(PathType::LOCAL);
	}
	if (remote.getPath() != "") {
		setAvailableSpace(PathType::REMOTE);
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
void MSTMainWindow::connectSlots() {
	connect(&local, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&remote, &MSTDataSource::totalSize, loading, &LoadingPage::setTotal);
	connect(&local, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(&remote, &MSTDataSource::currentProgress, loading, &LoadingPage::setProgress);
	connect(&local, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
	connect(&remote, &MSTDataSource::loadStarted, loading, &LoadingPage::showPage);
	connect(&local, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
	connect(&remote, QOverload<>::of(&MSTDataSource::loadFinished), loading, &LoadingPage::stopPage);
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
		tableManager->getFavoriteMusic(path, page, entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
	});
	connect(scanController, &MSTScanController::scanTotal, this, &MSTMainWindow::total);
	connect(scanController, &MSTScanController::scanCurrent, this, &MSTMainWindow::current);
	connect(scanController, &MSTScanController::errorOccurred, errorReporter, &MSTErrorReporter::popError);
	connect(scanController, &MSTScanController::loadErrorOccurred, errorReporter,
	        QOverload<const QString&, LoadErrorType>::of(&MSTErrorReporter::addToErrorList));
	// 标签扫描器进度信号
	connect(scanController->getTagScanner(), &MSTTagScanner::loadStarted, loading, &LoadingPage::showPage);
	connect(scanController, &MSTScanController::scanTotal, loading, &LoadingPage::setTotal);
	connect(scanController, &MSTScanController::scanCurrent, loading, &LoadingPage::setProgress);
	connect(scanController->getTagScanner(), &MSTTagScanner::totalSize, loading, &LoadingPage::setTotal);
	connect(scanController->getTagScanner(), &MSTTagScanner::currentProgress, loading, &LoadingPage::setProgress);
	connect(errorReporter, &MSTErrorReporter::requestRefreshMusic, this, [this](PathType path, unsigned short page) {
		if (tableManager->isFavoriteOnly(path)) {
			tableManager->getFavoriteMusic(path, page, entity.favoriteTag, toSortBy(entity.sortBy), toOrderBy(entity.orderBy));
		} else {
			scanController->resetScan(path);
			getMusic(path, page);
		}
	});
	mediaController->connectSignals();
	connect(mediaController, &MSTMediaController::errorOccurred, errorReporter, &MSTErrorReporter::popError);
}

/**
 * @brief 获取可用空间并显示在顶部
 * @param path 路径类型（本地或远程）
 * @details 通过文件管理器获取指定路径的存储空间信息，
 *          并在用户界面上显示可用空间大小，帮助用户了解
 *          磁盘使用情况
 */
void MSTMainWindow::setAvailableSpace(const PathType path) const {
	const shared_ptr<MSTFileManager> manager = path == PathType::LOCAL ? localManager : remoteManager;
	const QStorageInfo storage(path == PathType::LOCAL ? local.getPath() : remote.getPath());
	const QString textBuilder = tr("可用空间：") + manager->getSpaceInfo();
	(path == PathType::LOCAL ? ui.availableSpaceLocal : ui.availableSpaceRemote)->setText(textBuilder);
}
