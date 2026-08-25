#include "MSTDataSource.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSqlError>
#include <taglib/fileref.h>
#include "Logger.h"
#include "../Core/MSTTagUtils.h"

int MSTDataSource::getPageSize() const { return pageSize; }

void MSTDataSource::setPageSize(const int page_size) { pageSize = page_size; }

MSTDataSource::MSTDataSource(const QString& path) { this->path = path; }

void MSTDataSource::setPath(const QString& path_) { this->path = path_; }

void MSTDataSource::setConnectionName(const QString& connectionName) { connection = connectionName; }

bool MSTDataSource::openDB(const QString& path_) {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);

	db.setDatabaseName(QDir(path_).filePath("musicInfo.db"));

	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	query = QSqlQuery(db);
	return true;
}

bool MSTDataSource::openDB() {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);

	db.setDatabaseName(QDir(path).filePath("musicInfo.db"));

	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	query = QSqlQuery(db);
	return true;
}

void MSTDataSource::initTable() {
	query.exec("CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
		"track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)");
}

void MSTDataSource::closeDB() {
	if (db.isOpen()) {
		db.close();
	}
}

void MSTDataSource::prepareStatement(const QString& sqlStatement) { query.prepare(sqlStatement); }

void MSTDataSource::execQuery() { query.exec(); }

void MSTDataSource::updateFavoriteBatch(const QList<FavoriteUpdate>& updates) {
	query.exec("BEGIN TRANSACTION");
	for (const auto& update : updates) {
		prepareStatement("UPDATE musicInfo SET favorite = :fav WHERE fileName = :fileName");
		bindValue(":fav", update.isFavorite);
		bindValue(":fileName", update.fileName);
		execQuery();
	}
	query.exec("COMMIT");
}

void MSTDataSource::updateRuleHitBatch(const QList<RuleHitUpdate>& updates) {
	query.exec("BEGIN TRANSACTION");
	for (const auto& update : updates) {
		prepareStatement("UPDATE musicInfo SET ruleHit = :ruleHit WHERE fileName = :fileName");
		bindValue(":ruleHit", update.isRuleHit);
		bindValue(":fileName", update.fileName);
		execQuery();
	}
	query.exec("COMMIT");
}

void MSTDataSource::beginTransaction() {
	query.exec("BEGIN TRANSACTION");
}

void MSTDataSource::commitTransaction() {
	query.exec("COMMIT");
}

/**
 * @details 使用 WHERE fileName IN (...) 让数据库做过滤，
 *          避免全表加载后再内存过滤
 */
QList<QueryItem> MSTDataSource::getByFileNames(const QStringList& fileNames) {
	if (fileNames.isEmpty()) {
		return {};
	}

	// 构建参数化 IN 子句：WHERE fileName IN (:f0, :f1, ...)
	QString sql = "SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE fileName IN (";
	QStringList paramNames;
	for (int i = 0; i < fileNames.size(); i++) {
		paramNames << ":f" + QString::number(i);
	}
	sql += paramNames.join(",") + ") ORDER BY title ASC";

	prepareStatement(sql);
	for (int i = 0; i < fileNames.size(); i++) {
		bindValue(":f" + QString::number(i), fileNames.at(i));
	}
	execQuery();

	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(
			query.value(0).toString(),
			query.value(1).toString(),
			query.value(2).toString(),
			query.value(3).toString(),
			query.value(4).toUInt(),
			query.value(5).toUInt(),
			query.value(6).toString()
		));
	}
	return items;
}

QList<QueryItem> MSTDataSource::getAll(const QVector<QueryRows>& rows) {
	// 如果使用了 ALL 枚举，则禁用其他枚举同时出现
	QVector<QueryRows> effectiveRows = rows;
	if (rows.contains(QueryRows::ALL)) {
		effectiveRows = {QueryRows::ALL};
	}

	QString sql = "SELECT ";

	// 根据查询行类型构建SELECT语句
	for (const auto& row : effectiveRows) {
		switch (row) {
		case QueryRows::TITLE:
			sql += "title, ";
			break;
		case QueryRows::ARTIST:
			sql += "artist, ";
			break;
		case QueryRows::ALBUM:
			sql += "album, ";
			break;
		case QueryRows::GENRE:
			sql += "genre, ";
			break;
		case QueryRows::YEAR:
			sql += "year, ";
			break;
		case QueryRows::TRACK:
			sql += "track, ";
			break;
		case QueryRows::FILENAME:
			sql += "fileName, ";
			break;
		case QueryRows::ALL:
			sql += "title, artist, album, genre, year, track, fileName, ";
			break;
		}
	}

	// 创建行映射，用于后续数据解析
	QMap<int, QueryRows> rowMap;
	for (int i = 0; i < effectiveRows.size(); ++i) {
		rowMap.insert(i, effectiveRows[i]);
	}

	sql.chop(2);
	sql += " FROM musicInfo ORDER BY title ASC";

	prepareStatement(sql);
	execQuery();

	QList<QueryItem> items;
	QueryItem currentRow;
	while (query.next()) {
		for (int i = 0; i < rowMap.size(); i++) {
			switch (rowMap[i]) {
			case QueryRows::TITLE:
				currentRow.setTitle(query.value(i).toString());
				break;
			case QueryRows::ARTIST:
				currentRow.setArtist(query.value(i).toString());
				break;
			case QueryRows::ALBUM:
				currentRow.setAlbum(query.value(i).toString());
				break;
			case QueryRows::GENRE:
				currentRow.setGenre(query.value(i).toString());
				break;
			case QueryRows::YEAR:
				currentRow.setYear(query.value(i).toUInt());
				break;
			case QueryRows::TRACK:
				currentRow.setTrack(query.value(i).toUInt());
				break;
			case QueryRows::FILENAME:
				currentRow.setFileName(query.value(i).toString());
				break;
			case QueryRows::ALL:
				// 查询所有字段时，按固定顺序设置值
				currentRow.setTitle(query.value(0).toString());
				currentRow.setArtist(query.value(1).toString());
				currentRow.setAlbum(query.value(2).toString());
				currentRow.setGenre(query.value(3).toString());
				currentRow.setYear(query.value(4).toUInt());
				currentRow.setTrack(query.value(5).toUInt());
				currentRow.setFileName(query.value(6).toString());
				break;
			}
		}
		items.append(currentRow);
	}
	return items;
}

