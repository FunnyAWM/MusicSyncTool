#pragma once

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "Logger.h"

/**
 * @brief Logger类的单元测试
 * 测试日志记录器的各种功能，包括不同级别的日志记录、文件输出等
 */
class TestLogger : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir* tempDir;  ///< 临时测试目录

private slots:
    /**
     * @brief 测试初始化
     */
    void init();
    
    /**
     * @brief 测试清理
     */
    void cleanup();
    
    /**
     * @brief 测试Info级别日志
     * 验证信息级别日志的记录功能
     */
    void testInfoLogging();
    
    /**
     * @brief 测试Warning级别日志
     * 验证警告级别日志的记录功能
     */
    void testWarningLogging();
    
    /**
     * @brief 测试Error级别日志
     * 验证错误级别日志的记录功能
     */
    void testErrorLogging();
    
    /**
     * @brief 测试Fatal级别日志
     * 验证致命错误级别日志的记录功能
     */
    void testFatalLogging();
    
    /**
     * @brief 测试Debug级别日志
     * 验证调试级别日志的记录功能
     */
    void testDebugLogging();
    
    /**
     * @brief 测试日志消息格式
     * 验证日志消息的格式和内容
     */
    void testLogMessageFormat();
    
    /**
     * @brief 测试特殊字符和长消息
     * 验证特殊字符和超长消息的处理
     */
    void testSpecialCharactersAndLongMessages();
    
    /**
     * @brief 测试中文日志消息
     * 验证中文字符的日志记录
     */
    void testChineseLogMessages();
    
    /**
     * @brief 测试空消息和边界条件
     * 验证边界情况的处理
     */
    void testEmptyMessagesAndEdgeCases();
    
    /**
     * @brief 测试并发日志记录
     * 验证多线程环境下的日志记录安全性
     */
    void testConcurrentLogging();
    
    /**
     * @brief 测试日志级别区分
     * 验证不同日志级别的区分和处理
     */
    void testLogLevelDifferentiation();

private:
    /**
     * @brief 辅助方法：捕获控制台输出
     * @param logFunction 要测试的日志函数
     * @param message 日志消息
     * @return 捕获到的输出内容
     */
    QString captureConsoleOutput(std::function<void(const QString&)> logFunction, const QString& message);
};
