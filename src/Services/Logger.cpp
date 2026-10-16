#include "Logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTextStream>
#include <cstdlib>

#include "MSTSettingsManager.h"

namespace {
	/// @brief 日志时间戳格式，与控制台输出保持一致
	const QString timestampFormat = "yyyy-MM-dd hh:mm:ss:zzz";

	bool logToFileEnabled = false;
	QMutex logMutex;
	QtMessageHandler previousHandler = nullptr;

	/// @brief 将一行日志追加写入 <baseDir>/log/lastRun.log；失败时静默降级为仅控制台输出
	void appendToLogFile(const QString& line) {
		const QMutexLocker locker(&logMutex);
		const QDir logDir(MSTSettingsManager::logDirPath());
		if (!logDir.exists() && !logDir.mkpath(".")) {
			return;
		}
		QFile file(logDir.filePath("lastRun.log"));
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
			return;
		}
		QTextStream out(&file);
		out << line << '\n';
		file.close();
	}

	/// @brief 统一的 Qt 消息处理器：按需落盘，并继续交给上一个处理器输出到控制台
	void messageHandler(const QtMsgType type, const QMessageLogContext& context, const QString& message) {
		if (logToFileEnabled) {
			QString level;
			switch (type) {
			case QtDebugMsg: level = "DEBUG";
				break;
			case QtInfoMsg: level = "INFO";
				break;
			case QtWarningMsg: level = "WARN";
				break;
			case QtCriticalMsg: level = "ERROR";
				break;
			case QtFatalMsg: level = "FATAL";
				break;
			}
			appendToLogFile(QDateTime::currentDateTime().toString(timestampFormat) + " [" + level + "] " + message);
		}
		if (previousHandler != nullptr) {
			previousHandler(type, context, message);
		}
	}
}

void Logger::setLogToFile(const bool enabled) {
	logToFileEnabled = enabled;
	if (previousHandler == nullptr) {
		previousHandler = qInstallMessageHandler(messageHandler);
	}
}

bool Logger::isLogToFile() { return logToFileEnabled; }

void Logger::Info(const QString& message) {
	qInfo().noquote() << QDateTime::currentDateTime().toString(timestampFormat) << "[INFO]" << message;
}

void Logger::Warn(const QString& message) {
	qWarning().noquote() << QDateTime::currentDateTime().toString(timestampFormat) << "[WARN]" << message;
}

void Logger::Error(const QString& message) {
	// 错误级别使用 qCritical，便于按级别过滤真实错误
	qCritical().noquote() << QDateTime::currentDateTime().toString(timestampFormat) << "[ERROR]" << message;
}

void Logger::Fatal(const QString& message) {
	qCritical().noquote() << QDateTime::currentDateTime().toString(timestampFormat) << "[FATAL]" << message;
	std::exit(EXIT_FAILURE);
}

void Logger::Debug(const QString& message) {
	qDebug().noquote() << QDateTime::currentDateTime().toString(timestampFormat) << "[DEBUG]" << message;
}
