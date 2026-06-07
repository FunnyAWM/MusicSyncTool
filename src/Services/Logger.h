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
    static const QString logFileName;        ///< 日志文件名

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
