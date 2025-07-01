//
// Created by funnyawm on 25-4-11.
//
#include "Logger.h"
#include <QDateTime>
#include <QCoreApplication>

PROPERTIES::LogToFile Logger::logToFile = PROPERTIES::LogToFile::DISABLE;
const QString Logger::logFileName = QCoreApplication::applicationDirPath() + "/log/lastRun.log";

/**
 * @brief 记录信息级别日志
 * @param message 要记录的日志消息
 */
void Logger::Info(const QString& message) {
    qInfo().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[INFO]" << message;
}

/**
 * @brief 记录警告级别日志
 * @param message 要记录的日志消息
 */
void Logger::Warn(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[WARN]" << message;
}

/**
 * @brief 记录错误级别日志
 * @param message 要记录的日志消息
 */
void Logger::Error(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[ERROR]" << message;
}

/**
 * @brief 记录致命错误级别日志
 * @param message 要记录的日志消息
 */
void Logger::Fatal(const QString& message) {
    qFatal().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[FATAL]" << message;
}

/**
 * @brief 记录调试级别日志
 * @param message 要记录的日志消息
 */
void Logger::Debug(const QString& message) {
    qDebug().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[DEBUG]" << message;
}