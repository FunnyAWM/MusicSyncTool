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
#include "AboutPage.h"
#include "LoadingPage.h"

class MusicSyncTool : public QMainWindow
{
    Q_OBJECT

    Ui::MusicSyncToolClass ui;
    QString localPath;
    QString remotePath;
    QSqlDatabase dbLocal;
    QSqlDatabase dbRemote;
    QSqlQuery queryLocal;
    QSqlQuery queryRemote;
public:
    enum class pathType { LOCAL, REMOTE };
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void openFolder(pathType);
    void getMusic(pathType);
    QStringList getDuplicatedMusic(pathType);
    QStringList getSelectedMusic(pathType);
    void copyMusic(QString, QStringList, QString);
public slots:
    void on_actionRemote_triggered(bool);
    void on_actionLocal_triggered(bool);
	void on_actionAbout_triggered(bool);
    void on_copyToRemote_clicked();
    void on_copyToLocal_clicked();
	void on_actionExit_triggered(bool);
};
// QT_END_NAMESPACE