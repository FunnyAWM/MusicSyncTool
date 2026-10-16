#pragma warning(disable: 4996)
#include <functional>
#include <QApplication>
#include <QTest>

// Test headers
#include "TestBlackBox.h"
#include "TestIntegration.h"
#include "TestLyricIgnoreRule.h"
#include "TestMSTDataSource.h"
#include "TestMSTFileManager.h"
#include "TestMSTSettingsManager.h"
#include "TestMusicProperties.h"
#include "TestQueryItem.h"
#include "TestStringSimilarity.h"

// Test implementations (included directly to avoid MOC complications)
#include "TestBlackBox.cpp"
#include "TestIntegration.cpp"
#include "TestLyricIgnoreRule.cpp"
#include "TestMSTDataSource.cpp"
#include "TestMSTFileManager.cpp"
#include "TestMSTSettingsManager.cpp"
#include "TestMusicProperties.cpp"
#include "TestQueryItem.cpp"
#include "TestStringSimilarity.cpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    int status = 0;

    // 可选：以类名作为第一个参数只运行该类（mst_tests.exe TestMSTDataSource），便于定位失败与 CI 分片
    QString onlyClass = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString();

    struct TestCase { const char* name; std::function<QObject*()> create; };
    const TestCase tests[] = {
        {"TestMusicProperties",    []{ return new TestMusicProperties(); }},
        {"TestStringSimilarity",   []{ return new TestStringSimilarity(); }},
        {"TestQueryItem",          []{ return new TestQueryItem(); }},
        {"TestLyricIgnoreRule",    []{ return new TestLyricIgnoreRule(); }},
        {"TestMSTFileManager",     []{ return new TestMSTFileManager(); }},
        {"TestMSTDataSource",      []{ return new TestMSTDataSource(); }},
        {"TestMSTSettingsManager", []{ return new TestMSTSettingsManager(); }},
        {"TestIntegration",        []{ return new TestIntegration(); }},
        {"TestBlackBox",           []{ return new TestBlackBox(); }},
    };

    // 类名被识别时从 QTest 的参数里剔除（否则会被当成测试函数过滤器），其余选项照常透传
    int firstArg = 1;
    if (!onlyClass.isEmpty()) {
        for (const auto& tc : tests) {
            if (onlyClass == QLatin1String(tc.name)) {
                firstArg = 2;
                break;
            }
        }
        if (firstArg == 1) {
            onlyClass.clear();
        }
    }

    for (const auto& tc : tests) {
        if (!onlyClass.isEmpty() && onlyClass != QLatin1String(tc.name)) {
            continue;
        }
        QObject* t = tc.create();
        const int r = QTest::qExec(t, argc - firstArg + 1, argv + firstArg - 1);
        if (r != 0) {
            qWarning() << "FAILED:" << tc.name << "(exit code" << r << ")";
            status |= r;
        }
        delete t;
    }

    return status;
}