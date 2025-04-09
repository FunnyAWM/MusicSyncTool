#include "MSTDataSource.h"

#include <QFile>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

MSTDataSource::MSTDataSource(const QString& path) {
	this->path = path;
	if (!openDB(path + "/musicInfo.db")) {
		qWarning() << "[WARN] Failed to open database at path:" << path + "/musicInfo.db";
	}
}

bool MSTDataSource::openDB(const QString& path_) {
	db = QSqlDatabase::addDatabase("QSQLITE");
	if (path_.length() - path_.lastIndexOf('/') == 1) {
		//If we got a path with a trailing slash as last character
		db.setDatabaseName(path_ + "musicInfo.db");
	}
	else {
		db.setDatabaseName(path_ + "/musicInfo.db");
	}
	if (!db.open()) {
		qWarning() << "[WARN] Error opening database:" << db.lastError().text();
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

template <class T>
// Use template to avoid repeated code
void MSTDataSource::bindValue(const QString& key, const T& value) {
	const auto variantValue = QVariant(value); // Run type check to ensure value is valid
	if (!variantValue.isValid()) {
		qFatal("[FATAL] Error in bindValue: Invalid value for key %s", qPrintable(key));
	}
	query.bindValue(key, variantValue);
}

void MSTDataSource::execQuery() { query.exec(); }

void MSTDataSource::setFavorite(const QString& tag) {
	prepareStatement("SELECT fileName FROM musicInfo");
	execQuery();
	QStringList fileName;
	while (query.next()) {
		fileName.append(query.value(0).toString());
	}
	for (auto& file : fileName) {
		TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
		f = TagLib::FileRef(file.toStdWString().c_str());
#else
        f = TagLib::FileRef(file.toStdString().c_str());
#endif
		prepareStatement("UPDATE musicInfo SET favorite = :fav WHERE title = :title, artist = :artist, album = :album, "
			"fileName = :fileName");
		bindValue(":fav", f.properties().contains(tag.toStdString()));
		bindValue(":title", f.tag()->title().toCString());
		bindValue(":artist", f.tag()->artist().toCString());
		bindValue(":album", f.tag()->album().toCString());
	}
}

void MSTDataSource::setRuleHit(const QList<LyricIgnoreRule>& rules) {
	QList<QueryItem> items = getAll({QueryRows::ALL});
	for (auto& item : items) {
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

QList<QueryItem> MSTDataSource::getAll(const QVector<QueryRows>& rows) {
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
	auto errList = QStringList();
	for (const QString& file : files) {
		TagLib::FileRef fileRef;
#if defined(_WIN64) or defined(_WIN32)
		fileRef = TagLib::FileRef(file.toStdWString().c_str());
#else
        fileRef = TagLib::FileRef(file.toStdString().c_str());
#endif
		if (fileRef.isNull()) {
			errList.append(file);
			continue;
		}
		const TagLib::Tag* tag = fileRef.tag();
		prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) "
			"VALUES (:title, :artist, :album, :genre, :year, :track, :fileName)");
		bindValue(":title", tag->title().toCString());
		bindValue(":artist", tag->artist().toCString());
		bindValue(":album", tag->album().toCString());
		bindValue(":genre", tag->genre().toCString());
		bindValue(":year", QString::number(tag->year()));
		bindValue(":track", QString::number(tag->track()));
		bindValue(":fileName", file);
		if (!query.exec()) {
			qWarning() << "[WARN] Error inserting data into database:" << query.lastError().text();
			errList.append(file);
		}
	}
	return errList;
}

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
	return std::all_of(fileList.begin(), fileList.end(), [this](const QString& file) {
		prepareStatement("DELETE FROM musicInfo WHERE fileName = :fileName");
		bindValue(":fileName", file);
		if (!query.exec()) {
			qWarning() << "[WARN] Error deleting data from database:" << query.lastError().text();
			return false;
		}
		return true;
	});
}
