#pragma once

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include "QueryItem.h"

/**
 * @brief QueryItem类的单元测试
 * 测试音乐元数据项的各种功能，包括构造、getter/setter、相似度比较等
 */
class TestQueryItem : public QObject
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
     * @brief 测试默认构造函数
     * 验证默认构造的QueryItem对象状态
     */
    void testDefaultConstructor();
    
    /**
     * @brief 测试完整参数构造函数
     * 验证使用所有参数构造QueryItem对象
     */
    void testParameterConstructor();
    
    /**
     * @brief 测试Getter方法
     * 验证所有属性的获取方法
     */
    void testGetters();
    
    /**
     * @brief 测试Setter方法
     * 验证所有属性的设置方法
     */
    void testSetters();
    
    /**
     * @brief 测试相似度阈值功能
     * 验证静态相似度阈值的设置和获取
     */
    void testSensitivity();
    
    /**
     * @brief 测试相等比较运算符
     * 验证QueryItem对象的相等性比较
     */
    void testEqualityOperator();
    
    /**
     * @brief 测试相似度比较功能
     * 验证两个QueryItem对象的相似度计算
     */
    void testSimilarityComparison();
    
    /**
     * @brief 测试空值和特殊字符处理
     * 验证边界条件下的行为
     */
    void testEdgeCases();
    
    /**
     * @brief 测试中文字符处理
     * 验证对中文音乐信息的处理
     */
    void testChineseCharacters();
    
    /**
     * @brief 测试年份和音轨编号边界值
     * 验证数值类型属性的边界值处理
     */
    void testNumericBoundaries();
};
