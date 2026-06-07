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

#include "../Core/MusicProperties.h"
#include "../Core/SettingEntity.h"
#include "../Data/LyricIgnoreRule.h"
#include "MSTTagScanner.h"

class MSTDataSource;
class MSTTableManager;

using PROPERTIES::LoadErrorType;
using PROPERTIES::PET;
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
	/**
	 * @brief 构造函数
	 * @param localDataSource 本地数据源引用
	 * @param remoteDataSource 远程数据源引用
	 * @param tableManager 表格管理器指针
	 * @param parent 父对象指针
	 */
	explicit MSTScanController(MSTDataSource& localDataSource,
	                           MSTDataSource& remoteDataSource,
	                           MSTTableManager* tableManager,
	                           QObject* parent = nullptr);

	/**
	 * @brief 启动音乐扫描（多线程入口）
	 * @param path 路径类型（本地或远程）
	 * @param page 页码
	 * @param entity 当前设置实体
	 * @details 检查路径有效性后启动后台线程执行扫描
	 */
	void startScan(PathType path, unsigned short page, const set& entity);

	/**
	 * @brief 执行音乐扫描（并发执行体）
	 * @param path 路径类型
	 * @param page 页码
	 * @param entity 当前设置实体
	 * @details 核心扫描函数，执行以下操作：
	 *          1. 初始化数据库表和日志
	 *          2. 扫描目录获取文件列表，过滤不支持的格式
	 *          3. 比对新旧文件列表，识别增删文件
	 *          4. 删除不存在的记录，添加新文件到数据库
	 *          5. 设置收藏标签和规则命中
	 *          6. 通过表格管理器更新显示
	 */
	void scanConcurrent(PathType path, unsigned short page, const set& entity);

	/**
	 * @brief 仅加载页面数据（不执行目录扫描）
	 * @param path 路径类型
	 * @param page 页码
	 * @param entity 当前设置实体
	 * @details 从数据库读取指定页码的数据并填充表格，
	 *          用于已完成扫描后的翻页操作
	 */
	void loadPage(PathType path, unsigned short page, const set& entity);

	/**
	 * @brief 重置扫描标记
	 * @param path 路径类型
	 * @details 清除指定路径的已扫描标记，使下次getMusic调用触发完整扫描
	 */
	void resetScan(PathType path);

	/**
	 * @brief 获取标签扫描器指针
	 * @return 标签扫描器指针，用于信号连接
	 */
	[[nodiscard]] MSTTagScanner* getTagScanner();

	/**
	 * @brief 设置收藏标签
	 * @param path 路径类型
	 * @param filesToScan 需要扫描的文件名列表（增量）
	 * @param favoriteTag 收藏标签字符串
	 */
	void setFavorite(PathType path, const QStringList& filesToScan, const QString& favoriteTag);

	/**
	 * @brief 设置规则命中状态
	 * @param path 路径类型
	 * @param rules 歌词忽略规则列表
	 * @param filesToFilter 需要扫描的文件名列表（增量）
	 */
	void setRuleHit(PathType path, const QList<LyricIgnoreRule>& rules,
	                const QStringList& filesToFilter);

signals:
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
	void errorOccurred(PET type);

	/**
	 * @brief 加载错误信号（跨线程）
	 * @param file 文件名
	 * @param error 加载错误类型
	 */
	void loadErrorOccurred(QString file, LoadErrorType error);

private:
	MSTDataSource& localDataSource;  ///< 本地数据源引用
	MSTDataSource& remoteDataSource; ///< 远程数据源引用
	MSTTableManager* tableManager;   ///< 表格管理器指针
	MSTTagScanner tagScanner;        ///< 标签扫描器
	bool scanned[2] = {false, false}; ///< 扫描完成标记 [0:本地, 1:远程]

	/**
	 * @brief 获取指定路径对应的数据源
	 * @param path 路径类型
	 * @return 数据源引用
	 */
	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	/**
	 * @brief 获取指定路径对应的目录路径
	 * @param path 路径类型
	 * @return 目录路径字符串
	 */
	[[nodiscard]] QString getPath(PathType path) const;
};

#endif // MSTSCANCONTROLLER_H
