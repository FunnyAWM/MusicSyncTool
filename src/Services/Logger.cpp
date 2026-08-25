#include "Logger.h"
#include <QDateTime>
#include <cstdlib>

PROPERTIES::LogToFile Logger::logToFile = PROPERTIES::LogToFile::DISABLE;


void Logger::Info(const QString& message) {
    qInfo().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[INFO]" << message;
}

void Logger::Warn(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[WARN]" << message;
}

void Logger::Error(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[ERROR]" << message;
}

void Logger::Fatal(const QString& message) {
    qCritical().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[FATAL]" << message;
    std::exit(EXIT_FAILURE);
}

void Logger::Debug(const QString& message) {
    qDebug().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[DEBUG]" << message;
}
