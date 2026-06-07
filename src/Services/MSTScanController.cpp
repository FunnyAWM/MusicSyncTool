/**
 * @file MSTScanController.cpp
 * @brief 音乐扫描控制器类的实现
 * @details 实现音乐文件目录扫描、文件差异比对、数据库同步和收藏/规则设置
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTScanController.h"

#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QtConcurrentRun>
#include <ctime>

#include "../Core/MusicProperties.h"
#include "../Data/QueryItem.h"
#include "Logger.h"
#include "MSTDataSource.h"
#include "MSTFileManager.h"
#include "MSTSettingsManager.h"
#include "../UI/MusicSyncTool/MSTTableManager.h"

/**
 * @brief 构造函数
 */
MSTScanController::MSTScanController(MSTDataSource& localDataSource,
                                     MSTDataSource& remoteDataSource,
                                     MSTTableManager* tableManager,
                                     QObject* parent)
	: QObject(parent),
	  localDataSource(localDataSource),
	  remoteDataSource(remoteDataSource),
	  tableManager(tableManager) {
}

/**
 * @brief 获取指定路径对应的数据源
 */
MSTDataSource& MSTScanController::getDataSource(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource : remoteDataSource;
}

/**
 * @brief 获取指定路径对应的目录路径
 */
QString MSTScanController::getPath(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource.getPath() : remoteDataSource.getPath();
}

/**
 * @brief 启动音乐扫描（多线程入口）
 * @details 根据扫描标记决定执行完整扫描还是仅加载页面数据。
 *          已完成过扫描的路径将直接加载页面，避免翻页时重复扫描。
 */
void MSTScanController::startScan(const PathType path, const unsigned short page, const set& entity) {
	if (localDataSource.getPath().isEmpty() && remoteDataSource.getPath().isEmpty()) {
		Logger::Warn("No path selected");
		return;
	}
	const int idx = static_cast<int>(path);
	if (scanned[idx]) {
		QtConcurrent::run(&MSTScanController::loadPage, this, path, page, entity);
	} else {
		QtConcurrent::run(&MSTScanController::scanConcurrent, this, path, page, entity);
	}
}

/**
 * @brief 执行音乐扫描（并发执行体）
 */
void MSTScanController::scanConcurrent(const PathType path, const unsigned short page, const set& entity) {
	const clock_t start = clock();
	MSTDataSource& ds = getDataSource(path);
	bool newFileFound = false;

	// 设置当前页码和收藏过滤状态
	tableManager->setCurrentPage(path, page);
	tableManager->setFavoriteOnly(path, false);
	if (page == 1) {
		Logger::Info("Scanning started");
		const QString dirPath = getPath(path);
		const QString logFileNameBuilder = MSTSettingsManager::buildLogFileName(dirPath);
		QDateTime dateTime = MSTSettingsManager::getDateFromLog(logFileNameBuilder);
		const QDir dir(dirPath);

		ds.initTable();
		QStringList newFileList = dir.entryList(QDir::Files);
		for (const QString& file : newFileList) {
			if (!MSTFileManager::isFormatSupported(file)) {
				newFileList.removeOne(file);
			}
		}
		if (!newFileList.isEmpty()) {
			emit ds.loadStarted();
			newFileFound = true;
		}
		QList<QueryItem> tempList = ds.getAll({PROPERTIES::QueryRows::FILENAME});
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
			emit errorOccurred(PET::DBERROR);
			return;
		}
		newFileList.removeOne("musicInfo.db");
		emit scanTotal(newFileList.size() + oldFileList.size());
		emit scanCurrent(0);
		for (int i = 0; i < newFileList.size(); i++) {
			emit scanCurrent(i);
			QString file = newFileList.at(i).toUtf8();
			if (!MSTFileManager::isFormatSupported(file)) {
				continue;
			}
			if (!ds.addMusic(file)) {
				Logger::Warn("Error adding music: " + file);
				emit loadErrorOccurred(file, LoadErrorType::FNS);
			}
		}
		// 构建需要扫描标签/规则的文件列表（增量：新文件 + 修改过的旧文件）
		QStringList filesToScan = newFileList;
		for (const auto& file : oldFileList) {
			if (QFile(dirPath + "/" + file).fileTime(QFileDevice::FileModificationTime) > dateTime) {
				filesToScan.append(file);
			}
		}
		setFavorite(path, filesToScan, entity.favoriteTag);
		setRuleHit(path, entity.rules, filesToScan);
		dateTime = QDateTime::currentDateTime();
		MSTSettingsManager::writeLog(logFileNameBuilder, dateTime);
		if (newFileFound) {
			emit ds.loadFinished();
		}
		const clock_t end = clock();
		Logger::Info(
			"Scanning finished in " + QString::number(static_cast<double>(end - start) / CLOCKS_PER_SEC) + " seconds");
	}
	scanned[static_cast<int>(path)] = true;
	emit scanTotal(ds.getPageSize());
	const QList<QueryItem> tableData = ds.getMusicToTable(page, PROPERTIES::toSortBy(entity.sortBy),
		PROPERTIES::toOrderBy(entity.orderBy));
	const unsigned short totalPages = static_cast<unsigned short>(ds.getCount() / ds.getPageSize() + 1);
	tableManager->populateTable(path, tableData, page, totalPages);
	emit tableManager->requestScroll(path);
}

