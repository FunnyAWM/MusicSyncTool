#pragma once

#include <QTest>
#include <QObject>
#include "LyricIgnoreRule.h"
#include "MusicProperties.h"

using namespace PROPERTIES;

/**
 * @brief LyricIgnoreRule类的单元测试
 * 测试歌词忽略规则的各种功能，包括构造、比较、字符串转换等
 */
class TestLyricIgnoreRule : public QObject
{
    Q_OBJECT

private slots:
    /**
     * @brief 测试构造函数
     * 验证LyricIgnoreRule的正确构造
     */
    void testConstructor();
    
    /**
     * @brief 测试拷贝构造函数
     * 验证拷贝构造的正确性
     */
    void testCopyConstructor();
    
    /**
     * @brief 测试赋值运算符
     * 验证赋值操作的正确性
     */
    void testAssignmentOperator();
    
    /**
     * @brief 测试相等运算符
     * 验证对象相等性比较
     */
    void testEqualityOperator();
    
    /**
     * @brief 测试不等运算符
     * 验证对象不等性比较
     */
    void testInequalityOperator();
    
    /**
     * @brief 测试Getter方法
     * 验证所有属性的获取方法
     */
    void testGetters();
    
    /**
     * @brief 测试字符串转换方法
     * 验证枚举到字符串的转换功能
     */
    void testStringConversion();
    
    /**
     * @brief 测试字符串到枚举的转换
     * 验证字符串到枚举的转换功能
     */
    void testStringToEnum();
    
    /**
     * @brief 测试规则字符串表示的设置
     * 验证setRulesStr方法的功能
     */
    void testSetRulesStr();
    
    /**
     * @brief 测试边界条件和无效输入
     * 验证错误处理和边界情况
     */
    void testEdgeCases();
    
    /**
     * @brief 测试所有规则类型组合
     * 验证不同规则类型和字段的组合
     */
    void testAllCombinations();
    
    /**
     * @brief 测试中文字符串处理
     * 验证中文规则名称的处理
     */
    void testChineseRuleNames();
};
