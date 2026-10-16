#include "MSTDataSource.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSqlError>
#include <QThread>
#include <taglib/fileref.h>
#include "Logger.h"
#include "../Core/MSTTagUtils.h"

namespace {
	/// @brief 当前数据库 schema 版本；结构变更时递增并补充对应迁移步骤
	constexpr int schemaVersion = 1;
}

QSqlQuery& MSTDataSource::threadQuery() const {
	QMutexLocker locker(&threadQueryMutex);
	QSqlQuery*& query = threadQueries[QThread::currentThreadId()];
	if (query == nullptr) {
		// 每线程独立查询对象：QSqlDatabase 连接由 dbMutex 串行化，
		// 但 QSqlQuery 本身不可跨线程共享
		query = new QSqlQuery(db);
	}
	return *query;
}

void MSTDataSource::releaseThreadQueries() {
	QMutexLocker locker(&threadQueryMutex);
	for (QSqlQuery* query : threadQueries) {
		delete query;
	}
	threadQueries.clear();
}

MSTDataSource::~MSTDataSource() {
	releaseThreadQueries();
}

int MSTDataSource::getPageSize() const { return pageSize; }

void MSTDataSource::setPageSize(const int page_size) { pageSize = page_size; }

MSTDataSource::MSTDataSource(const QString& path) { this->path = path; }

void MSTDataSource::setPath(const QString& path_) { this->path = path_; }

void MSTDataSource::setConnectionName(const QString& connectionName) { connection = connectionName; }

bool MSTDataSource::openDatabaseFile(const QString& dir) {
	// 连接名固定：重复 addDatabase 会丢弃既有连接，使已创建的查询对象失效，
	// 因此复用同一连接，仅切换数据库文件
	if (!QSqlDatabase::contains(connection)) {
		db = QSqlDatabase::addDatabase("QSQLITE", connection);
	}
	else {
		db = QSqlDatabase::database(connection, false);
		db.close();
	}

	db.setDatabaseName(QDir(dir).filePath("musicInfo.db"));

	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	return true;
}

bool MSTDataSource::openDB(const QString& path_) {
	QMutexLocker locker(&dbMutex);
	return openDatabaseFile(path_);
}

bool MSTDataSource::openDB() {
	QMutexLocker locker(&dbMutex);
	return openDatabaseFile(path);
}

void MSTDataSource::initTable() {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	if (!query.exec("CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
		"track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)")) {
		Logger::Error("Error creating music table:" + query.lastError().text());
		return;
	}

	const int version = userVersion();
	if (version >= schemaVersion) {
		return;
	}
	// v0 -> v1：历史库可能缺少后续版本新增的列，逐列补齐（幂等）
	bool migrated = true;
	migrated &= ensureColumn("musicInfo", "title", "TEXT");
	migrated &= ensureColumn("musicInfo", "artist", "TEXT");
	migrated &= ensureColumn("musicInfo", "album", "TEXT");
	migrated &= ensureColumn("musicInfo", "genre", "TEXT");
	migrated &= ensureColumn("musicInfo", "year", "INT");
	migrated &= ensureColumn("musicInfo", "track", "INT");
	migrated &= ensureColumn("musicInfo", "favorite", "BOOL");
	migrated &= ensureColumn("musicInfo", "ruleHit", "BOOL");
	migrated &= ensureColumn("musicInfo", "fileName", "TEXT");
	if (!migrated) {
		// 不写入版本号：下次启动会重试补齐，避免旧库被标记成已迁移却缺列
		Logger::Warn("Music database schema migration incomplete, will retry on next launch");
		return;
	}
	setUserVersion(schemaVersion);
	Logger::Info("Music database schema migrated to version " + QString::number(schemaVersion));
}

int MSTDataSource::userVersion() {
	QSqlQuery& query = threadQuery();
	if (!query.exec("PRAGMA user_version") || !query.next()) {
		Logger::Warn("Failed to read schema version:" + query.lastError().text());
		return 0;
	}
	return query.value(0).toInt();
}

void MSTDataSource::setUserVersion(const int version) {
	QSqlQuery& query = threadQuery();
	// PRAGMA 不支持绑定参数；版本号来自本文件常量，不存在注入面
	if (!query.exec("PRAGMA user_version = " + QString::number(version))) {
		Logger::Warn("Failed to write schema version:" + query.lastError().text());
	}
}

