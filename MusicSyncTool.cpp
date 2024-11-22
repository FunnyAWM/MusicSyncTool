#include "MusicSyncTool.h"
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <taglib/fileref.h>
#include <iostream>

MusicSyncTool::MusicSyncTool(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	dbLocal = QSqlDatabase::addDatabase("QSQLITE", "musicInfoLocal");
	dbRemote = QSqlDatabase::addDatabase("QSQLITE", "musicInfoRemote");
	queryLocal = QSqlQuery(dbLocal);
	queryRemote = QSqlQuery(dbRemote);
}

MusicSyncTool::~MusicSyncTool() {
	if (dbLocal.isOpen()) {
		dbLocal.close();
	}
	if (dbRemote.isOpen()) {
		dbRemote.close();
	}
}

void MusicSyncTool::getMusic(pathType path) {
	QDir dir(path == pathType::LOCAL ? localPath : remotePath);
	QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, track INT)";
	QSqlQuery& query = path == pathType::LOCAL ? queryLocal : queryRemote;
	query.exec(sql);
	QStringList fileList = dir.entryList(QDir::Files);
	QSqlQuery insertQuery("INSERT INTO musicInfo (title, artist, album, genre, year, track) VALUES (:title, :artist, :album, :genre, :year, :track)", path == pathType::LOCAL ? dbLocal : dbRemote);
	for (int i = 0; i < fileList.size(); i++) {
		QString file = localPath + "/" + fileList.at(i);
		TagLib::FileRef f(file.toStdString().c_str());
		if (!f.isNull() && f.tag()) {
			TagLib::Tag* tag = f.tag();
			insertQuery.bindValue(":title", QString::fromStdString(tag->title().to8Bit(true)));
			insertQuery.bindValue(":artist", QString::fromStdString(tag->artist().to8Bit(true)));
			insertQuery.bindValue(":album", QString::fromStdString(tag->album().to8Bit(true)));
			insertQuery.bindValue(":genre", QString::fromStdString(tag->genre().to8Bit(true)));
			insertQuery.bindValue(":year", tag->year());
			insertQuery.bindValue(":track", tag->track());
			insertQuery.exec();
		}
		else {
			std::cerr << "Error reading file" << std::endl;
		}
	}
}

void MusicSyncTool::copyMusic(QString source, QStringList fileList, QString target) {
	QDir dir(target);
	if (dir.isEmpty()) {
		dir.mkpath(target);
	}
	for (int i = 0; i < fileList.size(); i++) {
		QString sourceFile = source + "/" + fileList.at(i);
		QString targetFile = target + "/" + fileList.at(i);
		QFile::copy(sourceFile, targetFile);
	}
}
void MusicSyncTool::openFolder(pathType path) {
	QFileDialog fileDialog;
	fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
	fileDialog.setFileMode(QFileDialog::Directory);
	QString dir = fileDialog.getExistingDirectory();
	path == pathType::LOCAL ? localPath : remotePath = dir;
	if (path == pathType::LOCAL) {
		dbLocal.setDatabaseName(localPath + "/musicInfo.db");
	}
	else {
		dbRemote.setDatabaseName(remotePath + "/musicInfo.db");
	}
	QSqlDatabase& db = path == pathType::LOCAL ? dbLocal : dbRemote;
	if (!db.open()) {
		std::string err = db.lastError().text().toStdString();
		std::cerr << "Error opening database: " << db.lastError().text().toStdString() << std::endl;
		exit(EXIT_FAILURE);
	}
}
void MusicSyncTool::on_actionRemote_triggered(bool triggered) {
	openFolder(pathType::REMOTE);
	getMusic(pathType::REMOTE);
}
void MusicSyncTool::on_actionLocal_triggered(bool triggered) {
	openFolder(pathType::LOCAL);
	getMusic(pathType::LOCAL);
}