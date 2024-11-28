#pragma execution_character_set("utf-8") 
#include "MusicSyncTool.h"
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QSet>
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <taglib/mpegfile.h>
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

void MusicSyncTool::openFolder(pathType path) {
	QFileDialog fileDialog;
	fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
	fileDialog.setFileMode(QFileDialog::Directory);
	QString dir = fileDialog.getExistingDirectory();
	if (dir == "") {
		return;
	}
	(path == pathType::LOCAL ? localPath : remotePath) = dir;
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

void MusicSyncTool::getMusic(pathType path) {
	if (localPath == "" && remotePath == "") {
		return;
	}
	LoadingPage loading;
	loading.show();
	QDir dir(path == pathType::LOCAL ? localPath : remotePath);
	QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, track INT, fileName TEXT)";
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	query.exec(sql);
	QStringList newFileList = dir.entryList(QDir::Files);
	sql = "SELECT fileName FROM musicInfo";
	query.exec(sql);
	QStringList oldFileList;
	while (query.next()) {
		oldFileList.append(query.value(0).toString());
	}
	newFileList.sort();
	oldFileList.sort();
	for (int i = 0; i < oldFileList.size(); i++) {
		if (!newFileList.contains(oldFileList.at(i))) {
			sql = "DELETE FROM musicInfo WHERE fileName = \"" + oldFileList.at(i) + "\"";
			query.exec(sql);
			oldFileList.removeAt(i);
			i = -1;
		}
	}
	for (int i = 0; i < newFileList.size(); i++) {
		if (oldFileList.contains(newFileList.at(i)) || newFileList.at(i).contains(".lrc")) {
			newFileList.removeAt(i);
			i = -1;
		}
	}
	newFileList.removeOne("musicInfo.db");
	for (int i = 0; i < newFileList.size(); i++) {
		QString file = (path == pathType::LOCAL ? localPath : remotePath) + "/" + newFileList.at(i).toUtf8();
		TagLib::FileRef f(file.toStdWString().c_str());
		if (!f.isNull() && f.tag()) {
			TagLib::Tag* tag = f.tag();
			sql = "INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) VALUES (\"" 
				+ QString::fromStdString(tag->title().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->artist().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->album().to8Bit(true))
				+ "\", \"" + QString::fromStdString(tag->genre().to8Bit(true))
				+ "\", " + QString::number(tag->year())
				+ ", " + QString::number(tag->track()) 
				+ ", \"" + newFileList.at(i)
				+ "\")";
			query.exec(sql);
		} else {
			continue;
		}
	}
	sql = "SELECT COUNT(*) FROM musicInfo";
	query.exec(sql);
	query.next();
	int tableSize = query.value(0).toInt();
	sql = "SELECT title, artist, album, genre, year, track FROM musicInfo";
	query.exec(sql);
	(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setRowCount(tableSize);
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
	}
	loading.close();
}

QStringList MusicSyncTool::getSelectedMusic(pathType path) {
	QSet<int> selectedRows;
	QTableWidget* table = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
	for (int i = 0; i < table->rowCount(); i++) {
		if (table->item(i, 0)->isSelected()) {
			selectedRows.insert(i);
		}
	}
	QStringList titleList;
	for (int i : selectedRows) {
		titleList.append(table->item(i, 0)->text());
	}
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	QString titleListString;
	for (int i = 0; i < selectedRows.size() - 1; i++) {
		titleListString += "\"" + titleList.at(i) + "\", ";
	}
	titleListString += "\"" + titleList.at(selectedRows.size() - 1) + "\"";
	QString sql = "SELECT fileName FROM musicInfo WHERE title IN("+ titleListString + ")";
	QStringList fileList;
	query.exec(sql);
	while (query.next()) {
		fileList.append(query.value(0).toString());
	}
	return fileList;
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
		if (QFile::exists(targetFile)) {
			continue;
		}
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
void MusicSyncTool::on_actionAbout_triggered(bool triggered) {
	AboutPage about;
	about.exec();
}
void MusicSyncTool::on_actionExit_triggered(bool triggered) {
	exit(EXIT_SUCCESS);
}