bool MSTDataSource::ensureColumn(const QString& tableName, const QString& columnName, const QString& definition) {
	QSqlQuery& query = threadQuery();
	query.prepare("PRAGMA table_info(" + tableName + ")");
	if (!query.exec()) {
		Logger::Error("Failed to read table info:" + query.lastError().text());
		return false;
	}
	bool columnExists = false;
	while (query.next()) {
		if (query.value(1).toString().compare(columnName, Qt::CaseInsensitive) == 0) {
			columnExists = true;
			break;
		}
	}
	if (columnExists) {
		return true;
	}
	if (!query.exec("ALTER TABLE " + tableName + " ADD COLUMN " + columnName + " " + definition)) {
		Logger::Error("Failed to add column " + columnName + ":" + query.lastError().text());
		return false;
	}
	Logger::Info("Schema migration: added column " + columnName);
	return true;
}

void MSTDataSource::closeDB() {
	QMutexLocker locker(&dbMutex);
	// 先丢弃各线程缓存的查询对象：连接关闭后它们不再可用，留着会被后续实例误用
	releaseThreadQueries();
	if (db.isOpen()) {
		db.close();
	}
}

void MSTDataSource::prepareStatement(const QString& sqlStatement) {
	QMutexLocker locker(&dbMutex);
	threadQuery().prepare(sqlStatement);
}

bool MSTDataSource::execQuery() {
	QMutexLocker locker(&dbMutex);
	return threadQuery().exec();
}

void MSTDataSource::updateFavoriteBatch(const QList<FavoriteUpdate>& updates) {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	if (!query.exec("BEGIN TRANSACTION")) {
		Logger::Error("Error beginning favorite transaction:" + query.lastError().text());
		return;
	}
	for (const auto& update : updates) {
		query.prepare("UPDATE musicInfo SET favorite = :fav WHERE fileName = :fileName");
		query.bindValue(":fav", update.isFavorite);
		query.bindValue(":fileName", update.fileName);
		if (!query.exec()) {
			Logger::Error("Error updating favorite state:" + query.lastError().text());
			query.exec("ROLLBACK");
			return;
		}
	}
	if (!query.exec("COMMIT")) {
		Logger::Error("Error committing favorite updates:" + query.lastError().text());
		query.exec("ROLLBACK");
	}
}

void MSTDataSource::updateRuleHitBatch(const QList<RuleHitUpdate>& updates) {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	if (!query.exec("BEGIN TRANSACTION")) {
		Logger::Error("Error beginning rule hit transaction:" + query.lastError().text());
		return;
	}
	for (const auto& update : updates) {
		query.prepare("UPDATE musicInfo SET ruleHit = :ruleHit WHERE fileName = :fileName");
		query.bindValue(":ruleHit", update.isRuleHit);
		query.bindValue(":fileName", update.fileName);
		if (!query.exec()) {
			Logger::Error("Error updating rule hit state:" + query.lastError().text());
			query.exec("ROLLBACK");
			return;
		}
	}
	if (!query.exec("COMMIT")) {
		Logger::Error("Error committing rule hit updates:" + query.lastError().text());
		query.exec("ROLLBACK");
	}
}

/**
 * @details 使用 WHERE fileName IN (...) 让数据库做过滤，
 *          避免全表加载后再内存过滤；文件数超过单批占位符上限时按批查询
 */
