﻿#ifndef MUSICSYNCTOOL_H
#define MUSICSYNCTOOL_H

#include <QAudioOutput>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QThread>
#include <QThreadPool>
#include <QTranslator>
#include <QtConcurrent>
#include "AboutPage.h"
#include "LoadingPage.h"
#include "MusicProperties.h"
#include "OperationResult.h"
#include "Settings.h"
#include "ShowDupe.h"
#include "ui_MusicSyncTool.h"

using namespace TOOLPROPERTIES;

class MusicSyncTool : public QMainWindow {
    Q_OBJECT

    Ui::MusicSyncToolClass ui;
    QTranslator *translator;
    QString localPath;
    QString remotePath;
    QSqlDatabase dbLocal;
    QSqlDatabase dbRemote;
    QSqlQuery queryLocal;
    QSqlQuery queryRemote;
    LoadingPage *loading = new LoadingPage();
    // 0: local, 1: remote
    short currentPage[2] = {1, 1};
    short totalPage[2];
    const short PAGESIZE = 200;
    bool favoriteOnly[2] = {false, false};
    qint64 availableSpace[2];
    set entity;
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QString nowPlaying;
    QStringList errorList;
    const QStringList supportedFormat = {".mp3", ".flac", ".ape", ".wav", ".wma"};

public:
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void initDatabase();
    void loadSettings();
    void loadDefaultSettings();
    void initMediaPlayer();
    void loadLanguage();
    void initUI();
    void setMediaWidget(playState);
    void openFolder(pathType);
    void getMusic(pathType, short);
    void getMusicConcurrent(pathType, short);
    void searchMusic(pathType, QString);
    void addToErrorList(QString, fileErrorType);
    void popError(PET);
    QStringList getDuplicatedMusic(pathType);
    QStringList getSelectedMusic(pathType);
    void showSettings();
    void copyMusic(QString, QStringList, QString);
    void setNowPlayingTitle(QString);
    QString getLanguage() const;
    void setTotalLength(pathType, int);
    void getFavoriteMusic(pathType, short);
    void connectSlots() const;
    bool isFull(QString, QString);
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
    void on_tableWidgetLocal_cellDoubleClicked(int, int);
    void on_tableWidgetRemote_cellDoubleClicked(int, int);
    void saveSettings(set);
    void on_actionAbout_triggered(bool);
    void on_actionExit_triggered(bool);
    void on_actionClean_log_files_triggered(bool);
    void on_playControl_clicked();
    void on_playSlider_sliderMoved(int);
    void on_playSlider_sliderPressed();
    void on_volumeSlider_sliderMoved(int);
    void on_volumeSlider_valueChanged(int);
    void on_volumeSlider_sliderPressed();
    void on_favoriteOnlyLocal_clicked();
    void on_favoriteOnlyRemote_clicked();
    void on_lastPageLocal_clicked();
    void on_nextPageLocal_clicked();
    void on_lastPageRemote_clicked();
    void on_nextPageRemote_clicked();
    void endMedia(QMediaPlayer::PlaybackState);
    void setSliderPosition(qint64);
    void showOperationResult(operationType);
    void addToErrorList(QString, loadErrorType);
signals:
    void started();
    void total(int total);
    void current(int current);
    void finished();
    void loadFinished(operationType);
    void copyFinished(operationType);
    void addToErrorListConcurrent(QString, loadErrorType);
private slots:
};
// QT_END_NAMESPACE
#endif // MUSICSYNCTOOL_H