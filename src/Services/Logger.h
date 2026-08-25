#ifndef LOGGER_H
#define LOGGER_H
#include <QCoreApplication>
#include <QString>

#include "../Core/MusicProperties.h"

class Logger final{
	static PROPERTIES::LogToFile logToFile;
    std::optional<QString> logFileName;

public:
    explicit Logger() {
	    try {
            logFileName = QCoreApplication::applicationDirPath() + "/log/lastRun.log";
		}
		catch (const std::exception& e) {
			// 如果获取应用程序目录失败，禁用日志文件输出
			logToFile = PROPERTIES::LogToFile::DISABLE;
            Warn("无法获取应用程序目录，日志文件输出已禁用:" + QString::fromStdString(e.what()));
		}
    }
	static void setLogToFile(const PROPERTIES::LogToFile& logToFile_) { 
		logToFile = logToFile_; 
	}
	
    static void Info(const QString& message);
    
    static void Debug(const QString& message);
    
    static void Warn(const QString& message);
    
    static void Error(const QString& message);
    
    static void Fatal(const QString& message);
};

#endif //LOGGER_H
