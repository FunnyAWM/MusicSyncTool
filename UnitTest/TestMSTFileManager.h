#pragma once

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "MSTFileManager.h"

/**
 * @brief MSTFileManager类的单元测试
 * 测试文件管理器的各种功能，包括格式支持检查、磁盘空间检查、文件回滚等
 */
class TestMSTFileManager : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir* tempDir;  ///< 临时测试目录

private slots:
    /**
     * @brief 测试初始化
     * 在每个测试用例执行前创建临时目录
     */
    void init();
    
    /**
     * @brief 测试清理
     * 在每个测试用例执行后清理资源
     */
    void cleanup();
    
    /**
     * @brief 测试支持的音频格式检查
     * 验证isFormatSupported函数对各种音频格式的判断
     */
    void testIsFormatSupported();
    
    /**
     * @brief 测试支持的音频格式常量
     * 验证supportedFormat常量包含预期的格式
     */
    void testSupportedFormatList();
    
    /**
     * @brief 测试文件名大小写不敏感
     * 验证格式检查对大小写不敏感
     */
    void testFormatCaseInsensitive();
    
    /**
     * @brief 测试磁盘空间检查
     * 验证isFull函数的磁盘空间检查功能
     */
    void testIsFull();
    
    /**
     * @brief 测试文件复制回滚功能
     * 验证rollBackCopy函数能正确删除文件
     */
    void testRollBackCopy();
    
    /**
     * @brief 测试构造函数和基本功能
     * 验证MSTFileManager的构造和基本存储信息获取
     */
    void testConstructorAndBasicFunctions();
    
    /**
     * @brief 测试文件复制能力检查
     * 验证copyable函数的文件复制能力检查
     */
    void testCopyable();
    
    /**
     * @brief 测试存储空间信息格式化
     * 验证getSpaceInfo函数的输出格式
     */
    void testGetSpaceInfo();
    
    /**
     * @brief 测试空文件名和无效路径
     * 验证边界条件和错误处理
     */
    void testEdgeCases();
};
