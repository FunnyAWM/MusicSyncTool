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
	static void createDefaultSettings();

	static bool loadSettings(SettingsData& entity);

	static bool saveSettings(const SettingsData& entity);

	static QDateTime getDateFromLog(const QString& logFilePath);

	static void writeLog(const QString& logFilePath, const QDateTime& dateTime);

	static QString buildLogFileName(const QString& path);

	static void cleanLog();
};

#endif // MSTSETTINGSMANAGER_H
