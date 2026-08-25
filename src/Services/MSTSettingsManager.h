#ifndef MSTSETTINGSMANAGER_H
#define MSTSETTINGSMANAGER_H
#include <QJsonObject>
#include <QString>

#include "../Core/SettingEntity.h"

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