QStringList MSTDataSource::addMusic(const QStringList& files) {
	QStringList failedFileList;

	for (const QString& file : files) {
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(path + "/" + file);

		if (fileRef.isNull()) {
			failedFileList.append(file);
			Logger::Warn("Failed to add file" + file);
			continue;
		}

		const TagLib::Tag* tag = fileRef.tag();

		prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
			":artist, :album, :genre, :year, :track, :fileName)");
		bindValue(":title", QString::fromUtf8(tag->title().to8Bit(true)));
		bindValue(":artist", QString::fromUtf8(tag->artist().to8Bit(true)));
		bindValue(":album", QString::fromUtf8(tag->album().to8Bit(true)));
		bindValue(":genre", QString::fromUtf8(tag->genre().to8Bit(true)));
		bindValue(":year", QString::number(tag->year()));
		bindValue(":track", QString::number(tag->track()));
		bindValue(":fileName", file);

		if (!query.exec()) {
			Logger::Warn("Error inserting data into database: " + query.lastError().text());
			failedFileList.append(file);
		}
	}
	return failedFileList;
}

bool MSTDataSource::addMusic(const QString& file) {
	if (file.isEmpty()) {
		return false;
	}

	const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(path + "/" + file);

	if (fileRef.isNull()) {
		return false;
	}

	const TagLib::Tag* tag = fileRef.tag();

	prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
		":artist, :album, :genre, :year, :track, :fileName)");
	bindValue(":title", QString::fromUtf8(tag->title().to8Bit(true)));
	bindValue(":artist", QString::fromUtf8(tag->artist().to8Bit(true)));
	bindValue(":album", QString::fromUtf8(tag->album().to8Bit(true)));
	bindValue(":genre", QString::fromUtf8(tag->genre().to8Bit(true)));
	bindValue(":year", QString::number(tag->year()));
	bindValue(":track", QString::number(tag->track()));
	bindValue(":fileName", file);

	if (!query.exec()) {
		Logger::Warn("Error inserting data into database: " + query.lastError().text());
		return false;
	}
	return true;
}

int MSTDataSource::getCount() {
	prepareStatement("SELECT COUNT(*) FROM musicInfo");
	execQuery();
	query.next();
	return query.value(0).toInt();
}

QList<QueryItem> MSTDataSource::getMusicToTable(const unsigned short pageNum, const SortByEnum sortBy,
                                                const OrderByEnum orderBy) {
	QString sql = "SELECT title, artist, album, genre, year, track FROM musicInfo ORDER BY";

	// 根据排序字段添加ORDER BY子句
	switch (sortBy) {
	case SortByEnum::TITLE:
		sql += " TITLE ";
		break;
	case SortByEnum::ARTIST:
		sql += " ARTIST ";
		break;
	case SortByEnum::ALBUM:
		sql += " ALBUM ";
		break;
	}

	// 根据排序顺序添加ASC/DESC
	switch (orderBy) {
	case OrderByEnum::ASC:
		sql += "ASC ";
		break;
	case OrderByEnum::DESC:
		sql += "DESC ";
		break;
	}

	sql += "LIMIT " + QString::number(pageSize) + " OFFSET " + QString::number((pageNum - 1) * pageSize);

	prepareStatement(sql);
	execQuery();

	QList<QueryItem> result;
	auto item = QueryItem();
	while (query.next()) {
		item.setTitle(query.value(0).toString());
		item.setArtist(query.value(1).toString());
		item.setAlbum(query.value(2).toString());
		item.setGenre(query.value(3).toString());
		item.setYear(query.value(4).toInt());
		item.setTrack(query.value(5).toInt());
		result.append(item);
	}
	return result;
}

