// ReSharper disable CppUnusedIncludeDirective
#ifndef MUSICSYNCTOOL_H
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
#include <QtConcurrent>
#include <QThread>
#include <QThreadPool>
#include <QTranslator>
#include <taglib/tag.h>

#include "AboutPage.h"
#include "LoadingPage.h"
#include "MusicProperties.h"
#include "OperationResult.h"
#include "Settings.h"
#include "ShowDupe.h"
#include "ui_MusicSyncTool.h"

using PROPERTIES::PathType;
using PROPERTIES::PlayState;
using PROPERTIES::FileErrorType;
using PROPERTIES::LoadErrorType;
using PROPERTIES::PET;
using PROPERTIES::OperationType;

class MusicSyncTool final : public QMainWindow {
	Q_OBJECT

		Ui::MusicSyncToolClass ui;
	QTranslator* translator;
	QString localPath;
	QString remotePath;
	QSqlDatabase dbLocal;
	QSqlDatabase dbRemote;
	QSqlQuery queryLocal;
	QSqlQuery queryRemote;
	LoadingPage* loading = new LoadingPage();
	// 0: local, 1: remote
	unsigned short currentPage[2] = { 1, 1 };
	unsigned short totalPage[2];
	bool favoriteOnly[2] = { false, false };
	const short PAGESIZE = 200;
	set entity;
	QMediaPlayer* mediaPlayer;
	QAudioOutput* audioOutput;
	QString nowPlaying;
	QStringList errorList;
	const QStringList supportedFormat = { "mp3", "flac", "ape", "wav", "wma" };

public:
	explicit MusicSyncTool(QWidget* parent = nullptr);
	~MusicSyncTool() override;
	void addToErrorList(const QString&, FileErrorType);
	void addToErrorList(const QString&, LoadErrorType);
	static void cleanLog();
	void connectSlots() const;
	void copyMusic(const QString&, const QStringList&, const QString&);
	void endMedia(QMediaPlayer::PlaybackState) const;
	[[nodiscard]] QString getLanguage() const;
	QStringList getDuplicatedMusic(PathType);
	QStringList getSelectedMusic(PathType);
	void getFavoriteMusic(PathType, unsigned short);
	void getMusic(PathType, unsigned short);
	void getMusicConcurrent(PathType, unsigned short);
	void initDatabase();
	void initMediaPlayer() const;
	void initUI();
	[[nodiscard]] bool isFormatSupported(const QString&) const;
	static bool isFull(const QString&, const QString&);
	static bool isRuleHit(const LyricIgnoreRuleSingleton&, const TagLib::Tag*);
	void loadLanguage();
	void loadSettings();
	static void loadDefaultSettings();
	void openFolder(PathType);
	void popError(PET);
	void searchMusic(PathType, const QString&);
	void setAvailableSpace(PathType) const;
	void setMediaWidget(PlayState) const;
	void setNowPlayingTitle(const QString&) const;
	void setSliderPosition(qint64) const;
	void setTotalLength(PathType, int);
	void showOperationResult(OperationType);
	void showSettings() const;

public slots:
	void on_actionAbout_triggered(bool);
	void on_actionClean_log_files_triggered(bool);
	void on_actionDupeLocal_triggered(bool);
	void on_actionDupeRemote_triggered(bool);
	void on_actionExit_triggered(bool);
	void on_actionLocal_triggered(bool);
	void on_actionRemote_triggered(bool);
	void on_actionSettings_triggered(bool) const;
	void on_copyToLocal_clicked();
	void on_copyToRemote_clicked();
	void on_favoriteOnlyLocal_clicked();
	void on_favoriteOnlyRemote_clicked();
	void on_lastPageLocal_clicked();
	void on_lastPageRemote_clicked();
	void on_nextPageLocal_clicked();
	void on_nextPageRemote_clicked();
	void on_playControl_clicked();
	void on_playSlider_sliderMoved(int) const;
	void on_playSlider_sliderPressed() const;
	void on_refreshLocal_clicked();
	void on_refreshRemote_clicked();
	void on_searchLocal_returnPressed();
	void on_searchRemote_returnPressed();
	void on_tableWidgetLocal_cellDoubleClicked(int, int);
	void on_tableWidgetRemote_cellDoubleClicked(int, int);
	void on_volumeSlider_sliderMoved(int) const;
	void on_volumeSlider_sliderPressed() const;
	void on_volumeSlider_valueChanged(int) const;
	void saveSettings(const set&);

signals:
	void addToErrorListConcurrent(QString, LoadErrorType);
	void copyFinished(OperationType);
	void current(qsizetype current);
	void finished();
	void loadFinished(OperationType);
	void started();
	void total(qsizetype total);
};

// QT_END_NAMESPACE
#endif // MUSICSYNCTOOL_H