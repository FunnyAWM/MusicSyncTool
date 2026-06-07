/**
 * @file MSTTableManager.cpp
 * @brief 音乐表格管理器类的实现
 * @details 实现本地和远程音乐表格的数据填充、分页导航、搜索和选择操作
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTTableManager.h"
#include "../../Services/MSTDataSource.h"
#include "../../Core/MusicProperties.h"

/**
 * @brief 构造函数
 * @param localTable 本地音乐表格控件
 * @param remoteTable 远程音乐表格控件
 * @param localPageLabel 本地分页标签控件
 * @param remotePageLabel 远程分页标签控件
 * @param localDataSource 本地数据源引用
 * @param remoteDataSource 远程数据源引用
 * @param parent 父对象指针
 */
MSTTableManager::MSTTableManager(QTableWidget* localTable,
                                 QTableWidget* remoteTable,
                                 QLabel* localPageLabel,
                                 QLabel* remotePageLabel,
                                 MSTDataSource& localDataSource,
                                 MSTDataSource& remoteDataSource,
                                 QObject* parent)
	: QObject(parent),
	  localTable(localTable),
	  remoteTable(remoteTable),
	  localPageLabel(localPageLabel),
	  remotePageLabel(remotePageLabel),
	  localDataSource(localDataSource),
	  remoteDataSource(remoteDataSource) {
	currentPage[0] = 1;
	currentPage[1] = 1;
	totalPage[0] = 1;
	totalPage[1] = 1;
	favoriteOnly[0] = false;
	favoriteOnly[1] = false;
	connectSignals();
}

void MSTTableManager::connectSignals() const {
	connect(this, &MSTTableManager::requestScroll, this, &MSTTableManager::scrollToTop);
}

/**
 * @brief 将PathType转换为数组索引
 * @param path 路径类型
 * @return 0表示本地，1表示远程
 */
int MSTTableManager::pathIndex(const PathType path) {
	return path == PathType::LOCAL ? 0 : 1;
}

/**
 * @brief 获取指定路径对应的表格控件
 * @param path 路径类型
 * @return 对应的表格指针
 */
QTableWidget* MSTTableManager::getTable(const PathType path) const {
	return path == PathType::LOCAL ? localTable : remoteTable;
}

/**
 * @brief 获取指定路径对应的分页标签
 * @param path 路径类型
 * @return 对应的标签指针
 */
QLabel* MSTTableManager::getPageLabel(const PathType path) const {
	return path == PathType::LOCAL ? localPageLabel : remotePageLabel;
}

/**
 * @brief 获取指定路径对应的数据源
 * @param path 路径类型
 * @return 数据源引用
 */
MSTDataSource& MSTTableManager::getDataSource(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource : remoteDataSource;
}

/**
 * @brief 将QueryItem列表填充到表格
 * @param table 目标表格
 * @param items 音乐数据列表
 * @param start 起始索引
 * @param count 填充条数
 */
void MSTTableManager::fillTableWithItems(QTableWidget* table,
                                         const QList<QueryItem>& items,
                                         const int count) {
	const int safeCount = qMin(count, items.size());
	table->clearContents();
	table->setRowCount(safeCount);
	for (int i = 0; i < safeCount; i++) {
		table->setItem(i, 0, new QTableWidgetItem(items.at(i).getTitle()));
		table->setItem(i, 1, new QTableWidgetItem(items.at(i).getArtist()));
		table->setItem(i, 2, new QTableWidgetItem(items.at(i).getAlbum()));
		table->setItem(i, 3, new QTableWidgetItem(items.at(i).getGenre()));
		table->setItem(i, 4, new QTableWidgetItem(items.at(i).getYear() != 0 ? QString::number(items.at(i).getYear()) : ""));
		table->setItem(i, 5, new QTableWidgetItem(QString::number(items.at(i).getTrack())));
	}
}

void MSTTableManager::scrollToTop(const PathType path) const {
	QTableWidget* targetTable = getTable(path);
	QTableWidgetItem const* item = targetTable->item(0, 0);
	targetTable->scrollToItem(item, QAbstractItemView::PositionAtTop);
}


