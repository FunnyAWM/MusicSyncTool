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

    for (const auto& tc : tests) {
        QObject* t = tc.create();
        int r = QTest::qExec(t, argc, argv);
        if (r != 0) {
            qWarning() << "FAILED:" << tc.name << "(exit code" << r << ")";
            status |= r;
        }
        delete t;
    }

    return status;
}