/**
 * @file MSTScanController.h
 * @brief 音乐扫描控制器类定义
 * @details 封装音乐文件扫描、数据库同步和并发执行逻辑，
 *          将扫描编排职责从主窗口中分离出来
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTSCANCONTROLLER_H
#define MSTSCANCONTROLLER_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include "MSTTagScanner.h"
#include "../Core/MusicProperties.h"
#include "../Core/SettingEntity.h"
#include "../Data/LyricIgnoreRule.h"

class MSTDataSource;
class MSTTableManager;

using PROPERTIES::LoadErrorType;
using PROPERTIES::AppErrorType;
using PROPERTIES::PathType;

/**
 * @brief 音乐扫描控制器
 * @details 负责音乐文件目录扫描、文件差异比对、数据库同步、
 *          收藏标签和规则命中设置。通过QtConcurrent在后台线程执行，
 *          通过信号通知UI更新进度
 */
class MSTScanController : public QObject {
	Q_OBJECT

public:
	explicit MSTScanController(MSTDataSource& localDataSource,
	                           MSTDataSource& remoteDataSource,
	                           MSTTableManager* tableManager,
	                           QObject* parent = nullptr);

private:
	MSTDataSource& localDataSource; ///< 本地数据源引用
	MSTDataSource& remoteDataSource; ///< 远程数据源引用
	MSTTableManager* tableManager; ///< 表格管理器指针
	MSTTagScanner tagScanner; ///< 标签扫描器
	bool scanned[2] = {false, false}; ///< 扫描完成标记 [0:本地, 1:远程]

	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	[[nodiscard]] QString getPath(PathType path) const;

public:
	void startScan(PathType path, unsigned short page, const SettingsData& entity);

	void scanConcurrent(PathType path, unsigned short page, const SettingsData& entity);

	void loadPage(PathType path, unsigned short page, const SettingsData& entity);

	void setFavorite(PathType path, const QStringList& filesToScan, const QString& favoriteTag,
	                 qsizetype progressOffset);

	void setRuleHit(PathType path, const QList<LyricIgnoreRule>& rules,
	                const QStringList& filesToFilter, qsizetype progressOffset);

	[[nodiscard]] MSTTagScanner* getTagScanner();

	void resetScan(PathType path);

signals:
	/**
	 * @brief 扫描开始信号（用于显示加载页面）
	 */
	void scanStart();

	/**
	 * @brief 扫描完成信号（用于关闭加载页面）
	 */
	void scanFinished();

	/**
	 * @brief 扫描总数量信号
	 * @param total 总文件数
	 */
	void scanTotal(qsizetype total);

	/**
	 * @brief 扫描当前进度信号
	 * @param current 当前处理文件索引
	 */
	void scanCurrent(qsizetype current);

	/**
	 * @brief 错误信号
	 * @param type 错误类型
	 */
	void errorOccurred(AppErrorType type);

	/**
	 * @brief 加载错误信号（跨线程）
	 * @param file 文件名
	 * @param error 加载错误类型
	 */
	void loadErrorOccurred(QString file, LoadErrorType error);
};

#endif // MSTSCANCONTROLLER_H
