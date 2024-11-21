#pragma once

#include <QMainWindow>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include "ui_MusicSyncTool.h"


class MusicSyncTool : public QMainWindow
{
    Q_OBJECT

public:
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void getMusic();
	void copyMusic(QString, QStringList, QString);

private:
    Ui::MusicSyncToolClass ui;
    QString localPath;
    QString remotePath;
	QSqlDatabase db;
	QSqlQuery query;
	enum class pathType { LOCAL, REMOTE };
public slots:
    void on_actionOpen_Folder_triggered(bool);
};
// QT_END_NAMESPACE