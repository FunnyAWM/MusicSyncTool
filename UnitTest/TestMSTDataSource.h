#pragma once

#include <QTest>
#include <QObject>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSignalSpy>
#include "MSTDataSource.h"
#include "QueryItem.h"
#include "LyricIgnoreRule.h"

/**
 * @brief MSTDataSource类的单元测试
 * 测试数据源管理器的各种功能，包括数据库操作、查询、添加、删除等
 */
class TestMSTDataSource : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir* tempDir;  ///< 临时测试目录
    QString testDbPath;      ///< 测试数据库路径

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
     * @brief 测试构造函数和基本属性
     * 验证MSTDataSource的构造和基本属性设置
     */
    void testConstructorAndBasicProperties();
    
    /**
     * @brief 测试数据库连接
     * 验证数据库连接的建立和管理
     */
    void testDatabaseConnection();
    
    /**
     * @brief 测试表初始化
     * 验证数据库表的创建和初始化
     */
    void testTableInitialization();
    
    /**
     * @brief 测试添加单个音乐文件
     * 验证单个音乐文件的添加功能
     */
    void testAddSingleMusic();
    
    /**
     * @brief 测试批量添加音乐文件
     * 验证批量音乐文件的添加功能
     */
    void testAddMultipleMusic();
    
    /**
     * @brief 测试查询所有音乐
     * 验证获取所有音乐数据的功能
     */
    void testGetAllMusic();
    
    /**
     * @brief 测试分页查询
     * 验证分页查询功能和页面大小设置
     */
    void testPaginatedQuery();
    
    /**
     * @brief 测试音乐搜索功能
     * 验证关键词搜索音乐的功能
     */
    void testSearchMusic();
    
    /**
     * @brief 测试删除音乐功能
     * 验证删除音乐记录的功能
     */
    void testDeleteMusic();
    
    /**
     * @brief 测试获取音乐总数
     * 验证统计音乐总数的功能
     */
    void testGetCount();
    
    /**
     * @brief 测试收藏功能
     * 验证音乐收藏状态的设置和查询
     */
    void testFavoriteFeature();
    
    /**
     * @brief 测试规则命中功能
     * 验证歌词忽略规则的命中检测
     */
    void testRuleHitFeature();
    
    /**
     * @brief 测试排序功能
     * 验证不同字段和顺序的排序功能
     */
    void testSortingFeature();
    
    /**
     * @brief 测试信号发射
     * 验证数据源操作过程中的信号发射
     */
    void testSignalEmission();
    
    /**
     * @brief 测试错误处理
     * 验证异常情况和错误的处理
     */
    void testErrorHandling();
    
    /**
     * @brief 测试边界条件
     * 验证边界值和特殊情况的处理
     */
    void testEdgeCases();
    
    /**
     * @brief 测试中文音乐信息
     * 验证中文字符的音乐信息处理
     */
    void testChineseMusicInfo();

private:
    /**
     * @brief 创建测试用的QueryItem
     * @param index 索引，用于生成不同的测试数据
     * @return 测试QueryItem对象
     */
    QueryItem createTestQueryItem(int index);
    
    /**
     * @brief 验证数据库中的音乐数量
     * @param dataSource 数据源对象
     * @param expectedCount 期望的数量
     */
    void verifyMusicCount(MSTDataSource& dataSource, int expectedCount);
};
