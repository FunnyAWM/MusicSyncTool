/**
 * @file Logger.h
 * @brief 日志记录器类定义
 * @details 提供全局静态日志记录功能，支持信息、调试、警告、错误和致命错误
 *          五个级别的日志输出，所有日志带时间戳前缀
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef LOGGER_H
#define LOGGER_H
#include <QCoreApplication>
#include <QString>

#include "../Core/MusicProperties.h"

/**
 * @brief 日志记录器类
 * 提供不同级别的日志记录功能（信息、调试、警告、错误、致命错误）。
 * 所有方法均为静态方法，支持控制台输出和可选的文件输出
 */
class Logger final{
	static PROPERTIES::LogToFile logToFile;  ///< 日志文件输出设置
    std::optional<QString> logFileName;        ///< 日志文件名

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
	/**
	 * @brief 设置日志文件输出模式
	 * @param logToFile_ 日志文件输出设置
	 */
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
