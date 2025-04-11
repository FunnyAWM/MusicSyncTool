//
// Created by funnyawm on 25-4-11.
//
#include "Logger.h"
#include <QDateTime>
void Logger::Info(const QString& message) {
    qInfo().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[INFO]" << message << "\n";
}

void Logger::Warn(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[WARN]" << message << "\n";
}

void Logger::Error(const QString& message) {
    qWarning().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[ERROR]" << message
               << "\n";
}

void Logger::Fatal(const QString& message) {
    qFatal().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[FATAL]" << message << "\n";
}

void Logger::Debug(const QString& message) {
    qDebug().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << "[DEBUG]" << message << "\n";
}