/**
 * @brief 搜索音乐并填充表格
 * @param path 路径类型（本地或远程）
 * @param text 搜索关键字
 */
void MSTTableManager::searchMusic(const PathType path, const QString& text) {
	if (text.isEmpty()) {
		emit requestLoadMusic(path, 1);
		return;
	}
	MSTDataSource& ds = getDataSource(path);
	QTableWidget* targetTable = getTable(path);
	targetTable->clearContents();
	const QList<QueryItem> items = ds.searchMusic(text);
	targetTable->setRowCount(items.count());
	for (int i = 0; i < items.count(); i++) {
		targetTable->setItem(i, 0, new QTableWidgetItem(items.at(i).getTitle()));
		targetTable->setItem(i, 1, new QTableWidgetItem(items.at(i).getArtist()));
		targetTable->setItem(i, 2, new QTableWidgetItem(items.at(i).getAlbum()));
		targetTable->setItem(i, 3, new QTableWidgetItem(items.at(i).getGenre()));
		targetTable->setItem(i, 4, new QTableWidgetItem(QString::number(items.at(i).getYear())));
		targetTable->setItem(i, 5, new QTableWidgetItem(QString::number(items.at(i).getTrack())));
	}
}

/**
 * @brief 获取表格中选中的音乐文件名
 * @param path 路径类型（本地或远程）
 * @return 选中音乐文件的文件名列表
 */
QStringList MSTTableManager::getSelectedMusic(const PathType path) const {
	const QTableWidget* const table = getTable(path);
	if (table->rowCount() == 0) {
		return {};
	}
	QSet<int> selectedRows;
	for (int i = 0; i < table->rowCount(); i++) {
		if (table->item(i, 0)->isSelected()) {
			selectedRows.insert(i);
		}
	}
	if (selectedRows.empty()) {
		return {};
	}
	QStringList titleList;
	QStringList artistList;
	QStringList albumList;
	for (const int& i : selectedRows) {
		titleList.append(table->item(i, 0)->text());
		artistList.append(table->item(i, 1)->text());
		albumList.append(table->item(i, 2)->text());
	}
	MSTDataSource& ds = getDataSource(path);
	QList<QueryItem> items;
	QueryItem item;
	for (int i = 0; i < titleList.count(); i++) {
		item.setTitle(titleList.at(i));
		item.setArtist(artistList.at(i));
		item.setAlbum(albumList.at(i));
		items.append(item);
	}
	return ds.getFileNameByMD(items);
}

/**
 * @brief 加载收藏音乐到表格
 * @param path 路径类型（本地或远程）
 * @param page 页码
 * @param favoriteTag 收藏标签关键字
 * @param sortBy 排序字段
 * @param orderBy 排序方向
 */
void MSTTableManager::getFavoriteMusic(const PathType path, const unsigned short page,
                                       const QString& favoriteTag,
                                       const PROPERTIES::SortByEnum sortBy,
                                       const PROPERTIES::OrderByEnum orderBy) {
	if (getDataSource(path).getPath().isEmpty()) {
		emit errorOccurred(PET::NPS);
		return;
	}
	if (favoriteTag.isEmpty()) {
		emit errorOccurred(PET::NFT);
		return;
	}

	const int idx = pathIndex(path);
	currentPage[idx] = page;

	MSTDataSource& ds = getDataSource(path);
	const auto fileList = ds.getFavorite(page, sortBy, orderBy);
	const int trueTotal = ds.getLastFavoriteCount();
	totalPage[idx] = trueTotal > 0
		? static_cast<unsigned short>((trueTotal + PAGESIZE - 1) / PAGESIZE)
		: 1;

	QTableWidget* targetTable = getTable(path);
	getPageLabel(path)->setText(
		QString::number(currentPage[idx]) + "/" + QString::number(totalPage[idx]));

	qsizetype rowSize;
	if (currentPage[idx] == totalPage[idx]) {
		rowSize = trueTotal % PAGESIZE == 0 ? PAGESIZE : trueTotal % PAGESIZE;
	} else {
		rowSize = PAGESIZE;
	}
	favoriteOnly[idx] = true;
	fillTableWithItems(targetTable, fileList, static_cast<int>(rowSize));
	emit requestScroll(path);
}