/**
 * @brief 获取标签扫描器指针
 */
MSTTagScanner* MSTScanController::getTagScanner() {
	return &tagScanner;
}

/**
 * @brief 设置收藏标签
 */
void MSTScanController::setFavorite(const PathType path,
                                    const QStringList& filesToScan, const QString& favoriteTag) {
	MSTDataSource& ds = getDataSource(path);
	if (favoriteTag.isEmpty()) {
		return;
	}
	// 扫描标签判定收藏状态
	const auto updates = tagScanner.scanFavorite(ds.getPath(), filesToScan, favoriteTag);
	// 批量写入数据库
	ds.updateFavoriteBatch(updates);
}

/**
 * @brief 设置规则命中状态
 */
void MSTScanController::setRuleHit(const PathType path, const QList<LyricIgnoreRule>& rules,
                                   const QStringList& filesToFilter) {
	MSTDataSource& ds = getDataSource(path);
	if (rules.isEmpty()) {
		return;
	}
	// 获取所有音乐项，按增量文件列表过滤
	const QList<QueryItem> allItems = ds.getAll();
	QList<QueryItem> items;
	for (const auto& item : allItems) {
		if (filesToFilter.contains(item.getFileName())) {
			items.append(item);
		}
	}
	// 扫描规则命中状态
	const auto updates = tagScanner.scanRuleHit(ds.getPath(), items, rules);
	// 批量写入数据库
	ds.updateRuleHitBatch(updates);
}

/**
 * @brief 仅加载页面数据（不执行目录扫描）
 * @details 从数据库读取指定页码的音乐数据并填充表格，
 *          用于已完成扫描后的翻页操作，避免重复扫描目录
 */
void MSTScanController::loadPage(const PathType path, const unsigned short page, const set& entity) {
	MSTDataSource& ds = getDataSource(path);
	emit scanTotal(ds.getPageSize());
	const QList<QueryItem> tableData = ds.getMusicToTable(page, PROPERTIES::toSortBy(entity.sortBy),
		PROPERTIES::toOrderBy(entity.orderBy));
	const unsigned short totalPages = static_cast<unsigned short>(ds.getCount() / ds.getPageSize() + 1);
	tableManager->populateTable(path, tableData, page, totalPages);
	emit tableManager->requestScroll(path);
}

/**
 * @brief 重置扫描标记
 * @details 清除指定路径的已扫描标记，使下次startScan调用触发完整扫描
 */
void MSTScanController::resetScan(const PathType path) {
	scanned[static_cast<int>(path)] = false;
}
