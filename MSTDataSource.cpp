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

void MSTDataSource::getAll(const QStringList& cols) {
	QString sql = "SELECT ";
	for (const QString& col : cols) {
		sql += col + ", ";
	}
	sql.chop(2);
	sql += " FROM musicInfo";
	query.exec(sql);
}

QStringList MSTDataSource::addMusic(const QStringList& fileList) {
	auto errList = QStringList();
	for (const QString& file : fileList) {
		TagLib::FileRef f;
		f = TagLib::FileRef(file.toStdString().c_str());
		if (f.isNull() || !f.tag()) {
			qWarning() << "[WARN] Error reading file:" << file;
			errList.append(file);
			continue;
		}
		prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (:title, "
			":artist, :album, :genre, :year, :track, :fileName)");
		bindValue(":title", f.tag()->title().toCString(true));
		bindValue(":artist", f.tag()->artist().toCString(true));
		bindValue(":album", f.tag()->album().toCString(true));
		bindValue(":genre", f.tag()->genre().toCString(true));
		bindValue(":year", QString::number(f.tag()->year()));
		bindValue(":track", QString::number(f.tag()->track()));
		if (!query.exec()) {
			qWarning() << "[WARN] Error inserting data to database:" << query.lastError().text();
			errList.append(file);
		}
	}
	return errList;
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
