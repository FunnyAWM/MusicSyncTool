/**
 * @file MSTDataSource.cpp
 * @brief 音乐同步工具数据源类的实现
 * @details 实现音乐数据库的全部CRUD操作，包括数据库连接管理、
 *          表结构初始化、音乐文件的增删查改、收藏状态管理和规则命中检测
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTDataSource.h"

#include <QFile>
#include <QSqlError>
#include <QRegularExpression>
#include <taglib/fileref.h>
#include "../Core/MSTTagUtils.h"
#include "Logger.h"

/**
 * @brief 获取分页大小
 * @return 当前分页大小
 */
int MSTDataSource::getPageSize() const { return pageSize; }

/**
 * @brief 设置分页大小
 * @param page_size 新的分页大小
 */
void MSTDataSource::setPageSize(const int page_size) { pageSize = page_size; }

/**
 * @brief 构造函数，设置数据库路径
 * @param path 数据库文件路径
 */
MSTDataSource::MSTDataSource(const QString& path) { this->path = path; }

/**
 * @brief 设置数据库路径
 * @param path_ 新的数据库路径
 */
void MSTDataSource::setPath(const QString& path_) { this->path = path_; }

/**
 * @brief 设置数据库连接名称
 * @param connectionName 连接名称
 */
void MSTDataSource::setConnectionName(const QString& connectionName) { connection = connectionName; }

/**
 * @brief 打开指定路径的数据库
 * @param path_ 数据库所在目录路径
 * @return 成功打开返回true，否则返回false
 */
bool MSTDataSource::openDB(const QString& path_) {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);

	// 规范化数据库文件路径（确保路径以斜杠结尾后拼接文件名）
	if (path_.length() - path_.lastIndexOf('/') == 1) {
		db.setDatabaseName(path_ + "musicInfo.db");
	}
	else {
		db.setDatabaseName(path_ + "/musicInfo.db");
	}

	// 尝试打开数据库
	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	query = QSqlQuery(db); // 初始化查询对象
	return true;
}

/**
 * @brief 使用已设置路径打开数据库
 * @return 成功打开返回true，否则返回false
 */
bool MSTDataSource::openDB() {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);

	// 规范化数据库文件路径
	if (path.length() - path.lastIndexOf('/') == 1) {
		db.setDatabaseName(path + "musicInfo.db");
	}
	else {
		db.setDatabaseName(path + "/musicInfo.db");
	}

	// 尝试打开数据库
	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	query = QSqlQuery(db); // 初始化查询对象
	return true;
}

/**
 * @brief 初始化数据库表结构
 * 创建音乐信息表，包含标题、艺术家、专辑、流派、年份、音轨、收藏、规则命中、文件名等字段
 */
void MSTDataSource::initTable() {
	query.exec("CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
		"track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)");
}

/**
 * @brief 关闭数据库连接
 */
void MSTDataSource::closeDB() {
	if (db.isOpen()) {
		db.close();
	}
}

/**
 * @brief 准备SQL语句
 * @param stmt 要准备的SQL语句
 */
void MSTDataSource::prepareStatement(const QString& stmt) { query.prepare(stmt); }

/**
 * @brief 绑定参数值到预处理语句
 * 使用模板避免重复代码，支持各种数据类型
 * @tparam T 参数类型
 * @param key 参数占位符
 * @param value 参数值
 */
template <class T>
void MSTDataSource::bindValue(const QString& key, const T& value) {
	// 运行时类型检查以避免错误
	if constexpr (std::is_convertible_v<const T&, QVariant>) {
		query.bindValue(key, value);
	}
	else {
		// 如果类型不直接支持，使用QVariant::fromValue转换
		query.bindValue(key, QVariant::fromValue(value));
	}
}

/**
 * @brief 执行查询
 */
void MSTDataSource::execQuery() { query.exec(); }

/**
 * @brief 批量更新收藏状态
 * @param updates 收藏状态更新结果列表
 */
void MSTDataSource::updateFavoriteBatch(const QList<FavoriteUpdate>& updates) {
	for (const auto& update : updates) {
		prepareStatement("UPDATE musicInfo SET favorite = :fav WHERE fileName = :fileName");
		bindValue(":fav", update.isFavorite);
		bindValue(":fileName", update.fileName);
		execQuery();
	}
}

