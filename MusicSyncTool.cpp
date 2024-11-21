#include "MusicSyncTool.h"
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <taglib/fileref.h>
#include <iostream>

MusicSyncTool::MusicSyncTool(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	db = QSqlDatabase::addDatabase("QSQLITE","musicInfoLocal");
}

MusicSyncTool::~MusicSyncTool()
{
	db.close();
}

void MusicSyncTool::getMusic() {
	if (db.isOpen()) {
		db.close();
	}
	QDir dir(localPath);
	QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, track INT)";
	query.exec(sql);
	QStringList fileList = dir.entryList(QDir::Files);
	for (int i = 0; i < fileList.size(); i++) {
		QString file = localPath + "/" + fileList.at(i);
		TagLib::FileRef f(file.toStdString().c_str());
		if (!f.isNull() && f.tag()) {
			TagLib::Tag* tag = f.tag();
			QString title = QString::fromStdString(tag->title().to8Bit(true));
			QString artist = QString::fromStdString(tag->artist().to8Bit(true));
			QString album = QString::fromStdString(tag->album().to8Bit(true));
			QString genre = QString::fromStdString(tag->genre().to8Bit(true));
			int year = tag->year();
			int track = tag->track();
			sql = "INSERT INTO musicInfo (title, artist, album, genre, year, track) VALUES ('" + title + "', '" + artist + "', '" + album + "', '" + genre + "', " + QString::number(year) + ", " + QString::number(track) + ")";
			query.exec(sql);
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
void MusicSyncTool::on_actionOpen_Folder_triggered(bool triggered) {
	QFileDialog fileDialog;
	fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
	fileDialog.setFileMode(QFileDialog::Directory);
	QString dir = fileDialog.getExistingDirectory();
	localPath = dir;
	db.setDatabaseName(localPath + "/musicInfo.db");
	if (!db.open()) {
		std::cerr << "Error opening database" << std::endl;
	}
	query = QSqlQuery(db);
}