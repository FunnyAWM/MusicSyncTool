#include "MSTDataSource.h"

#include <taglib/fileref.h>

MSTDataSource::MSTDataSource(const QString& path) {
	this->path = path;
	openDB(path + "/musicInfo.db");
}

void MSTDataSource::openDB(const QString& path) {
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName(path);
	if (!db.open()) {
		qWarning() << "[WARN] Error opening database:" << db.lastError().text();
	}
	query = QSqlQuery(db);
}

void MSTDataSource::initTable()
{
	query.exec("CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
		"track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)");
}

void MSTDataSource::closeDB() {
	if (db.isOpen()) {
		db.close();
	}
}

void MSTDataSource::prepareStatement(const QString& stmt) {
	query.prepare(stmt);
}

void MSTDataSource::bindValue(const QString& key, const QString& value) {
	query.bindValue(key, value);
}

void MSTDataSource::execQuery() {
	query.exec();
}

QList<QueryItem> MSTDataSource::getAll() {
	prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo");
	execQuery();
	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(), query.value(6).toString()));
	}
	return items;
}

QStringList MSTDataSource::addMusic(const QStringList& files) {
	auto errList = QStringList();
	for (const QString& file : files) {
		const TagLib::FileRef fileRef(file.toStdWString().c_str());
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
	prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title = :text OR artist = :text OR album = :text");
	bindValue(":text", text);
	execQuery();
	QList<QueryItem> items;
	while (query.next()) {
		items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(), query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(), query.value(6).toString()));
	}
	return items;
}

QStringList MSTDataSource::getFileName(const QList<QueryItem>& items)
{
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
	prepareStatement("SELECT favorite FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND fileName = :fileName");
	bindValue(":title", item.getTitle());
	bindValue(":artist", item.getArtist());
	bindValue(":album", item.getAlbum());
	bindValue(":fileName", item.getFileName());
	execQuery();
	query.next();
	return query.value(0).toBool();
}

inline bool MSTDataSource::getRuleHit(const QueryItem& item) {
	prepareStatement("SELECT ruleHit FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND fileName = :fileName");
	bindValue(":title", item.getTitle());
	bindValue(":artist", item.getArtist());
	bindValue(":album", item.getAlbum());
	bindValue(":fileName", item.getFileName());
	execQuery();
	query.next();
	return query.value(0).toBool();
}

bool MSTDataSource::deleteMusic(const QStringList& fileList) {
	for (const QString& file : fileList) {  // NOLINT(readability-use-anyofallof)
		prepareStatement("DELETE FROM musicInfo WHERE fileName = :fileName");
		bindValue(":fileName", file);
		if (!query.exec()) {
			qWarning() << "[WARN] Error deleting data from database:" << query.lastError().text();
			return false;
		}
	}
	return true;
}
