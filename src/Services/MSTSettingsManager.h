#ifndef MSTSETTINGSMANAGER_H
#define MSTSETTINGSMANAGER_H
#include <QJsonObject>
#include <QString>

#include "../Core/SettingEntity.h"

class MSTSettingsManager {
public:
	/**
	 * @brief 运行时文件（settings.json、log/）的基准目录
	 * @details 默认使用 QCoreApplication::applicationDirPath()，避免依赖进程工作目录；
	 *          测试可通过 setBaseDir 重定向到临时目录
	 */
	[[nodiscard]] static QString baseDir();

	/// @brief 重定向基准目录；传入空字符串恢复为 applicationDirPath()
	static void setBaseDir(const QString& dir);

	/// @brief 设置文件的绝对路径
	[[nodiscard]] static QString settingsFilePath();

	/// @brief 日志目录的绝对路径
	[[nodiscard]] static QString logDirPath();

	static void createDefaultSettings();

	static bool loadSettings(SettingsData& entity);

	static bool saveSettings(const SettingsData& entity);

	static QDateTime getDateFromLog(const QString& logFilePath);

	static void writeLog(const QString& logFilePath, const QDateTime& dateTime);

	static QString buildLogFileName(const QString& path);

	static void cleanLog();
};

#endif // MSTSETTINGSMANAGER_H
