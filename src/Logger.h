//
// Created by funnyawm on 25-4-11.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <QCoreApplication>
#include <QString>

#include "MusicProperties.h"

/**
 * @brief 日志记录器类
 * 提供不同级别的日志记录功能（信息、调试、警告、错误、致命错误）
 */
class Logger final{
	static PROPERTIES::LogToFile logToFile;  // 日志文件输出设置
    static const QString logFileName;        // 日志文件名

public:
	/**
	 * @brief 设置日志文件输出模式
	 * @param logToFile 日志文件输出设置
	 */
	static void setLogToFile(const PROPERTIES::LogToFile& logToFile) { 
		Logger::logToFile = logToFile; 
	}
	
	/**
	 * @brief 记录信息级别日志
	 * @param message 日志消息
	 */
    static void Info(const QString& message);
    
    /**
     * @brief 记录调试级别日志
     * @param message 日志消息
     */
    static void Debug(const QString& message);
    
    /**
     * @brief 记录警告级别日志
     * @param message 日志消息
     */
    static void Warn(const QString& message);
    
    /**
     * @brief 记录错误级别日志
     * @param message 日志消息
     */
    static void Error(const QString& message);
    
    /**
     * @brief 记录致命错误级别日志
     * @param message 日志消息
     */
    static void Fatal(const QString& message);
};

#endif //LOGGER_H