QList<QueryItem> MSTDataSource::getByFileNames(const QStringList& fileNames) {
	if (fileNames.isEmpty()) {
		return {};
	}
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();

	// SQLITE_MAX_VARIABLE_NUMBER 旧版本为 999，按 500 分批留出余量
	constexpr qsizetype batchSize = 500;
	QList<QueryItem> items;
	for (qsizetype offset = 0; offset < fileNames.size(); offset += batchSize) {
		const QStringList batch = fileNames.mid(offset, batchSize);

		// 构建参数化 IN 子句：WHERE fileName IN (:f0, :f1, ...)
		QString sql = "SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE fileName IN (";
		QStringList paramNames;
		for (qsizetype i = 0; i < batch.size(); i++) {
			paramNames << ":f" + QString::number(i);
		}
		sql += paramNames.join(",") + ") ORDER BY title ASC";

		query.prepare(sql);
		for (qsizetype i = 0; i < batch.size(); i++) {
			query.bindValue(":f" + QString::number(i), batch.at(i));
		}
		if (!query.exec()) {
			Logger::Error("Error querying by file names:" + query.lastError().text());
			continue;
		}

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

	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare(sql);
	if (!query.exec()) {
		Logger::Error("Error querying all music:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> items;
	while (query.next()) {
		// 每条记录使用独立对象，避免上一行的残留字段串入下一行
		QueryItem currentRow;
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

/**
 * @details 单文件写入：读取标签并插入一条记录，供低层调用与测试使用
 */
bool MSTDataSource::insertMusic(const QString& file) {
	const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(QDir(path).filePath(file));

	if (fileRef.isNull()) {
		return false;
	}

	const TagLib::Tag* tag = fileRef.tag();

	QSqlQuery& query = threadQuery();
	query.prepare("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
		":artist, :album, :genre, :year, :track, :fileName)");
	query.bindValue(":title", QString::fromUtf8(tag->title().to8Bit(true)));
	query.bindValue(":artist", QString::fromUtf8(tag->artist().to8Bit(true)));
	query.bindValue(":album", QString::fromUtf8(tag->album().to8Bit(true)));
	query.bindValue(":genre", QString::fromUtf8(tag->genre().to8Bit(true)));
	query.bindValue(":year", QString::number(tag->year()));
	query.bindValue(":track", QString::number(tag->track()));
	query.bindValue(":fileName", file);

	if (!query.exec()) {
		Logger::Warn("Error inserting data into database: " + query.lastError().text());
		return false;
	}
	return true;
}

bool MSTDataSource::addMusicBatch(const QStringList& files, QStringList& failedFiles) {
	if (files.isEmpty()) {
		return true;
	}
	// 整批写入在同一把锁内完成，其他线程的语句不会混入事务边界
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();

	if (!query.exec("BEGIN TRANSACTION")) {
		Logger::Error("Error beginning insert transaction:" + query.lastError().text());
		return false;
	}

	for (const QString& file : files) {
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(QDir(path).filePath(file));
		if (fileRef.isNull() || fileRef.tag() == nullptr) {
			failedFiles.append(file);
			Logger::Warn("Failed to read tags of file: " + file);
			continue;
		}
		const TagLib::Tag* tag = fileRef.tag();
		query.prepare("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
			":artist, :album, :genre, :year, :track, :fileName)");
		query.bindValue(":title", QString::fromUtf8(tag->title().to8Bit(true)));
		query.bindValue(":artist", QString::fromUtf8(tag->artist().to8Bit(true)));
		query.bindValue(":album", QString::fromUtf8(tag->album().to8Bit(true)));
		query.bindValue(":genre", QString::fromUtf8(tag->genre().to8Bit(true)));
		query.bindValue(":year", QString::number(tag->year()));
		query.bindValue(":track", QString::number(tag->track()));
		query.bindValue(":fileName", file);
		if (!query.exec()) {
			failedFiles.append(file);
			Logger::Warn("Error inserting data into database: " + query.lastError().text());
		}
	}

	if (!query.exec("COMMIT")) {
		Logger::Error("Error committing inserted data:" + query.lastError().text());
		query.exec("ROLLBACK");
		return false;
	}
	return true;
}

QStringList MSTDataSource::addMusic(const QStringList& files) {
	QStringList failedFileList;
	// 复用批量写入路径，保证事务与持锁语义一致
	if (!addMusicBatch(files, failedFileList)) {
		return files;
	}
	return failedFileList;
}

bool MSTDataSource::addMusic(const QString& file) {
	if (file.isEmpty()) {
		return false;
	}
	QMutexLocker locker(&dbMutex);
	return insertMusic(file);
}

int MSTDataSource::getCount() {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare("SELECT COUNT(*) FROM musicInfo");
	if (!query.exec() || !query.next()) {
		Logger::Error("Error counting music:" + query.lastError().text());
		return 0;
	}
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

	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare(sql);
	if (!query.exec()) {
		Logger::Error("Error querying page data:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> result;
	while (query.next()) {
		QueryItem item;
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

/**
 * @details 转义 LIKE 模式中的通配符：% 与 _ 必须按字面匹配，
 *          反斜杠本身也要转义；配合 SQL 的 ESCAPE '\' 使用
 */
QString MSTDataSource::escapeLikePattern(const QString& text) {
	QString escaped = text;
	escaped.replace("\\", "\\\\");
	escaped.replace("%", "\\%");
	escaped.replace("_", "\\_");
	return escaped;
}

QList<QueryItem> MSTDataSource::searchMusic(const QString& text) {
	if (text.isEmpty()) {
		// 空关键字不返回整库结果，与表格层的处理保持一致
		return {};
	}
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title LIKE '%' || :text || '%' ESCAPE '\\' OR "
		"artist LIKE '%' || :text || '%' ESCAPE '\\' OR album LIKE '%' || :text || '%' ESCAPE '\\'");
	query.bindValue(":text", escapeLikePattern(text));
	if (!query.exec()) {
		Logger::Error("Error searching music:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
		                       query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(),
		                       query.value(6).toString()));
	}
	return items;
}

int MSTDataSource::getSearchCount(const QString& text) {
	if (text.isEmpty()) {
		return 0;
	}
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare("SELECT COUNT(*) FROM musicInfo WHERE title LIKE '%' || :text || '%' ESCAPE '\\' OR "
		"artist LIKE '%' || :text || '%' ESCAPE '\\' OR album LIKE '%' || :text || '%' ESCAPE '\\'");
	query.bindValue(":text", escapeLikePattern(text));
	if (!query.exec() || !query.next()) {
		Logger::Error("Error counting search results:" + query.lastError().text());
		return 0;
	}
	return query.value(0).toInt();
}

QList<QueryItem> MSTDataSource::searchMusic(const QString& text, const unsigned short pageNum) {
	if (text.isEmpty()) {
		return {};
	}
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title LIKE '%' || :text || '%' ESCAPE '\\' OR "
		"artist LIKE '%' || :text || '%' ESCAPE '\\' OR album LIKE '%' || :text || '%' ESCAPE '\\' "
		"ORDER BY title ASC LIMIT :limit OFFSET :offset");
	query.bindValue(":text", escapeLikePattern(text));
	query.bindValue(":limit", pageSize);
	query.bindValue(":offset", (pageNum - 1) * pageSize);
	if (!query.exec()) {
		Logger::Error("Error searching music page:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
		                       query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(),
		                       query.value(6).toString()));
	}
	return items;
}

QStringList MSTDataSource::getFileNameByMetadata(const QList<QueryItem>& items) {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	QStringList fileList;
	for (const QueryItem& item : items) {
		query.prepare("SELECT fileName FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album");
		query.bindValue(":title", item.getTitle());
		query.bindValue(":artist", item.getArtist());
		query.bindValue(":album", item.getAlbum());
		if (!query.exec()) {
			Logger::Error("Error querying file name by metadata:" + query.lastError().text());
			continue;
		}
		// 元数据可能对应多条记录：全部返回，避免重复项被静默吞掉
		while (query.next()) {
			fileList.append(query.value(0).toString());
		}
	}
	return fileList;
}

QList<QueryItem> MSTDataSource::getFavorite(const unsigned short pageNum, const SortByEnum sortBy,
                                            const OrderByEnum orderBy) {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();

	// 获取收藏音乐的真实总数（不带LIMIT，用于分页计算）
	query.prepare("SELECT COUNT(*) FROM musicInfo WHERE favorite = 1");
	if (!query.exec() || !query.next()) {
		Logger::Error("Error counting favorite music:" + query.lastError().text());
		return {};
	}
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

	query.prepare(sql);
	if (!query.exec()) {
		Logger::Error("Error querying favorite music:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> items;
	int i = 0;
	while (query.next()) {
		QueryItem item;
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

	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare(sql);
	if (!query.exec()) {
		Logger::Error("Error querying rule hit music:" + query.lastError().text());
		return {};
	}

	QList<QueryItem> items;
	while (query.next()) {
		QueryItem item;
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

	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	// 使用std::all_of检查所有文件是否都删除成功
	return std::all_of(fileList.begin(), fileList.end(), [&query, this](const QString& file) {
		query.prepare("DELETE FROM musicInfo WHERE fileName = :fileName");
		query.bindValue(":fileName", file);
		if (!query.exec()) {
			Logger::Error("Error deleting data from database:" + query.lastError().text());
			return false;
		}
		return true;
	});
}

bool MSTDataSource::getRuleHit(const QString& fileName) {
	QMutexLocker locker(&dbMutex);
	QSqlQuery& query = threadQuery();
	query.prepare("SELECT ruleHit FROM musicInfo WHERE fileName = :fileName");
	query.bindValue(":fileName", fileName);
	if (!query.exec()) {
		Logger::Error("Error reading rule hit state:" + query.lastError().text());
		return false;
	}

	if (query.next()) {
		const QVariant value = query.value(0);
		return !value.isNull() && value.toBool(); // 如果规则命中状态不为NULL且为true，返回true
	}
	return false; // 如果没有找到记录，默认返回false
}
