#pragma once

#include <QMainWindow>
#include <QFile>
#include <QJsonDocument>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDriver>
#include "ui_MusicSyncTool.h"
#include "AboutPage.h"
#include "LoadingPage.h"
#include "ShowDupe.h"
#include "Settings.h"
#include "CopyResult.h"

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
    QJsonDocument settings;
	LoadingPage loading = LoadingPage();
    bool ignoreLyric;
	int sortBy;
    QString language;
public:
    enum class pathType { LOCAL, REMOTE };
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void openFolder(pathType);
    void getMusic(pathType);
	void searchMusic(pathType, QString);
    QStringList getDuplicatedMusic(pathType);
    QStringList getSelectedMusic(pathType);
    void showSettings();
    void copyMusic(QString, QStringList, QString) const;
    QString getLanguage();
    void showLoading();
	void stopLoading();
public slots:
    void on_actionRemote_triggered(bool);
    void on_actionLocal_triggered(bool);
	void on_actionSettings_triggered(bool);
    void on_copyToRemote_clicked();
    void on_copyToLocal_clicked();
    void on_actionDupeLocal_triggered(bool);
    void on_actionDupeRemote_triggered(bool);
	void on_refreshLocal_clicked();
	void on_refreshRemote_clicked();
	void on_searchLocal_returnPressed();
	void on_searchRemote_returnPressed();
    void saveSettings(Settings::set);
    void on_actionAbout_triggered(bool);
	void on_actionExit_triggered(bool);
};
// QT_END_NAMESPACE