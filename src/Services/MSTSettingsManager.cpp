#include "MSTSettingsManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <utility>

#include "Logger.h"
#include "../Data/LyricIgnoreRule.h"

namespace {
	/// @brief 测试注入的基准目录；为空时使用 applicationDirPath()
	QString configuredBaseDir;
}

QString MSTSettingsManager::baseDir() {
	if (configuredBaseDir.isEmpty()) {
		return QCoreApplication::applicationDirPath();
	}
	return configuredBaseDir;
}

void MSTSettingsManager::setBaseDir(const QString& dir) {
	configuredBaseDir = dir;
}

QString MSTSettingsManager::settingsFilePath() {
	return QDir(baseDir()).filePath("settings.json");
}

QString MSTSettingsManager::logDirPath() {
	return QDir(baseDir()).filePath("log");
}

void MSTSettingsManager::createDefaultSettings() {
	QFile file(settingsFilePath());
	if (!file.open(QIODevice::WriteOnly)) {
		Logger::Error("Failed to create default settings file");
		return;
	}
	// 默认值集中取自 SettingsData 的成员默认值，保证结构体与文件口径一致
	const SettingsData defaults;
	QJsonObject obj;
	obj["ignoreLyric"] = defaults.ignoreLyric;
	obj["sortBy"] = defaults.sortBy;
	obj["orderBy"] = defaults.orderBy;
	obj["language"] = defaults.language;
	obj["favoriteTag"] = defaults.favoriteTag;
	obj["rules"] = QJsonArray();
	obj["recursiveScan"] = defaults.recursiveScan;
	obj["themeMode"] = defaults.themeMode;
	obj["accentColor"] = defaults.accentColor;
	obj["logToFile"] = defaults.logToFile;
	file.write(QJsonDocument(obj).toJson());
	file.close();
}

bool MSTSettingsManager::loadSettings(SettingsData& entity) {
	QFile file(settingsFilePath());
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
	entity.language = obj["language"].toString();
	entity.favoriteTag = obj["favoriteTag"].toString();
	entity.recursiveScan = obj["recursiveScan"].toBool();
	// 旧版本设置文件可能缺少键：缺键时保留结构体默认值，避免编码被置成 0 等非法值
	entity.sortBy = obj.contains("sortBy") ? static_cast<short>(obj["sortBy"].toInt()) : entity.sortBy;
	entity.orderBy = obj.contains("orderBy") ? static_cast<short>(obj["orderBy"].toInt()) : entity.orderBy;
	entity.themeMode = obj.contains("themeMode") ? static_cast<short>(obj["themeMode"].toInt()) : entity.themeMode;
	entity.accentColor = obj.contains("accentColor") ? static_cast<short>(obj["accentColor"].toInt()) : entity.accentColor;
	entity.logToFile = obj["logToFile"].toBool();
	QList<LyricIgnoreRule> rules;
	QJsonArray rulesArray = obj["rules"].toArray();
	for (const QJsonValue& rule : std::as_const(rulesArray)) {
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

bool MSTSettingsManager::saveSettings(const SettingsData& entity) {
	QFile file(settingsFilePath());
	if (!file.open(QIODevice::WriteOnly)) {
		// 设置写入失败属于可恢复错误，不应终止进程
		Logger::Error("Error opening settings file");
		return false;
	}
	QJsonObject obj;
	obj["ignoreLyric"] = entity.ignoreLyric;
	obj["sortBy"] = entity.sortBy;
	obj["orderBy"] = entity.orderBy;
	obj["language"] = entity.language;
	obj["favoriteTag"] = entity.favoriteTag;
	obj["recursiveScan"] = entity.recursiveScan;
	obj["themeMode"] = entity.themeMode;
	obj["accentColor"] = entity.accentColor;
	obj["logToFile"] = entity.logToFile;
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

QDateTime MSTSettingsManager::getDateFromLog(const QString& logFilePath) {
	QFile file(logFilePath);
	QDateTime dateTime;
	const QString logDir = logDirPath();
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

void MSTSettingsManager::writeLog(const QString& logFilePath, const QDateTime& dateTime) {
	QFile file(logFilePath);
	QTextStream out(&file);
	if (file.open(QIODevice::WriteOnly)) {
		out << dateTime.toString();
	}
	file.close();
}

/**
 * @details 根据音乐文件夹路径生成唯一的日志文件名，
 *          格式为 "lastScan - <路径片段>.log"
 */
QString MSTSettingsManager::buildLogFileName(const QString& path) {
	// 先归一化分隔符，避免 Windows 的原生反斜杠被当作单个路径片段
	QStringList pathForLog = QDir::fromNativeSeparators(path).split("/", Qt::SkipEmptyParts);
	QString logFileName = "lastScan";
	if (!pathForLog.isEmpty()) {
		pathForLog[0].remove(":");
	}
	for (const QString& pathSegment : std::as_const(pathForLog)) {
		logFileName += " - " + pathSegment;
	}
	logFileName += ".log";
	return QDir(logDirPath()).filePath(logFileName);
}

void MSTSettingsManager::cleanLog() {
	const QString logDir = logDirPath();
	const QDir dir(logDir);
	if (!dir.exists()) {
		return;
	}
	const QStringList logFiles = dir.entryList(QDir::Files);
	for (const QString& logFile : logFiles) {
		QFile file(dir.filePath(logFile));
		file.remove();
	}
}
