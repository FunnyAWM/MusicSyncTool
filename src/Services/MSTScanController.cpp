#include "MSTScanController.h"

#include <algorithm>
#include <ctime>
#include <utility>
#include <QDirIterator>
#include <QFile>
#include <QFileDevice>
#include <QMetaObject>
#include <QRegularExpression>
#include <QtConcurrentRun>

#include <taglib/tpropertymap.h>
#include "Logger.h"
#include "MSTDataSource.h"
#include "MSTFileManager.h"
#include "MSTSettingsManager.h"
#include "../Core/MSTTagUtils.h"
#include "../Core/MusicProperties.h"
#include "../Data/QueryItem.h"
#include "../UI/MusicSyncTool/MSTTableManager.h"

MSTScanController::MSTScanController(MSTDataSource& localDataSource,
                                     MSTDataSource& remoteDataSource,
                                     MSTTableManager* tableManager,
                                     QObject* parent)
	: QObject(parent),
	  localDataSource(localDataSource),
	  remoteDataSource(remoteDataSource),
	  tableManager(tableManager) {
	// 扫描任务在工作线程发射信号回 GUI 线程，参数类型必须已注册
	PROPERTIES::registerMetatypes();
	qRegisterMetaType<QList<QueryItem>>("QList<QueryItem>");
}

MSTDataSource& MSTScanController::getDataSource(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource : remoteDataSource;
}

QString MSTScanController::getPath(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource.getPath() : remoteDataSource.getPath();
}

/**
 * @details 根据扫描标记决定执行完整扫描还是仅加载页面数据。
 *          已完成过扫描的路径将直接加载页面，避免翻页时重复扫描。
 *          扫描任务串行执行，并由 scanWatcher 持有 future，保证析构前任务已结束。
 */
void MSTScanController::startScan(const PathType path, const unsigned short page, const SettingsData& entity) {
	if (localDataSource.getPath().isEmpty() && remoteDataSource.getPath().isEmpty()) {
		Logger::Warn("No path selected");
		return;
	}
	if (scanWatcher.isRunning()) {
		scanWatcher.waitForFinished();
	}
	// 页码与过滤状态属于 GUI 状态，在 GUI 线程设置，工作线程不再触碰
	tableManager->setCurrentPage(path, page);
	tableManager->setFavoriteOnly(path, false);
	const int idx = static_cast<int>(path);
	if (scanned[idx].load()) {
		scanWatcher.setFuture(QtConcurrent::run(&MSTScanController::loadPage, this, path, page, entity));
	}
	else {
		scanWatcher.setFuture(QtConcurrent::run(&MSTScanController::scanConcurrent, this, path, page, entity));
	}
}

bool MSTScanController::isScanRunning() const { return scanWatcher.isRunning(); }

void MSTScanController::waitForFinished() {
	if (scanWatcher.isRunning()) {
		scanWatcher.waitForFinished();
	}
}