QList<QueryItem> MSTDataSource::searchMusic(const QString& text) {
	prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title LIKE '%' || :text || '%' OR "
		"artist LIKE '%' || :text || '%' OR album LIKE '%' || :text || '%'");
	bindValue(":text", text);
	execQuery();

	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
		                       query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(),
		                       query.value(6).toString()));
	}
	return items;
}

QStringList MSTDataSource::getFileNameByMetadata(const QList<QueryItem>& items) {
	QStringList fileList;
	for (const QueryItem& item : items) {
		prepareStatement("SELECT fileName FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album");
		bindValue(":title", item.getTitle());
		bindValue(":artist", item.getArtist());
		bindValue(":album", item.getAlbum());
		execQuery();
		query.next();
		fileList.append(query.value(0).toString());
	}
	return fileList;
}

QList<QueryItem> MSTDataSource::getFavorite(const unsigned short pageNum, const SortByEnum sortBy,
                                            const OrderByEnum orderBy) {
	// emit loadStarted();

	// 获取收藏音乐的真实总数（不带LIMIT，用于分页计算）
	prepareStatement("SELECT COUNT(*) FROM musicInfo WHERE favorite = 1");
	execQuery();
	query.next();
	const int size = query.value(0).toInt();
	lastFavoriteCount = size;
	emit totalSize(size);

	QString sql = "SELECT title, artist, album, genre, year, track FROM musicInfo WHERE favorite = 1 ORDER BY";
	switch (sortBy) {
	case SortByEnum::TITLE:
		sql += " TITLE ";
		break;
	case SortByEnum::ARTIST:
		sql += " ARTIST ";
		break;
	case SortByEnum::ALBUM:
		sql += " ALBUM ";
		break;
	}
	switch (orderBy) {
	case OrderByEnum::ASC:
		sql += "ASC ";
		break;
	case OrderByEnum::DESC:
		sql += "DESC ";
		break;
	}
	sql += "LIMIT " + QString::number(pageSize) + " OFFSET " + QString::number((pageNum - 1) * pageSize);

	prepareStatement(sql);
	execQuery();

	QList<QueryItem> items;
	QueryItem item;
	int i = 0;
	while (query.next()) {
		emit currentProgress(i);
		item.setTitle(query.value(0).toString());
		item.setArtist(query.value(1).toString());
		item.setAlbum(query.value(2).toString());
		item.setGenre(query.value(3).toString());
		item.setYear(query.value(4).toInt());
		item.setTrack(query.value(5).toInt());
		items.append(item);
		i++;
	}
	// emit loadFinished();
	return items;
}

QList<QueryItem> MSTDataSource::getRuleHit(const unsigned short pageNum, const SortByEnum sortBy,
                                           const OrderByEnum orderBy) {
	QString sql = "SELECT title, artist, album, genre, year, track FROM musicInfo WHERE ruleHit = 1 ORDER BY";
	switch (sortBy) {
	case SortByEnum::TITLE:
		sql += " TITLE ";
		break;
	case SortByEnum::ARTIST:
		sql += " ARTIST ";
		break;
	case SortByEnum::ALBUM:
		sql += " ALBUM ";
		break;
	}
	switch (orderBy) {
	case OrderByEnum::ASC:
		sql += "ASC ";
		break;
	case OrderByEnum::DESC:
		sql += "DESC ";
		break;
	}
	sql += "LIMIT " + QString::number(pageSize) + " OFFSET " + QString::number((pageNum - 1) * pageSize);

	prepareStatement(sql);
	execQuery();

	QList<QueryItem> items;
	QueryItem item;
	while (query.next()) {
		item.setTitle(query.value(0).toString());
		item.setArtist(query.value(1).toString());
		item.setAlbum(query.value(2).toString());
		item.setGenre(query.value(3).toString());
		item.setYear(query.value(4).toInt());
		item.setTrack(query.value(5).toInt());
		items.append(item);
	}
	return items;
}

bool MSTDataSource::deleteMusic(const QStringList& fileList) {
	if (fileList.isEmpty()) {
		return true; // 空列表视为成功
	}

	// 使用std::all_of检查所有文件是否都删除成功
	return std::all_of(fileList.begin(), fileList.end(), [this](const QString& file) {
		prepareStatement("DELETE FROM musicInfo WHERE fileName = :fileName");
		bindValue(":fileName", file);
		if (!query.exec()) {
			Logger::Error("Error deleting data from database:" + query.lastError().text());
			return false;
		}
		return true;
	});
}

bool MSTDataSource::getRuleHit(const QString& fileName) {
	prepareStatement("SELECT ruleHit FROM musicInfo WHERE fileName = :fileName");
	bindValue(":fileName", fileName);
	execQuery();

	if (query.next()) {
		QVariant value = query.value(0);
		return !value.isNull() && value.toBool(); // 如果规则命中状态不为NULL且为true，返回true
	}
	return false; // 如果没有找到记录，默认返回false
}