/**
 * @brief 填充音乐列表到表格
 * @param path 路径类型（本地或远程）
 * @param items 音乐数据列表
 * @param page 当前页码
 * @param totalPages 总页数
 */
void MSTTableManager::populateTable(const PathType path, const QList<QueryItem>& items,
                                    const unsigned short page, const unsigned short totalPages) {
	const int idx = pathIndex(path);
	currentPage[idx] = page;
	totalPage[idx] = totalPages;
	favoriteOnly[idx] = false;

	QTableWidget* targetTable = getTable(path);
	getPageLabel(path)->setText(
		QString::number(currentPage[idx]) + "/" + QString::number(totalPage[idx]));

	const qsizetype totalSize = items.size();
	const qsizetype lastPageSize = totalSize % PAGESIZE;
	qsizetype rowSize;
	if (currentPage[idx] == totalPage[idx]) {
		rowSize = lastPageSize;
	} else {
		rowSize = PAGESIZE;
	}
	fillTableWithItems(targetTable, items, static_cast<int>(rowSize));
}

/**
 * @brief 尝试翻到上一页
 * @param path 路径类型（本地或远程）
 * @param hasPath 是否已选择路径
 * @param favoriteTag 收藏标签关键字
 * @param sortBy 排序字段
 * @param orderBy 排序方向
 */
void MSTTableManager::goToPrevPage(const PathType path, const bool hasPath,
                                   const QString& favoriteTag,
                                   const PROPERTIES::SortByEnum sortBy,
                                   const PROPERTIES::OrderByEnum orderBy) {
	if (!hasPath) {
		emit errorOccurred(PET::NPS);
		return;
	}
	const int idx = pathIndex(path);
	if (currentPage[idx] == 1) {
		emit errorOccurred(PET::FIRST);
		return;
	}
	--currentPage[idx];
	if (favoriteOnly[idx]) {
		emit requestLoadFavorite(path, currentPage[idx]);
	} else {
		emit requestLoadMusic(path, currentPage[idx]);
	}
}

/**
 * @brief 尝试翻到下一页
 * @param path 路径类型（本地或远程）
 * @param hasPath 是否已选择路径
 * @param favoriteTag 收藏标签关键字
 * @param sortBy 排序字段
 * @param orderBy 排序方向
 */
void MSTTableManager::goToNextPage(const PathType path, const bool hasPath,
                                   const QString& favoriteTag,
                                   const PROPERTIES::SortByEnum sortBy,
                                   const PROPERTIES::OrderByEnum orderBy) {
	if (!hasPath) {
		emit errorOccurred(PET::NPS);
		return;
	}
	const int idx = pathIndex(path);
	if (currentPage[idx] == totalPage[idx]) {
		emit errorOccurred(PET::LAST);
		return;
	}
	++currentPage[idx];
	if (favoriteOnly[idx]) {
		emit requestLoadFavorite(path, currentPage[idx]);
	} else {
		emit requestLoadMusic(path, currentPage[idx]);
	}
}

/// @brief 获取指定路径的当前页码
unsigned short MSTTableManager::getCurrentPage(const PathType path) const {
	return currentPage[pathIndex(path)];
}

/// @brief 设置指定路径的当前页码
void MSTTableManager::setCurrentPage(const PathType path, const unsigned short page) {
	currentPage[pathIndex(path)] = page;
}

/// @brief 获取指定路径的总页数
unsigned short MSTTableManager::getTotalPage(const PathType path) const {
	return totalPage[pathIndex(path)];
}

/// @brief 设置指定路径的总页数
void MSTTableManager::setTotalPage(const PathType path, const unsigned short total) {
	totalPage[pathIndex(path)] = total;
}

/// @brief 获取指定路径是否处于收藏过滤模式
bool MSTTableManager::isFavoriteOnly(const PathType path) const {
	return favoriteOnly[pathIndex(path)];
}

/// @brief 设置指定路径的收藏过滤模式
void MSTTableManager::setFavoriteOnly(const PathType path, const bool value) {
	favoriteOnly[pathIndex(path)] = value;
}