void MSTScanController::scanConcurrent(const PathType path, const unsigned short page, const SettingsData& entity) {
	const clock_t start = clock();
	MSTDataSource& dataSource = getDataSource(path);

	if (page == 1) {
		Logger::Info("Scanning started");
		// 扫描一开始就显示加载页：即使没有新增文件，收藏与规则也要重新比对
		emit scanStart();
		const QString dirPath = getPath(path);
		const QString logFilePath = MSTSettingsManager::buildLogFileName(dirPath);
		QDateTime dateTime = MSTSettingsManager::getDateFromLog(logFilePath);
		const QDir dir(dirPath);

		dataSource.initTable();
		QStringList dirFileList;
		if (entity.recursiveScan) {
			// 递归扫描：记录相对音乐根目录的路径，保留子目录结构
			QDirIterator iterator(dirPath, QDir::Files, QDirIterator::Subdirectories);
			while (iterator.hasNext()) {
				dirFileList.append(dir.relativeFilePath(iterator.next()));
			}
		}
		else {
			dirFileList = dir.entryList(QDir::Files);
		}
		// 过滤非音频文件与数据库文件（原实现在遍历中删除元素，会漏掉相邻项）
		QStringList newFileList;
		for (const QString& file : std::as_const(dirFileList)) {
			if (file != "musicInfo.db" && MSTFileManager::isFormatSupported(file)) {
				newFileList.append(file);
			}
		}
		QList<QueryItem> storedDbItems = dataSource.getAll({PROPERTIES::QueryRows::FILENAME});
		QStringList oldFileList;
		for (const QueryItem& item : storedDbItems) {
			oldFileList.append(item.getFileName());
		}
		QStringList dirFiles = newFileList;
		QStringList dbFiles = oldFileList;
		dirFiles.sort();
		dbFiles.sort();

		// 删除列表：数据库中存在但目录中不存在的文件
		QStringList deleteList;
		std::set_difference(dbFiles.begin(), dbFiles.end(),
		                    dirFiles.begin(), dirFiles.end(),
		                    std::back_inserter(deleteList));

		// 存量文件：数据库和目录中都存在的文件（用于后续修改时间增量判断）
		QStringList existingFiles;
		std::set_intersection(dbFiles.begin(), dbFiles.end(),
		                      dirFiles.begin(), dirFiles.end(),
		                      std::back_inserter(existingFiles));

		// 新增列表：目录中存在但数据库中不存在的文件
		QStringList addedList;
		std::set_difference(dirFiles.begin(), dirFiles.end(),
		                    dbFiles.begin(), dbFiles.end(),
		                    std::back_inserter(addedList));
		newFileList.clear();
		for (const QString& f : addedList) {
			if (MSTFileManager::isFormatSupported(f)) {
				newFileList.append(f);
			}
		}
		newFileList.removeOne("musicInfo.db");

		if (!dataSource.deleteMusic(deleteList)) {
			emit errorOccurred(AppErrorType::DB_ERROR);
			emit scanFinished();
			return;
		}

		// 第一段进度：逐文件比对修改时间（原实现此阶段没有任何进度反馈）
		const int compareTotal = static_cast<int>(existingFiles.size());
		emit scanTotal(compareTotal);
		QStringList filesToScan = newFileList;
		int compared = 0;
		for (const auto& file : existingFiles) {
			if (QFile(dir.filePath(file)).fileTime(QFileDevice::FileModificationTime) > dateTime) {
				filesToScan.append(file);
			}
			++compared;
			// 每 20 个文件上报一次，避免高频信号冲刷事件队列
			if (compared % 20 == 0) {
				emit scanCurrent(compared);
			}
		}
		emit scanCurrent(compareTotal);

		// 第二段进度 = 添加文件 + 收藏扫描 + 规则扫描
		const int globalTotal = static_cast<int>(newFileList.size() + filesToScan.size() + filesToScan.size());
		emit scanTotal(globalTotal);
		int globalProgress = 0;
		// 添加新文件到数据库（单事务整体持锁写入）
		QStringList failedFiles;
		if (!dataSource.addMusicBatch(newFileList, failedFiles)) {
			Logger::Error("Failed to write scanned music into database");
			emit errorOccurred(AppErrorType::DB_ERROR);
			emit scanFinished();
			return;
		}
		for (const QString& file : failedFiles) {
			Logger::Warn("Error adding music: " + file);
			emit loadErrorOccurred(file, LoadErrorType::FILE_NOT_SCANNABLE);
		}
		globalProgress += static_cast<int>(newFileList.size());
		emit scanCurrent(globalProgress);
		clock_t endPhase = clock();
		Logger::Info("Basic scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");

		setFavorite(path, filesToScan, entity.favoriteTag, globalProgress);
		globalProgress += static_cast<int>(filesToScan.size());
		endPhase = clock();
		Logger::Info("Favorite scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");
		setRuleHit(path, entity.rules, filesToScan, globalProgress);
		globalProgress += static_cast<int>(filesToScan.size());
		endPhase = clock();
		Logger::Info("Rule scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");
		dateTime = QDateTime::currentDateTime();
		MSTSettingsManager::writeLog(logFilePath, dateTime);
		// 扫描结束必须关闭加载页，否则界面会停留在加载状态
		emit scanFinished();
		const clock_t end = clock();
		Logger::Info(
			"Scanning finished in " + QString::number(static_cast<double>(end - start) / CLOCKS_PER_SEC) + " seconds");
	}
	scanned[static_cast<int>(path)].store(true);
	const QList<QueryItem> tableData = dataSource.getMusicToTable(page, PROPERTIES::toSortBy(entity.sortBy),
		PROPERTIES::toOrderBy(entity.orderBy));
	// 总页数向上取整；总数为 0 时至少显示 1 页，避免出现 "1/0"
	const int musicCount = dataSource.getCount();
	const unsigned short totalPages = static_cast<unsigned short>(qMax(
		1, (musicCount + dataSource.getPageSize() - 1) / dataSource.getPageSize()));
	emit tablePageReady(path, tableData, page, totalPages);
}

