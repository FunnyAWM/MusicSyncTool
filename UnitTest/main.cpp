#include <QTest>
#include <QCoreApplication>

#include "TestMSTFileManager.h"
#include "TestMSTDataSource.h"
#include "TestQueryItem.h"
#include "TestLyricIgnoreRule.h"
#include "TestLogger.h"

/**
 * @brief 单元测试主函数
 * 运行所有测试类的测试用例
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    int status = 0;
    
    // 运行MSTFileManager测试
    {
        TestMSTFileManager testFileManager;
        status |= QTest::qExec(&testFileManager, argc, argv);
    }
    
    // 运行MSTDataSource测试
    {
        TestMSTDataSource testDataSource;
        status |= QTest::qExec(&testDataSource, argc, argv);
    }
    
    // 运行QueryItem测试
    {
        TestQueryItem testQueryItem;
        status |= QTest::qExec(&testQueryItem, argc, argv);
    }
    
    // 运行LyricIgnoreRule测试
    {
        TestLyricIgnoreRule testLyricIgnoreRule;
        status |= QTest::qExec(&testLyricIgnoreRule, argc, argv);
    }
    
    // 运行Logger测试
    {
        TestLogger testLogger;
        status |= QTest::qExec(&testLogger, argc, argv);
    }
    
    return status;
}
