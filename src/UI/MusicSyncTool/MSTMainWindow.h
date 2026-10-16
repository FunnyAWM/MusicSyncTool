// ReSharper disable CppUnusedIncludeDirective
#ifndef MUSICSYNCTOOL_H
#define MUSICSYNCTOOL_H

// 标准库头文件
#include <atomic>
#include <memory>

// Qt核心模块
#include <QAudioOutput>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
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
#include "MSTErrorReporter.h"
#include "MSTMediaController.h"
#include "MSTTableManager.h"
#include "ui_MSTMainWindow.h"
#include "../../Core/MusicProperties.h"
#include "../../Services/MSTDataSource.h"
#include "../../Services/MSTFileManager.h"
#include "../../Services/MSTMediaPlayer.h"
#include "../../Services/MSTScanController.h"
#include "../../Services/MSTSettingsManager.h"
#include "../AboutPage/AboutPage.h"
#include "../LoadingPage/LoadingPage.h"
#include "../OperationResult/OperationResult.h"
#include "../Settings/Settings.h"
#include "../ShowDupe/ShowDupe.h"

// 命名空间使用声明
using PROPERTIES::FileErrorType;
using PROPERTIES::LoadErrorType;
using PROPERTIES::OperationType;
using PROPERTIES::PathType;
using PROPERTIES::AppErrorType;
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
 * 
 * 设置持久化和日志管理已委托给MSTSettingsManager处理
 */
class MSTMainWindow final : public QMainWindow {
	Q_OBJECT
	
private:
	// UI相关成员变量
	Ui::MSTMainWindowClass ui;
	QTranslator* translator;
	LoadingPage* loading;
	
	// 数据源和文件管理
	MSTDataSource local;
	MSTDataSource remote;
	shared_ptr<MSTFileManager> localManager;
	shared_ptr<MSTFileManager> remoteManager;
	
	// 表格管理器
	MSTTableManager* tableManager;
	
	// 扫描控制器
	MSTScanController* scanController;
	
	// 错误报告器
	MSTErrorReporter* errorReporter;
	
	// 媒体播放相关
	shared_ptr<MSTMediaPlayer> player;
	
	// 媒体播放控制器
	MSTMediaController* mediaController;

	// 状态管理
	/// @brief 复制任务进行中标记：工作线程与 GUI 线程都会访问
	std::atomic<bool> copyOperationInProgress{false};
	/// @brief 持有复制任务的 QFuture，保证窗口析构前任务已结束
	QFutureWatcher<void> copyWatcher;

	// 配置和常量
	SettingsData entity;

public:
	explicit MSTMainWindow(QWidget* parent = nullptr);

	~MSTMainWindow() override;
	void initDatabase();
	void initUI();
	void loadSettings();
	void loadLanguage();
	void connectSlots();
	void openFolder(PathType);
	void getMusic(PathType path, unsigned short page) const;
	void popError(AppErrorType type) const;
	QStringList getDuplicatedMusic(PathType);
	void showSettings();

	/**
	 * @brief 按设置拼接 base → mode → accent 三段样式表并应用到整个应用
	 * @details 启动时在创建主窗口之前调用，切换主题时即时重载
	 */
	static void applyTheme(const SettingsData& entity);
	void copyMusic(const QString&, const QStringList&, const QString&);
	/// @brief 启动复制任务并防止并发复制
	void startCopy(const QString& source, const QStringList& fileList, const QString& target);
	[[nodiscard]] QString getLanguage() const;
	void setAvailableSpace(PathType) const;

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
	void on_favoriteOnlyLocal_clicked() const;
	void on_favoriteOnlyRemote_clicked() const;
	void on_lastPageLocal_clicked() const;
	void on_lastPageRemote_clicked() const;
	void on_nextPageLocal_clicked() const;
	void on_nextPageRemote_clicked() const;
	void on_playControl_clicked() const;
	void on_playSlider_sliderMoved(int) const;
	void on_playSlider_sliderPressed() const;
	void on_refreshLocal_clicked();
	void on_refreshRemote_clicked();
	void on_searchLocal_returnPressed() const;
	void on_searchRemote_returnPressed() const;
	void on_tableWidgetLocal_cellDoubleClicked(int, int) const;
	void on_tableWidgetRemote_cellDoubleClicked(int, int) const;
	void on_volumeSlider_sliderMoved(int) const;
	void on_volumeSlider_sliderPressed() const;
	void on_volumeSlider_valueChanged(int) const;
	void on_copyFinished(OperationType);
	void saveSettings(const SettingsData&);

	/**
	 * @brief 在 GUI 线程刷新音乐表格
	 * @details 扫描任务在工作线程完成取数，表格控件只能在 GUI 线程更新
	 */
	void applyTablePage(PathType path, QList<QueryItem> items, unsigned short page, unsigned short totalPages);

signals:
	void addToErrorListConcurrent(QString, LoadErrorType);
	void copyFinished(OperationType);
	/// @brief 复制过程中的文件错误，经排队连接回到 GUI 线程登记
	void fileCopyError(QString fileName, FileErrorType error);
	void current(int current);
	void finished();
	void loadFinished(OperationType);
	void started();
	void total(int total);
};

// QT_END_NAMESPACE
#endif // MUSICSYNCTOOL_H