void MSTScanController::setFavorite(const PathType path,
                                    const QStringList& filesToScan, const QString& favoriteTag,
                                    const qsizetype progressOffset) {
	if (favoriteTag.isEmpty()) {
		return;
	}
	MSTDataSource& dataSource = getDataSource(path);
	// 逐文件扫描标签判定收藏状态并报告进度
	QList<FavoriteUpdate> results;
	for (qsizetype i = 0; i < filesToScan.size(); i++) {
		emit scanCurrent(static_cast<int>(progressOffset + i));
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(
			QDir(dataSource.getPath()).filePath(filesToScan.at(i)));
		FavoriteUpdate favResult;
		favResult.fileName = filesToScan.at(i);
		favResult.isFavorite = fileRef.properties().contains(favoriteTag.toStdString().c_str());
		results.append(favResult);
	}
	dataSource.updateFavoriteBatch(results);
}

void MSTScanController::setRuleHit(const PathType path, const QList<LyricIgnoreRule>& rules,
                                   const QStringList& filesToFilter, const qsizetype progressOffset) {
	MSTDataSource& dataSource = getDataSource(path);
	if (rules.isEmpty()) {
		return;
	}
	// 通过 SQL 层面过滤，只获取需要扫描的文件记录
	const QList<QueryItem> items = dataSource.getByFileNames(filesToFilter);
	// 逐项扫描规则命中状态并报告进度
	QList<RuleHitUpdate> results;
	for (qsizetype i = 0; i < items.size(); i++) {
		emit scanCurrent(static_cast<int>(progressOffset + i));
		const auto& item = items.at(i);
		RuleHitUpdate ruleResult;
		ruleResult.fileName = item.getFileName();
		ruleResult.isRuleHit = std::any_of(rules.begin(), rules.end(), [&item](const LyricIgnoreRule& rule) {
			QString fieldValue;
			switch (rule.getRuleField()) {
			case RuleField::TITLE:  fieldValue = item.getTitle();  break;
			case RuleField::ARTIST: fieldValue = item.getArtist(); break;
			case RuleField::ALBUM:  fieldValue = item.getAlbum();  break;
			}
			const QRegularExpression regExp(rule.getRuleName());
			const bool hasMatch = regExp.match(fieldValue).hasMatch();
			return (rule.getRuleType() == RuleType::INCLUDES) ? hasMatch : !hasMatch;
		});
		results.append(ruleResult);
	}
	dataSource.updateRuleHitBatch(results);
}

MSTTagScanner* MSTScanController::getTagScanner() {
	return &tagScanner;
}

/**
 * @details 从数据库读取指定页码的音乐数据并填充表格，
 *          用于已完成扫描后的翻页操作，避免重复扫描目录
 */
void MSTScanController::loadPage(const PathType path, const unsigned short page, const SettingsData& entity) {
	MSTDataSource& dataSource = getDataSource(path);
	emit scanTotal(dataSource.getPageSize());
	const QList<QueryItem> tableData = dataSource.getMusicToTable(page, PROPERTIES::toSortBy(entity.sortBy),
		PROPERTIES::toOrderBy(entity.orderBy));
	// 总页数向上取整；总数为 0 时至少显示 1 页，避免出现 "1/0"
	const int musicCount = dataSource.getCount();
	const unsigned short totalPages = static_cast<unsigned short>(qMax(
		1, (musicCount + dataSource.getPageSize() - 1) / dataSource.getPageSize()));
	emit tablePageReady(path, tableData, page, totalPages);
}

/**
 * @details 清除指定路径的已扫描标记，使下次startScan调用触发完整扫描
 */
void MSTScanController::resetScan(const PathType path) {
	scanned[static_cast<int>(path)].store(false);
}
