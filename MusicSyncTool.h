#pragma once

#include <QMainWindow>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlresult>
#include "ui_MusicSyncTool.h"


class MusicSyncTool : public QMainWindow
{
    Q_OBJECT

public:
    enum class pathType { LOCAL, REMOTE };
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void openFolder(pathType);
    void getMusic(pathType);
    void copyMusic(QString, QStringList, QString);
private:
    Ui::MusicSyncToolClass ui;
    QString localPath;
    QString remotePath;
	QSqlDatabase dbLocal;
    QSqlDatabase dbRemote;
	QSqlQuery queryLocal;
    QSqlQuery queryRemote;
public slots:
    void on_actionRemote_triggered(bool);
    void on_actionLocal_triggered(bool);
};
// QT_END_NAMESPACE