/**
 * @file MSTSettingsManager.h
 * @brief 音乐同步工具设置管理器类定义
 * @details 负责应用程序设置的持久化操作，包括从JSON文件加载设置、
 *          保存设置到JSON文件、创建默认设置以及日志文件管理。
 *          从MSTMainWindow中提取出设置I/O和日志管理职责
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTSETTINGSMANAGER_H
#define MSTSETTINGSMANAGER_H
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "../Core/SettingEntity.h"

/**
 * @brief 设置管理器类
 * 封装应用程序设置的加载、保存和日志文件管理功能，
 * 将设置持久化逻辑从主窗口中分离出来
 */
class MSTSettingsManager {
public:
	/**
	 * @brief 从settings.json文件加载设置
	 * @param entity 输出的设置实体引用
	 * @return 成功加载返回true，否则返回false（将创建默认设置）
	 */
	static bool loadSettings(set& entity);

	/**
	 * @brief 保存设置到settings.json文件
	 * @param entity 要保存的设置实体
	 * @return 成功保存返回true，否则返回false
	 */
	static bool saveSettings(const set& entity);

	/**
	 * @brief 创建默认设置文件
	 * @details 当settings.json不存在或格式错误时，创建包含默认值的设置文件
	 */
	static void createDefaultSettings();

	/**
	 * @brief 从日志文件获取上次扫描时间
	 * @param logFilePath 日志文件路径
	 * @return 上次扫描的时间，如果没有找到则返回1970年1月1日
	 */
	static QDateTime getDateFromLog(const QString& logFilePath);

	/**
	 * @brief 将扫描时间写入日志文件
	 * @param logFilePath 日志文件路径
	 * @param dateTime 要记录的时间
	 */
	static void writeLog(const QString& logFilePath, const QDateTime& dateTime);

	/**
	 * @brief 构建扫描日志文件名
	 * @param path 音乐文件夹路径
	 * @return 完整的日志文件路径
	 */
	static QString buildLogFileName(const QString& path);

	/**
	 * @brief 清理日志目录下的所有日志文件
	 */
	static void cleanLog();
};

#endif // MSTSETTINGSMANAGER_H
