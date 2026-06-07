/**
 * @file MSTMainWindow.h
 * @brief 音乐同步工具主窗口类定义
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
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
#include "../../Services/MSTDataSource.h"
#include "../../Services/MSTFileManager.h"
#include "../../Services/MSTMediaPlayer.h"
#include "../../Core/MusicProperties.h"
#include "../../Services/MSTSettingsManager.h"
#include "../../Services/MSTScanController.h"
#include "../OperationResult/OperationResult.h"
#include "../Settings/Settings.h"
#include "../ShowDupe/ShowDupe.h"
#include "MSTErrorReporter.h"
#include "MSTMediaController.h"
#include "MSTTableManager.h"
#include "ui_MSTMainWindow.h"

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
 * 
 * 设置持久化和日志管理已委托给MSTSettingsManager处理
 */
class MSTMainWindow final : public QMainWindow {
	Q_OBJECT
	
private:
	// UI相关成员变量
	Ui::MSTMainWindowClass ui;              ///< 主窗口UI对象
	QTranslator* translator;                ///< 翻译器，用于多语言支持
	LoadingPage* loading;                   ///< 加载进度页面
	
	// 数据源和文件管理
	MSTDataSource local;                    ///< 本地音乐数据源
	MSTDataSource remote;                   ///< 远程音乐数据源
	shared_ptr<MSTFileManager> localManager;  ///< 本地文件管理器
	shared_ptr<MSTFileManager> remoteManager; ///< 远程文件管理器
	
	// 表格管理器
	MSTTableManager* tableManager;          ///< 表格分页与显示管理器
	
	// 扫描控制器
	MSTScanController* scanController;      ///< 音乐扫描控制器
	
	// 错误报告器
	MSTErrorReporter* errorReporter;        ///< 错误报告与结果展示
	
	// 媒体播放相关
	shared_ptr<MSTMediaPlayer> player;      ///< 音乐播放器
	
	// 媒体播放控制器
	MSTMediaController* mediaController;    ///< 媒体播放UI控制器
	
	// 状态管理
	QAtomicPointer<bool> copyStats;         ///< 复制操作状态（原子指针）
	
	// 配置和常量
	set entity;                             ///< 应用程序设置实体

public:
	/**
	 * @brief 构造函数
	 * 初始化主窗口，设置UI，加载配置和连接信号槽
	 * @param parent 父窗口指针，默认为nullptr
	 */
	explicit MSTMainWindow(QWidget* parent = nullptr);
	
	/**
	 * @brief 析构函数
	 * 清理资源，关闭数据库连接，释放内存
	 */
	~MSTMainWindow() override;
	void connectSlots();
	void copyMusic(const QString&, const QStringList&, const QString&);
	[[nodiscard]] QString getLanguage() const;
	QStringList getDuplicatedMusic(PathType);
	/**
	 * @brief 启动音乐扫描
	 * @param path 路径类型
	 * @param page 页码
	 * @details 委托给scanController执行，保持与其他模块的接口兼容
	 */
	void getMusic(PathType path, unsigned short page);
	void initDatabase();
	void initUI();
	void loadLanguage();
	void loadSettings();
	void openFolder(PathType);
	/**
	 * @brief 弹出错误对话框（委托给errorReporter）
	 * @param type 错误类型
	 */
	void popError(PET type);
	void setAvailableSpace(PathType) const;
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
