#include "MSTTableManager.h"
#include "../../Core/MusicProperties.h"
#include "../../Services/MSTDataSource.h"

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

int MSTTableManager::pathIndex(const PathType path) {
	return path == PathType::LOCAL ? 0 : 1;
}

QTableWidget* MSTTableManager::getTable(const PathType path) const {
	return path == PathType::LOCAL ? localTable : remoteTable;
}

QLabel* MSTTableManager::getPageLabel(const PathType path) const {
	return path == PathType::LOCAL ? localPageLabel : remotePageLabel;
}

MSTDataSource& MSTTableManager::getDataSource(const PathType path) const {
	return path == PathType::LOCAL ? localDataSource : remoteDataSource;
}

void MSTTableManager::fillTableWithItems(QTableWidget* table,
                                         const QList<QueryItem>& items,
                                         const int count) {
	const int visibleRowCount = qMin(count, items.size());
	table->clearContents();
	table->setRowCount(visibleRowCount);
	for (int i = 0; i < visibleRowCount; i++) {
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


void MSTTableManager::searchMusic(const PathType path, const QString& text) {
	const int idx = pathIndex(path);
	if (text.isEmpty()) {
		searchText[idx].clear();
		favoriteOnly[idx] = false;
		emit requestLoadMusic(path, 1);
		return;
	}
	MSTDataSource& dataSource = getDataSource(path);
	QTableWidget* targetTable = getTable(path);
	searchText[idx] = text;
	favoriteOnly[idx] = false;
	currentPage[idx] = 1;

	const QList<QueryItem> allItems = dataSource.searchMusic(text);
	const int total = allItems.size();
	totalPage[idx] = total > 0 ? static_cast<unsigned short>((total + PAGESIZE - 1) / PAGESIZE) : 1;
	getPageLabel(path)->setText(
		QString::number(currentPage[idx]) + "/" + QString::number(totalPage[idx]));

	const int start = 0;
	const int count = qMin(static_cast<int>(PAGESIZE), total - start);
	targetTable->clearContents();
	targetTable->setRowCount(count);
	for (int i = 0; i < count; i++) {
		targetTable->setItem(i, 0, new QTableWidgetItem(allItems.at(start + i).getTitle()));
		targetTable->setItem(i, 1, new QTableWidgetItem(allItems.at(start + i).getArtist()));
		targetTable->setItem(i, 2, new QTableWidgetItem(allItems.at(start + i).getAlbum()));
		targetTable->setItem(i, 3, new QTableWidgetItem(allItems.at(start + i).getGenre()));
		targetTable->setItem(i, 4, new QTableWidgetItem(allItems.at(start + i).getYear() != 0 ? QString::number(allItems.at(start + i).getYear()) : ""));
		targetTable->setItem(i, 5, new QTableWidgetItem(QString::number(allItems.at(start + i).getTrack())));
	}
}

void MSTTableManager::loadSearchPage(const PathType path) {
	const int idx = pathIndex(path);
	MSTDataSource& dataSource = getDataSource(path);
	QTableWidget* targetTable = getTable(path);

	const QList<QueryItem> allItems = dataSource.searchMusic(searchText[idx]);
	const int total = allItems.size();
	totalPage[idx] = total > 0 ? static_cast<unsigned short>((total + PAGESIZE - 1) / PAGESIZE) : 1;
	getPageLabel(path)->setText(
		QString::number(currentPage[idx]) + "/" + QString::number(totalPage[idx]));

	const int start = (currentPage[idx] - 1) * PAGESIZE;
	const int count = qMin(static_cast<int>(PAGESIZE), total - start);
	targetTable->clearContents();
	targetTable->setRowCount(count);
	for (int i = 0; i < count; i++) {
		targetTable->setItem(i, 0, new QTableWidgetItem(allItems.at(start + i).getTitle()));
		targetTable->setItem(i, 1, new QTableWidgetItem(allItems.at(start + i).getArtist()));
		targetTable->setItem(i, 2, new QTableWidgetItem(allItems.at(start + i).getAlbum()));
		targetTable->setItem(i, 3, new QTableWidgetItem(allItems.at(start + i).getGenre()));
		targetTable->setItem(i, 4, new QTableWidgetItem(allItems.at(start + i).getYear() != 0 ? QString::number(allItems.at(start + i).getYear()) : ""));
		targetTable->setItem(i, 5, new QTableWidgetItem(QString::number(allItems.at(start + i).getTrack())));
	}
}

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
	MSTDataSource& dataSource = getDataSource(path);
	QList<QueryItem> items;
	QueryItem item;
	for (int i = 0; i < titleList.count(); i++) {
		item.setTitle(titleList.at(i));
		item.setArtist(artistList.at(i));
		item.setAlbum(albumList.at(i));
		items.append(item);
	}
	return dataSource.getFileNameByMetadata(items);
}

void MSTTableManager::getFavoriteMusic(const PathType path, const unsigned short page,
                                       const QString& favoriteTag,
                                       const PROPERTIES::SortByEnum sortBy,
                                       const PROPERTIES::OrderByEnum orderBy) {
	if (getDataSource(path).getPath().isEmpty()) {
		emit errorOccurred(AppErrorType::NO_PATH);
		return;
	}
	if (favoriteTag.isEmpty()) {
		emit errorOccurred(AppErrorType::NO_FAV_TAG);
		return;
	}

	const int idx = pathIndex(path);
	currentPage[idx] = page;

	MSTDataSource& dataSource = getDataSource(path);
	const auto fileList = dataSource.getFavorite(page, sortBy, orderBy);
	const int trueTotal = dataSource.getLastFavoriteCount();
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

void MSTTableManager::populateTable(const PathType path, const QList<QueryItem>& items,
                                    const unsigned short page, const unsigned short totalPages) {
	const int idx = pathIndex(path);
	currentPage[idx] = page;
	totalPage[idx] = totalPages;
	favoriteOnly[idx] = false;
	searchText[idx].clear();

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

void MSTTableManager::goToPrevPage(const PathType path, const bool hasPath,
                                   const QString& favoriteTag,
                                   const PROPERTIES::SortByEnum sortBy,
                                   const PROPERTIES::OrderByEnum orderBy) {
	if (!hasPath) {
		emit errorOccurred(AppErrorType::NO_PATH);
		return;
	}
	const int idx = pathIndex(path);
	if (currentPage[idx] == 1) {
		emit errorOccurred(AppErrorType::FIRST);
		return;
	}
	--currentPage[idx];
	if (!searchText[idx].isEmpty()) {
		loadSearchPage(path);
		return;
	}
	if (favoriteOnly[idx]) {
		emit requestLoadFavorite(path, currentPage[idx]);
	} else {
		emit requestLoadMusic(path, currentPage[idx]);
	}
}

void MSTTableManager::goToNextPage(const PathType path, const bool hasPath,
                                   const QString& favoriteTag,
                                   const PROPERTIES::SortByEnum sortBy,
                                   const PROPERTIES::OrderByEnum orderBy) {
	if (!hasPath) {
		emit errorOccurred(AppErrorType::NO_PATH);
		return;
	}
	const int idx = pathIndex(path);
	if (currentPage[idx] == totalPage[idx]) {
		emit errorOccurred(AppErrorType::LAST);
		return;
	}
	++currentPage[idx];
	if (!searchText[idx].isEmpty()) {
		loadSearchPage(path);
		return;
	}
	if (favoriteOnly[idx]) {
		emit requestLoadFavorite(path, currentPage[idx]);
	} else {
		emit requestLoadMusic(path, currentPage[idx]);
	}
}

unsigned short MSTTableManager::getCurrentPage(const PathType path) const {
	return currentPage[pathIndex(path)];
}

void MSTTableManager::setCurrentPage(const PathType path, const unsigned short page) {
	currentPage[pathIndex(path)] = page;
}

unsigned short MSTTableManager::getTotalPage(const PathType path) const {
	return totalPage[pathIndex(path)];
}

void MSTTableManager::setTotalPage(const PathType path, const unsigned short total) {
	totalPage[pathIndex(path)] = total;
}

bool MSTTableManager::isFavoriteOnly(const PathType path) const {
	return favoriteOnly[pathIndex(path)];
}

void MSTTableManager::setFavoriteOnly(const PathType path, const bool value) {
	favoriteOnly[pathIndex(path)] = value;
}
