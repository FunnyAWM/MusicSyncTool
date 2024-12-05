#pragma execution_character_set("utf-8") 
#include "MusicSyncTool.h"
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QSet>
#include <QThread>
#include <QString>
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
	ui.tableWidgetLocal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableWidgetRemote->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
		qDebug() << "[FATAL] Error opening database:" << db.lastError().text().toStdString();
		exit(EXIT_FAILURE);
	}
}

void MusicSyncTool::getMusic(pathType path) {
	qDebug() << "[INFO] Scanning started";
	clock_t start = clock();
	if (localPath == "" && remotePath == "") {
		qDebug() << "[WARN] No path selected";
		return;
	}
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
	QTableWidget* targetTable = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
	targetTable->clearContents();
	targetTable->setRowCount(tableSize);
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				targetTable->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
	}
	clock_t end = clock();
	qDebug() << "[INFO] Scanning finished in" << double(end - start) / CLOCKS_PER_SEC << "seconds";
	loading.close();
}

void MusicSyncTool::searchMusic(pathType path, QString text)
{
	if (text == "") {
		getMusic(path);
		return;
	}
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->clearContents();
	query.clear();
	QString sql = "SELECT COUNT(*) FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text + "%\" OR album LIKE \"%" + text + "%\"";
	query.exec(sql);
	query.next();
	int tableSize = query.value(0).toInt();
	(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setRowCount(tableSize);
	sql = "SELECT * FROM musicInfo WHERE title LIKE \"%" + text + "%\" OR artist LIKE \"%" + text + "%\" OR album LIKE \"%" + text + "%\"";
	query.clear();
	query.exec(sql);
	while (query.next()) {
		for (int i = 0; i < 6; i++) {
			if (query.value(i) != 0) {
				(path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote)->setItem(query.at(), i, new QTableWidgetItem(query.value(i).toString()));
			}
		}
	}
}

QStringList MusicSyncTool::getDuplicatedMusic(pathType path) {
	ShowDupe dp;
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	QString sql = "SELECT * FROM musicInfo ORDER BY title";
	query.clear();
	query.exec(sql);
	QString fast;
	QString fastFileName;
	QString slow;
	QString slowFileName;
	QStringList dupeList;
	query.next();
	slow = query.value(0).toString() + query.value(1).toString();
	slowFileName = query.value(6).toString();
	while (query.next()) {
		fast = query.value(0).toString() + query.value(1).toString();
		fastFileName = query.value(6).toString();
		if (fast == slow) {
			dupeList.append(fastFileName);
			dupeList.append(slowFileName);
		}
		slow = fast;
		slowFileName = fastFileName;
	}
	for (int i = 0; i < dupeList.size(); i++) {
		qDebug() << "[INFO] Found duplicated music named" << dupeList.at(i) << "at" << (path == pathType::LOCAL ? localPath : remotePath);
	}
	for (int i = 0; i < dupeList.size(); i++) {
		dp.add(dupeList.at(i));
	}
	dp.exec();
	return dupeList;
}

QStringList MusicSyncTool::getSelectedMusic(pathType path) {
	QSet<int> selectedRows;
	const QTableWidget* const table = (path == pathType::LOCAL ? ui.tableWidgetLocal : ui.tableWidgetRemote);
	if (table->rowCount() == 0) {
		return QStringList();
	}
	for (int i = 0; i < table->rowCount(); i++) {
		if (table->item(i, 0)->isSelected()) {
			selectedRows.insert(i);
		}
	}
	QStringList titleList;
	QStringList artistList;
	for (int i : selectedRows) {
		titleList.append(table->item(i, 0)->text());
		artistList.append(table->item(i, 1)->text());
	}
	QSqlQuery& query = (path == pathType::LOCAL ? queryLocal : queryRemote);
	QString titleListString;
	QString artistListString;
	for (int i = 0; i < selectedRows.size() - 1; i++) {
		titleListString += "\"" + titleList.at(i) + "\", ";
		artistListString += "\"" + artistList.at(i) + "\", ";
	}
	titleListString += "\"" + titleList.at(selectedRows.size() - 1) + "\"";
	artistListString += "\"" + artistList.at(selectedRows.size() - 1) + "\"";
	QString sql = "SELECT fileName FROM musicInfo WHERE title IN("+ titleListString + ") AND artist IN(" + artistListString + ")";
	QStringList fileList;
	query.clear();
	query.exec(sql);
	while (query.next()) {
		fileList.append(query.value(0).toString());
	}
	return fileList;
}

void MusicSyncTool::copyMusic(const QString source, const QStringList fileList, const QString target) {
	if (source == "" || fileList.isEmpty() || target == "") {
		return;
	}
	QDir dir(target);
	if (dir.isEmpty()) {
		dir.mkpath(target);
	}
	for (int i = 0; i < fileList.size(); i++) {
		QString sourceFile = source + "/" + fileList.at(i);
		QString targetFile = target + "/" + fileList.at(i);
		if (QFile::exists(targetFile)) {
			qDebug() << "[WARN] File existed, skipping" << targetFile;
			continue;
		}
		QFile::copy(sourceFile, targetFile);
	}
	QStringList lyricsList;
	for (int i = 0; i < fileList.size(); i++) {
		QString current = fileList.at(i);
		if (current.contains(".mp3")) {
			lyricsList.append(current.replace(".mp3", ".lrc"));
		}
		else if (current.contains(".flac")) {
			lyricsList.append(current.replace(".flac", ".lrc"));
		}
	}
	for (int i = 0; i < lyricsList.size(); i++) {
		QString sourceFile = source + "/" + lyricsList.at(i);
		QString targetFile = target + "/" + lyricsList.at(i);
		if (QFile::exists(targetFile) || !QFile::exists(sourceFile)) {
			qDebug() << "[WARN] Skipping" << targetFile;
			continue;
		}
		QFile::copy(sourceFile, targetFile);
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

void MusicSyncTool::on_copyToRemote_clicked() {
	QStringList fileList = getSelectedMusic(pathType::LOCAL);
	copyMusic(localPath, fileList, remotePath);
	getMusic(pathType::REMOTE);
}

void MusicSyncTool::on_copyToLocal_clicked() {
	QStringList fileList = getSelectedMusic(pathType::REMOTE);
	copyMusic(remotePath, fileList, localPath);
	getMusic(pathType::LOCAL);
}

void MusicSyncTool::on_actionDupeLocal_triggered(bool triggered) {
	getDuplicatedMusic(pathType::LOCAL);
}

void MusicSyncTool::on_actionDupeRemote_triggered(bool triggered) {
	getDuplicatedMusic(pathType::REMOTE);
}

void MusicSyncTool::on_refreshLocal_clicked() {
	getMusic(pathType::LOCAL);
}

void MusicSyncTool::on_refreshRemote_clicked() {
	getMusic(pathType::REMOTE);
}

void MusicSyncTool::on_searchLocal_returnPressed()
{
	searchMusic(pathType::LOCAL, ui.searchLocal->text());
}

void MusicSyncTool::on_searchRemote_returnPressed()
{
	searchMusic(pathType::REMOTE, ui.searchRemote->text());
}

void MusicSyncTool::on_actionExit_triggered(bool triggered) {
	exit(EXIT_SUCCESS);
}
