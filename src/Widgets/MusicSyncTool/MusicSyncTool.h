/**
 * @file MusicSyncTool.h
 * @brief 音乐同步工具主窗口类定义
 * @author FunnyAWM
 * @date 创建日期
 * @version 2.3.0
 * 
 * 这是音乐同步工具的主窗口类，负责整个应用程序的核心功能，包括：
 * - 音乐文件的扫描、加载和显示
 * - 本地和远程路径之间的音乐文件同步
 * - 音乐播放器控制和预览功能
 * - 重复音乐检测和处理
 * - 用户界面交互和设置管理
 * - 多线程文件操作和进度显示
 */

// ReSharper disable CppUnusedIncludeDirective
#ifndef MUSICSYNCTOOL_H
#define MUSICSYNCTOOL_H

// 标准库头文件
#include <memory>

// Qt核心模块
#include <QAudioOutput>
#include <QAtomicInt>
#include <QAtomicPointer>
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
#include <QStorageInfo>
#include <QString>
#include <QtConcurrentRun>
#include <QThread>
#include <QThreadPool>
#include <QTranslator>

// TagLib音频元数据库
#include <taglib/tag.h>

// 项目内部头文件
#include "../AboutPage/AboutPage.h"
#include "../LoadingPage/LoadingPage.h"
#include "../../MSTDataSource.h"
#include "../../MSTFileManager.h"
#include "../../MSTMediaPlayer.h"
#include "../../MusicProperties.h"
#include "../OperationResult/OperationResult.h"
#include "../Settings/Settings.h"
#include "../ShowDupe/ShowDupe.h"
#include "ui_MusicSyncTool.h"
#include "../../MSTFileManager.h"

// 命名空间使用声明
using PROPERTIES::FileErrorType;
using PROPERTIES::LoadErrorType;
using PROPERTIES::OperationType;
using PROPERTIES::PathType;
using PROPERTIES::PET;
using PROPERTIES::PlayState;
using std::shared_ptr;

/**
 * @brief 音乐同步工具主窗口类
 * 
 * 这是应用程序的核心类，继承自QMainWindow，提供完整的音乐文件管理功能。
 * 主要功能包括：
 * - 本地和远程音乐库的管理和同步
 * - 音乐文件的预览和播放控制
 * - 重复音乐检测和清理
 * - 多线程文件操作和进度显示
 * - 用户设置管理和多语言支持
 */
class MusicSyncTool final : public QMainWindow {
	Q_OBJECT
	
private:
	// UI相关成员变量
	Ui::MusicSyncToolClass ui;              ///< 主窗口UI对象
	QTranslator* translator;                ///< 翻译器，用于多语言支持
	LoadingPage* loading;                   ///< 加载进度页面
	
	// 数据源和文件管理
	MSTDataSource local;                    ///< 本地音乐数据源
	MSTDataSource remote;                   ///< 远程音乐数据源
	shared_ptr<MSTFileManager> localManager;  ///< 本地文件管理器
	shared_ptr<MSTFileManager> remoteManager; ///< 远程文件管理器
	
	// 媒体播放相关
	shared_ptr<MSTMediaPlayer> player;      ///< 音乐播放器
	QString nowPlaying;                     ///< 当前播放的音乐文件名
	
	// 分页和状态管理
	unsigned short currentPage[2];          ///< 当前页码 [0:本地, 1:远程]
	unsigned short totalPage[2];            ///< 总页数 [0:本地, 1:远程]
	bool favoriteOnly[2];                   ///< 仅显示收藏 [0:本地, 1:远程]
	QAtomicPointer<bool> copyStats;         ///< 复制操作状态（原子指针）
	
	// 配置和常量
	const short PAGESIZE = 200;             ///< 每页显示的音乐数量
	set entity;                             ///< 应用程序设置实体
	QStringList errorList;                  ///< 错误信息列表

public:
	/**
	 * @brief 构造函数
	 * 初始化主窗口，设置UI，加载配置和连接信号槽
	 * @param parent 父窗口指针，默认为nullptr
	 */
	explicit MusicSyncTool(QWidget* parent = nullptr);
	
	/**
	 * @brief 析构函数
	 * 清理资源，关闭数据库连接，释放内存
	 */
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
	void setFavorite(PathType, const TagLib::String&, const QDateTime&);
	void setRuleHit(PathType, const QList<LyricIgnoreRule>&, const QDateTime&);
	static QDateTime getDateFromLog(const QString&);
	static void writeLog(const QString&, const QDateTime&);

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
	void on_copyFinished(OperationType) const;
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
