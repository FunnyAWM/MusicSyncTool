/**
 * @file MSTSettingsManager.cpp
 * @brief 音乐同步工具设置管理器类的实现
 * @details 实现设置的JSON持久化、默认设置创建和日志文件管理功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTSettingsManager.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QTextStream>

#include "Logger.h"
#include "../Data/LyricIgnoreRule.h"

/**
 * @brief 创建默认设置文件
 */
void MSTSettingsManager::createDefaultSettings() {
	QFile file("settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		Logger::Error("Failed to create default settings file");
		return;
	}
	QJsonObject obj;
	obj["ignoreLyric"] = false;
	obj["sortBy"] = PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE);
	obj["orderBy"] = PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC);
	obj["language"] = "";
	obj["favoriteTag"] = "";
	obj["rules"] = QJsonArray();
	obj["recursiveScan"] = false;
	file.write(QJsonDocument(obj).toJson());
	file.close();
}

/**
 * @brief 从settings.json文件加载设置
 * @param entity 输出的设置实体引用
 * @return 成功加载返回true，否则返回false
 */
bool MSTSettingsManager::loadSettings(SettingsData& entity) {
	QFile file("settings.json");
	if (!file.open(QIODevice::ReadOnly)) {
		Logger::Warn("No settings file found, creating default setting file named settings.json");
		file.close();
		createDefaultSettings();
		return false;
	}
	const QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
	if (settings.isNull()) {
		Logger::Warn("No settings file found, creating default setting file named settings.json");
		file.close();
		createDefaultSettings();
		return false;
	}
	QJsonObject obj = settings.object();
	entity.ignoreLyric = obj["ignoreLyric"].toBool();
	entity.sortBy = static_cast<short>(obj["sortBy"].toInt());
	entity.orderBy = static_cast<short>(obj["orderBy"].toInt());
	entity.language = obj["language"].toString();
	entity.favoriteTag = obj["favoriteTag"].toString();
	entity.recursiveScan = obj["recursiveScan"].toBool();
	QList<LyricIgnoreRule> rules;
	QJsonArray rulesArray = obj["rules"].toArray();
	for (QJsonValue rule : rulesArray) {
		QJsonObject ruleObj = rule.toObject();
		if (ruleObj["ruleType"].isNull() || ruleObj["ruleField"].isNull() || ruleObj["ruleName"].isNull()) {
			Logger::Warn("Invalid rule found in settings file, skipping");
			continue;
		}
		rules.append(LyricIgnoreRule(LyricIgnoreRule::stringToIgnoreRules(ruleObj["ruleType"].toString()),
		                             LyricIgnoreRule::stringToLyricRules(ruleObj["ruleField"].toString()),
		                             ruleObj["ruleName"].toString()));
	}
	entity.rules = rules;
	file.close();
	return true;
}

/**
 * @brief 保存设置到settings.json文件
 * @param entity 要保存的设置实体
 * @return 成功保存返回true，否则返回false
 */
bool MSTSettingsManager::saveSettings(const SettingsData& entity) {
	QFile file("settings.json");
	if (!file.open(QIODevice::WriteOnly)) {
		Logger::Fatal("Error opening settings file");
		return false;
	}
	QJsonObject obj;
	obj["ignoreLyric"] = entity.ignoreLyric;
	obj["sortBy"] = entity.sortBy;
	obj["orderBy"] = entity.orderBy;
	obj["language"] = entity.language;
	obj["favoriteTag"] = entity.favoriteTag;
	obj["recursiveScan"] = entity.recursiveScan;
	QJsonArray rulesArray;
	for (const auto& rule : entity.rules) {
		QJsonObject ruleObj;
		ruleObj["ruleType"] = rule.getRuleTypeStr();
		ruleObj["ruleField"] = rule.getRuleFieldStr();
		ruleObj["ruleName"] = rule.getRuleName();
		rulesArray.append(ruleObj);
	}
	obj["rules"] = rulesArray;
	QJsonDocument settings;
	settings.setObject(obj);
	file.write(settings.toJson());
	file.close();
	return true;
}

/**
 * @brief 从日志文件获取上次扫描时间
 * @param logFilePath 日志文件路径
 * @return 上次扫描的时间
 */
QDateTime MSTSettingsManager::getDateFromLog(const QString& logFilePath) {
	QFile file(logFilePath);
	QDateTime dateTime;
	const QString logDir = QCoreApplication::applicationDirPath() + "/log";
	const QDir dir(logDir);
	if (!dir.exists()) {
		// ReSharper disable once CppExpressionWithoutSideEffects
		dir.mkpath(logDir);
	}
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream in(&file);
		dateTime = QDateTime::fromString(in.readLine());
		if (dateTime.isNull()) {
			Logger::Warn("No last scan log found, scanning all files");
			dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
		}
	}
	else {
		Logger::Warn(file.errorString());
		Logger::Warn("No last scan log found, scanning all files");
		// Set last scan time to 1970-01-01 00:00:00 if no log found
		dateTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
	}
	return dateTime;
}

/**
 * @brief 将扫描时间写入日志文件
 * @param logFilePath 日志文件路径
 * @param dateTime 要记录的时间
 */
void MSTSettingsManager::writeLog(const QString& logFilePath, const QDateTime& dateTime) {
	QFile file(logFilePath);
	QTextStream out(&file);
	if (file.open(QIODevice::WriteOnly)) {
		out << dateTime.toString();
	}
	file.close();
}

/**
 * @brief 构建扫描日志文件名
 * @param path 音乐文件夹路径
 * @return 完整的日志文件路径
 * @details 根据音乐文件夹路径生成唯一的日志文件名，
 *          格式为 "lastScan - <路径片段>.log"
 */
QString MSTSettingsManager::buildLogFileName(const QString& path) {
	QStringList pathForLog = path.split("/");
	QString logFilePath = "lastScan";
	if (!pathForLog[0].isEmpty()) {
		pathForLog[0].remove(":");
	}
	else {
		pathForLog.removeFirst();
	}
	for (const QString& pathSegment : pathForLog) {
		logFilePath += " - " + pathSegment;
	}
	logFilePath += ".log";
	logFilePath = QCoreApplication::applicationDirPath() + "/log/" + logFilePath;
	return logFilePath;
}

/**
 * @brief 清理日志目录下的所有日志文件
 */
void MSTSettingsManager::cleanLog() {
	const QDir logDir("log");
	if (!logDir.exists()) {
		return;
	}
	QStringList logFiles = logDir.entryList(QDir::Files);
	for (QString& logFile : logFiles) {
		QFile file("log/" + logFile);
		file.remove();
	}
}
