#include "MSTDataSource.h"

#include <QFile>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>
#include "Logger.h"
#include "SingleInstance.h"

int MSTDataSource::getPageSize() const { return pageSize; }

void MSTDataSource::setPageSize(const int page_size) { pageSize = page_size; }

MSTDataSource::MSTDataSource(const QString& path) { this->path = path; }

void MSTDataSource::setPath(const QString& path_) { this->path = path_; }

void MSTDataSource::setConnectionName(const QString& connectionName) { connection = connectionName; }

bool MSTDataSource::openDB(const QString& path_) {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);
	if (path_.length() - path_.lastIndexOf('/') == 1) {
		// If we got a path with a trailing slash as last character
		db.setDatabaseName(path_ + "musicInfo.db");
	}
	else {
		db.setDatabaseName(path_ + "/musicInfo.db");
	}
	if (!db.open()) {
		Logger::Warn("Error opening database:" + db.lastError().text());
		return false;
	}
	query = QSqlQuery(db);
	return true;
}

bool MSTDataSource::openDB() {
	db = QSqlDatabase::addDatabase("QSQLITE", connection);
	if (path.length() - path.lastIndexOf('/') == 1) {
		db.setDatabaseName(path + "musicInfo.db");
	}
	else {
		db.setDatabaseName(path + "/musicInfo.db");
	}
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

void MSTDataSource::prepareStatement(const QString& stmt) { query.prepare(stmt); }

template <class T> // Use template to avoid repeated code
void MSTDataSource::bindValue(const QString& key, const T& value) {
	// Run type check to avoid errors
	if constexpr (std::is_convertible_v<const T&, QVariant>) {
		query.bindValue(key, value);
	}
	else {
		// If type is not directly supported
		query.bindValue(key, QVariant::fromValue(value));
	}
}

void MSTDataSource::execQuery() { query.exec(); }

void MSTDataSource::setFavorite(const TagLib::String& tag, const QDateTime& timeFromLog) {
	emit loadStarted();
	emit totalSize(this->getCount());
	if (tag.isEmpty()) {
		return;
	}
	prepareStatement("SELECT fileName FROM musicInfo");
	execQuery();
	QStringList fileName;
	while (query.next()) {
		fileName.append(query.value(0).toString());
	}
	int progress = 0;
	for (auto& file : fileName) {
		emit currentProgress(progress++);
		if (QFile(path + "/" + file).fileTime(QFileDevice::FileModificationTime) <= timeFromLog) {
			continue;
		}
		TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
		f = TagLib::FileRef((path + "/" + file).toStdWString().c_str());
#elif defined(__linux)
        f = TagLib::FileRef((path + "/" + file).toStdString().c_str());
#endif
		prepareStatement("UPDATE musicInfo SET favorite = :fav WHERE title = :title, artist = :artist, album = :album, "
			"fileName = :fileName");
		bindValue(":fav", f.properties().contains(tag));
		bindValue(":title", QString::fromUtf8(f.tag()->title().to8Bit(true)));
		bindValue(":artist", QString::fromUtf8(f.tag()->artist().to8Bit(true)));
		bindValue(":album", QString::fromUtf8(f.tag()->album().to8Bit(true)));
		bindValue(":fileName", file);
		execQuery();
	}
	emit loadFinished();
}

void MSTDataSource::setRuleHit(const QList<LyricIgnoreRule>& rules, const QDateTime& timeFromLog) {
	emit loadStarted();
	if (rules.isEmpty()) {
		return;
	}
	int progress = 0;
	QList<QueryItem> items = getAll({QueryRows::ALL}, ASC, TITLE);
	for (auto& item : items) {
		emit currentProgress(progress++);
		if (QFile(path + "/" + item.getFileName()).fileTime(QFileDevice::FileModificationTime) <= timeFromLog) {
			continue;
		}
		prepareStatement("UPDATE musicInfo SET "
			"ruleHit = :ruleHit WHERE "
			"title = :title, "
			"artist = :artist, "
			"album = :album, "
			"fileName = :fileName");
		bindValue(":ruleHit", std::any_of(rules.begin(), rules.end(), [&item](const LyricIgnoreRule& rule) {
			QString fieldStr;
			switch (rule.getRuleField()) {
			case RuleField::TITLE:
				fieldStr = item.getTitle();
				break;
			case RuleField::ARTIST:
				fieldStr = item.getArtist();
				break;
			case RuleField::ALBUM:
				fieldStr = item.getAlbum();
				break;
			}
			const QRegularExpression regExp(rule.getRuleName());
			const bool hasMatch = regExp.match(fieldStr).hasMatch();
			return (rule.getRuleType() == RuleType::INCLUDES) ? hasMatch : !hasMatch;
		}));
		bindValue(":title", item.getTitle());
		bindValue(":artist", item.getArtist());
		bindValue(":album", item.getAlbum());
		execQuery();
	}
}

QList<QueryItem> MSTDataSource::getAll(const QVector<QueryRows>& rows, const OrderByEnum orderBy = ASC,
                                       const SortByEnum sortBy = TITLE) {
	QString sql = "SELECT ";
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
	QMap<int, QueryRows> rowMap;
	for (int i = 0; i < rows.size(); ++i) {
		rowMap.insert(i, rows[i]);
	}
	sql.chop(2); // Remove the last comma and space
	sql += " FROM musicInfo";
	switch (sortBy) {
	case TITLE:
		sql += " ORDER BY title ";
		break;
	case ARTIST:
		sql += " ORDER BY artist ";
		break;
	case ALBUM:
		sql += " ORDER BY album ";
		break;
	}
	switch (orderBy) {
	case ASC:
		sql += "ASC";
		break;
	case DESC:
		sql += "DESC";
		break;
	}
	prepareStatement(sql);
	execQuery();
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

QStringList MSTDataSource::addMusic(const QStringList& files) {
	QStringList errList;
	emit loadStarted();
	int progress = 0;
	emit totalSize(files.size());
	for (const QString& file : files) {
		emit currentProgress(progress);
		TagLib::FileRef fileRef;
#if defined(_WIN64) or defined(_WIN32)
		fileRef = TagLib::FileRef((path + "/" + file).toStdWString().c_str());
#elif defined(__linux)
        fileRef = TagLib::FileRef((path + "/" + file).toStdString().c_str());
#endif
		if (fileRef.isNull()) {
			errList.append(file);
			Logger::Warn("Failed to add file" + file);
			continue;
		}
		const TagLib::Tag* tag = fileRef.tag();
		prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
			":artist, :album, :genre, :year, :track, :fileName)");
		bindValue(":title", tag->title().to8Bit(true));
		bindValue(":artist", tag->artist().to8Bit(true));
		bindValue(":album", tag->album().to8Bit(true));
		bindValue(":genre", tag->genre().to8Bit(true));
		bindValue(":year", QString::number(tag->year()));
		bindValue(":track", QString::number(tag->track()));
		bindValue(":fileName", file);
		if (!query.exec()) {
			qWarning() << "[WARN] Error inserting data into database:" << query.lastError().text();
			errList.append(file);
		}
		progress++;
	}
	emit loadFinished();
	return errList;
}

bool MSTDataSource::addMusic(const QString& file) {
	if (file.isEmpty()) {
		return false;
	}
	TagLib::FileRef fileRef;
#if defined(_WIN64) or defined(_WIN32)
	fileRef = TagLib::FileRef((path + "/" + file).toStdWString().c_str());
#elif defined(__linux)
    fileRef = TagLib::FileRef((path + "/" + file).toStdString().c_str());
#endif
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
		qWarning() << "[WARN] Error inserting data into database:" << query.lastError().text();
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
	emit loadStarted();
	if (getCount() - (pageNum - 1) * pageSize < pageSize) {
		emit totalSize(getCount() - (pageNum - 1) * pageSize);
	}
	else {
		emit totalSize(pageSize);
	}
	QString sql = "SELECT title, artist, album, genre, year, track FROM musicInfo ORDER BY";
	switch (sortBy) {
	case TITLE:
		sql += " TITLE ";
		break;
	case ARTIST:
		sql += " ARTIST ";
		break;
	case ALBUM:
		sql += " ALBUM ";
		break;
	}
	switch (orderBy) {
	case ASC:
		sql += "ASC ";
		break;
	case DESC:
		sql += "DESC ";
		break;
	}
	sql += "LIMIT " + QString::number(pageSize) + " OFFSET " + QString::number((pageNum - 1) * pageSize);
	prepareStatement(sql);
	execQuery();
	QList<QueryItem> result;
	auto item = QueryItem();
	int progress = 0;
	while (query.next()) {
		item.setTitle(query.value(0).toString());
		item.setArtist(query.value(1).toString());
		item.setAlbum(query.value(2).toString());
		item.setGenre(query.value(3).toString());
		item.setYear(query.value(4).toInt());
		item.setTrack(query.value(5).toInt());
		result.append(item);
		emit currentProgress(progress++);
	}
	emit loadFinished();
	return result;
}

QPair<int, QList<QueryItem>> MSTDataSource::searchMusic(const QString& text) {
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
	prepareStatement("SELECT COUNT(*) FROM musicInfo WHERE title = :text OR "
		"artist = :text OR album = :text");
	bindValue(":text", text);
	query.next();
	int searchCount = query.value(0).toInt();
	const auto result = QPair<int, QList<QueryItem>>(searchCount, items);
	return result;
}

QString MSTDataSource::getFileNameByMD(const QueryItem& item) {
	prepareStatement("SELECT fileName FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album");
	bindValue(":title", item.getTitle());
	bindValue(":artist", item.getArtist());
	bindValue(":album", item.getAlbum());
	execQuery();
	query.next();
	QString fileName = query.value(0).toString();
	return fileName;
}

inline bool MSTDataSource::getFavorite(const QueryItem& item) {
	prepareStatement("SELECT favorite FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND "
		"fileName = :fileName");
	bindValue(":title", item.getTitle());
	bindValue(":artist", item.getArtist());
	bindValue(":album", item.getAlbum());
	bindValue(":fileName", item.getFileName());
	execQuery();
	query.next();
	return query.value(0).toBool();
}

inline bool MSTDataSource::getRuleHit(const QueryItem& item) {
	prepareStatement("SELECT ruleHit FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND "
		"fileName = :fileName");
	bindValue(":title", item.getTitle());
	bindValue(":artist", item.getArtist());
	bindValue(":album", item.getAlbum());
	bindValue(":fileName", item.getFileName());
	execQuery();
	query.next();
	return query.value(0).toBool();
}

bool MSTDataSource::deleteMusic(const QStringList& fileList) {
	if (fileList.isEmpty()) {
		return true;
	}
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
