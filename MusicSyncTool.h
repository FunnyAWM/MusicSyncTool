#ifndef MUSICSYNCTOOL_H
#define MUSICSYNCTOOL_H

#include <QMainWindow>
#include <QFile>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlDriver>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileDialog>
#include <QJsonObject>
#include <QDir>
#include <QSet>
#include <QString>
#include <QMessageBox>
#include <QTranslator>
#include <QtConcurrent>
#include <QFuture>
#include <QThread>
#include <QThreadPool>
#include "ui_MusicSyncTool.h"
#include "AboutPage.h"
#include "LoadingPage.h"
#include "ShowDupe.h"
#include "Settings.h"
#include "CopyResult.h"
#include "MusicProperties.h"

using namespace TOOLPROPERTIES;

class MusicSyncTool : public QMainWindow
{
    Q_OBJECT

    Ui::MusicSyncToolClass ui;
    QTranslator *translator;
    QString localPath;
    QString remotePath;
    QSqlDatabase dbLocal;
    QSqlDatabase dbRemote;
    QSqlQuery queryLocal;
    QSqlQuery queryRemote;
	LoadingPage* loading = new LoadingPage();
    set entitySave;
    QMediaPlayer* mediaPlayer;
    QAudioOutput* audioOutput;
    QString nowPlaying;
    QStringList errorList;
    QStringList supportedFormat = {".mp3", ".flac", ".ape", ".wav", ".wma"};
public:
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();
    void initDatabase();
    void loadSettings();
    void initMediaPlayer();
    void loadLanguage();
    void initUI();
    void popError(PET);
    void setMediaWidget(playState);
    void openFolder(pathType);
    void getMusic(pathType);
    void getMusicConcurrent(pathType);
    void searchMusic(pathType, QString);
    void addToErrorList(QString, fileErrorType);
    QStringList getDuplicatedMusic(pathType);
    QStringList getSelectedMusic(pathType);
    void showSettings();
    void copyMusic(QString, QStringList, QString);
    void setNowPlayingTitle(QString);
    QString getLanguage();
    void setTotalLength(pathType, int);
    void getFavoriteMusic(pathType);
    void connectSlots();
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
    void on_tableWidgetLocal_cellDoubleClicked(int,int);
    void on_tableWidgetRemote_cellDoubleClicked(int, int);
    void saveSettings(set);
    void on_actionAbout_triggered(bool);
	void on_actionExit_triggered(bool);
    void on_playControl_clicked();
    void on_playSlider_sliderMoved(int);
    void on_playSlider_sliderPressed();
    void on_volumeSlider_sliderMoved(int);
    void on_volumeSlider_sliderPressed();
    void on_favoriteOnlyLocal_clicked();
    void on_favoriteOnlyRemote_clicked();
    void endMedia(QMediaPlayer::PlaybackState);
    void setSliderPosition(qint64);
    void showCopyResult();
signals:
    void total(int total);
    void current(int current);
    void finished();
    void copyFinished();
};
// QT_END_NAMESPACE
#endif // MUSICSYNCTOOL_H