/**
 * @brief 批量更新规则命中状态
 * @param updates 规则命中更新结果列表
 */
void MSTDataSource::updateRuleHitBatch(const QList<RuleHitUpdate>& updates) {
	for (const auto& update : updates) {
		prepareStatement("UPDATE musicInfo SET ruleHit = :ruleHit WHERE fileName = :fileName");
		bindValue(":ruleHit", update.isRuleHit);
		bindValue(":fileName", update.fileName);
		execQuery();
	}
}

/**
 * @brief 获取所有音乐数据
 * 根据指定的查询行类型构建SQL查询并返回结果
 * @param rows 要查询的字段类型向量，默认查询所有字段
 * @return 查询结果列表
 */
QList<QueryItem> MSTDataSource::getAll(const QVector<QueryRows>& rows) {
	QString sql = "SELECT ";

	// 根据查询行类型构建SELECT语句
	for (const auto& row : rows) {
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
	for (int i = 0; i < rows.size(); ++i) {
		rowMap.insert(i, rows[i]);
	}

	sql.chop(2); // 移除最后的逗号和空格
	sql += " FROM musicInfo ORDER BY title ASC"; // 按标题升序排序

	prepareStatement(sql);
	execQuery();

	// 解析查询结果
	QList<QueryItem> items;
	QueryItem temp;
	while (query.next()) {
		for (int i = 0; i < rowMap.size(); i++) {
			switch (rowMap[i]) {
			case QueryRows::TITLE:
				temp.setTitle(query.value(i).toString());
				break;
			case QueryRows::ARTIST:
				temp.setArtist(query.value(i).toString());
				break;
			case QueryRows::ALBUM:
				temp.setAlbum(query.value(i).toString());
				break;
			case QueryRows::GENRE:
				temp.setGenre(query.value(i).toString());
				break;
			case QueryRows::YEAR:
				temp.setYear(query.value(i).toUInt());
				break;
			case QueryRows::TRACK:
				temp.setTrack(query.value(i).toUInt());
				break;
			case QueryRows::FILENAME:
				temp.setFileName(query.value(i).toString());
				break;
			case QueryRows::ALL:
				// 查询所有字段时，按固定顺序设置值
				temp.setTitle(query.value(0).toString());
				temp.setArtist(query.value(1).toString());
				temp.setAlbum(query.value(2).toString());
				temp.setGenre(query.value(3).toString());
				temp.setYear(query.value(4).toUInt());
				temp.setTrack(query.value(5).toUInt());
				temp.setFileName(query.value(6).toString());
				break;
			}
		}
		items.append(temp);
	}
	return items;
}

/**
 * @brief 批量添加音乐文件到数据库
 * @param files 音乐文件路径列表
 * @return 添加失败的文件路径列表
 */
QStringList MSTDataSource::addMusic(const QStringList& files) {
	QStringList errList; // 错误文件列表

	for (const QString& file : files) {
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(path + "/" + file);

		// 检查文件是否有效
		if (fileRef.isNull()) {
			errList.append(file);
			Logger::Warn("Failed to add file" + file);
			continue;
		}

		const TagLib::Tag* tag = fileRef.tag(); // 获取音乐标签

		// 插入音乐信息到数据库
		prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
			":artist, :album, :genre, :year, :track, :fileName)");
		bindValue(":title", tag->title().to8Bit(true));
		bindValue(":artist", tag->artist().to8Bit(true));
		bindValue(":album", tag->album().to8Bit(true));
		bindValue(":genre", tag->genre().to8Bit(true));
		bindValue(":year", QString::number(tag->year()));
		bindValue(":track", QString::number(tag->track()));
		bindValue(":fileName", file);

		// 执行插入操作，失败时记录错误
		if (!query.exec()) {
			Logger::Warn("Error inserting data into database: " + query.lastError().text());
			errList.append(file);
		}
	}
	return errList;
}

/**
 * @brief 添加单个音乐文件到数据库
 * @param file 音乐文件路径
 * @return 成功添加返回true，否则返回false
 */
