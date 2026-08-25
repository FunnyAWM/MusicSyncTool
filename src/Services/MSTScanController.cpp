#include "MSTScanController.h"

#include <algorithm>
#include <ctime>
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
 */
void MSTScanController::startScan(const PathType path, const unsigned short page, const SettingsData& entity) {
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

void MSTScanController::scanConcurrent(const PathType path, const unsigned short page, const SettingsData& entity) {
	const clock_t start = clock();
	MSTDataSource& dataSource = getDataSource(path);

	tableManager->setCurrentPage(path, page);
	tableManager->setFavoriteOnly(path, false);
	if (page == 1) {
		bool newFileFound = false;
		Logger::Info("Scanning started");
		const QString dirPath = getPath(path);
		const QString logFilePath = MSTSettingsManager::buildLogFileName(dirPath);
		QDateTime dateTime = MSTSettingsManager::getDateFromLog(logFilePath);
		const QDir dir(dirPath);

		dataSource.initTable();
		QStringList newFileList = dir.entryList(QDir::Files);
		for (const QString& file : newFileList) {
			if (!MSTFileManager::isFormatSupported(file)) {
				newFileList.removeOne(file);
			}
		}
		if (!newFileList.isEmpty()) {
			emit scanStart();
			newFileFound = true;
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
			return;
		}

		// 构建需要扫描标签/规则的文件列表（增量：新文件 + 修改过的旧文件）
		QStringList filesToScan = newFileList;
		for (const auto& file : existingFiles) {
			if (QFile(dirPath + "/" + file).fileTime(QFileDevice::FileModificationTime) > dateTime) {
				filesToScan.append(file);
			}
		}
		
		// 全局总进度 = 添加文件 + 收藏扫描 + 规则扫描
		const qsizetype globalTotal = newFileList.size() + filesToScan.size() + filesToScan.size();
		emit scanTotal(globalTotal);
		qsizetype globalProgress = 0;
		// 添加新文件到数据库（事务包裹批量写入）
		dataSource.beginTransaction();
		for (int i = 0; i < newFileList.size(); i++) {
			emit scanCurrent(globalProgress + i);
			const QString file = newFileList.at(i);
			if (!dataSource.addMusic(file)) {
				Logger::Warn("Error adding music: " + file);
				emit loadErrorOccurred(file, LoadErrorType::FILE_NOT_SCANNABLE);
			}
		}
		clock_t endPhase = clock();
		Logger::Info("Basic scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");

		dataSource.commitTransaction();
		globalProgress += newFileList.size();
		setFavorite(path, filesToScan, entity.favoriteTag, globalProgress);
		globalProgress += filesToScan.size();
		endPhase = clock();
		Logger::Info("Favorite scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");
		setRuleHit(path, entity.rules, filesToScan, globalProgress);
		globalProgress += filesToScan.size();
		endPhase = clock();
		Logger::Info("Rule scanning finished in " + QString::number(static_cast<double>(endPhase - start) / CLOCKS_PER_SEC) + " seconds");
		dateTime = QDateTime::currentDateTime();
		MSTSettingsManager::writeLog(logFilePath, dateTime);
		if (newFileFound) {
			emit scanFinished();
		}
		const clock_t end = clock();
		Logger::Info(
			"Scanning finished in " + QString::number(static_cast<double>(end - start) / CLOCKS_PER_SEC) + " seconds");
	}
	scanned[static_cast<int>(path)] = true;
	const QList<QueryItem> tableData = dataSource.getMusicToTable(page, PROPERTIES::toSortBy(entity.sortBy),
		PROPERTIES::toOrderBy(entity.orderBy));
	const unsigned short totalPages = static_cast<unsigned short>(dataSource.getCount() / dataSource.getPageSize() + 1);
	tableManager->populateTable(path, tableData, page, totalPages);
	emit tableManager->requestScroll(path);
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
		emit scanCurrent(progressOffset + i);
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(dataSource.getPath() + "/" + filesToScan.at(i));
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
		emit scanCurrent(progressOffset + i);
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
	const unsigned short totalPages = static_cast<unsigned short>(dataSource.getCount() / dataSource.getPageSize() + 1);
	tableManager->populateTable(path, tableData, page, totalPages);
	emit tableManager->requestScroll(path);
}

/**
 * @details 清除指定路径的已扫描标记，使下次startScan调用触发完整扫描
 */
void MSTScanController::resetScan(const PathType path) {
	scanned[static_cast<int>(path)] = false;
}