bool MSTDataSource::addMusic(const QString& file) {
	if (file.isEmpty()) {
		return false; // 文件路径为空
	}

	const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(path + "/" + file);

	// 检查文件是否有效
	if (fileRef.isNull()) {
		return false;
	}

	const TagLib::Tag* tag = fileRef.tag(); // 获取音乐标签

	// 插入音乐信息到数据库
	prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
		":artist, :album, :genre, :year, :track, :fileName)");
	bindValue(":title", QString::fromUtf8(tag->title().to8Bit(true)));
	bindValue(":artist", QString::fromUtf8(tag->artist().to8Bit(true)));
	bindValue(":album", QString::fromUtf8(tag->album().to8Bit(true)));
	bindValue(":genre", QString::fromUtf8(tag->genre().to8Bit(true)));
	bindValue(":year", QString::number(tag->year()));
	bindValue(":track", QString::number(tag->track()));
	bindValue(":fileName", file);

	// 执行插入操作        
	if (!query.exec()) {
		Logger::Warn("Error inserting data into database: " + query.lastError().text());
		return false;
	}
	return true;
}

/**
 * @brief 获取数据库中音乐记录的总数
 * @return 音乐记录总数
 */
int MSTDataSource::getCount() {
	prepareStatement("SELECT COUNT(*) FROM musicInfo");
	execQuery();
	query.next();
	return query.value(0).toInt();
}

/**
 * @brief 获取用于表格显示的音乐数据
 * 支持分页、排序和排序顺序控制
 * @param pageNum 页码（从1开始）
 * @param sortBy 排序字段
 * @param orderBy 排序顺序
 * @return 查询结果列表
 */
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

	// 添加分页限制
	sql += "LIMIT " + QString::number(pageSize) + " OFFSET " + QString::number((pageNum - 1) * pageSize);

	prepareStatement(sql);
	execQuery();

	// 构建结果列表
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

/**
 * @brief 搜索音乐
 * 在标题、艺术家和专辑字段中搜索指定文本
 * @param text 搜索关键词
 * @return 匹配的音乐列表
 */
QList<QueryItem> MSTDataSource::searchMusic(const QString& text) {
	prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title = :text OR "
		"artist = :text OR album = :text");
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

/**
 * @brief 根据音乐元数据获取文件名列表
 * @param items 音乐项列表
 * @return 对应的文件名列表
 */
QStringList MSTDataSource::getFileNameByMD(const QList<QueryItem>& items) {
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

/**
 * @brief 获取收藏的音乐列表
 * 支持分页、排序和进度报告
 * @param pageNum 页码（从1开始）
 * @param sortBy 排序字段
 * @param orderBy 排序顺序
 * @return 收藏音乐列表
 */
QList<QueryItem> MSTDataSource::getFavorite(const unsigned short pageNum, const SortByEnum sortBy,
                                            const OrderByEnum orderBy) {
	// emit loadStarted(); // 发出加载开始信号

	// 获取收藏音乐的真实总数（不带LIMIT，用于分页计算）
	prepareStatement("SELECT COUNT(*) FROM musicInfo WHERE favorite = 1");
	execQuery();
	query.next();
	const int size = query.value(0).toInt();
	lastFavoriteCount = size;
	emit totalSize(size); // 发出总数量信号

	// 构建查询语句
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
		emit currentProgress(i); // 发出当前进度信号
		item.setTitle(query.value(0).toString());
		item.setArtist(query.value(1).toString());
		item.setAlbum(query.value(2).toString());
		item.setGenre(query.value(3).toString());
		item.setYear(query.value(4).toInt());
		item.setTrack(query.value(5).toInt());
		items.append(item);
		i++;
	}
	// emit loadFinished(); // 发出加载完成信号
	return items;
}

/**
 * @brief 获取规则命中的音乐列表
 * 支持分页和排序
 * @param pageNum 页码（从1开始）
 * @param sortBy 排序字段
 * @param orderBy 排序顺序
 * @return 规则命中的音乐列表
 */
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

/**
 * @brief 删除指定的音乐文件记录
 * @param fileList 要删除的文件路径列表
 * @return 全部删除成功返回true，否则返回false
 */
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

/** 
 * @brief 通过文件名获取是否命中规则
 * @param fileName 音乐文件名
 * @return 如果命中规则返回true，否则返回false
 */
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
