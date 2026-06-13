/**
 * @file main.cpp
 * @brief Unit test runner for MusicSyncTool
 * @details Executes all test classes using QTest framework.
 *          Test source files are included directly to avoid MOC complications
 *          with separate compilation units. AUTOMOC generates main.moc which
 *          provides Q_OBJECT implementations for all test classes.
 *
 * Design note: Each Test*.cpp file is included as a source (not compiled
 * separately). The Q_OBJECT macro implementations are provided by the
 * main.moc include at the bottom of this file.
 */

#include <functional>
#include <QApplication>
#include <QTest>

// Test headers (included first for AUTOMOC Q_OBJECT detection)
#include "TestBlackBox.h"
#include "TestLyricIgnoreRule.h"
#include "TestMSTDataSource.h"
#include "TestMSTFileManager.h"
#include "TestMSTSettingsManager.h"
#include "TestMusicProperties.h"
#include "TestQueryItem.h"
#include "TestStringSimilarity.h"

// Test implementations (included directly, not compiled as separate units)
#include "TestBlackBox.cpp"
#include "TestLyricIgnoreRule.cpp"
#include "TestMSTDataSource.cpp"
#include "TestMSTFileManager.cpp"
#include "TestMSTSettingsManager.cpp"
#include "TestMusicProperties.cpp"
#include "TestQueryItem.cpp"
#include "TestStringSimilarity.cpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    // Optional: pass a test class index (1-8) as first arg to run just one class
    int runIndex = 0; // 0 = run all
    if (argc > 1) {
        bool ok = false;
        int idx = QString(argv[1]).toInt(&ok);
        if (ok && idx >= 1 && idx <= 8)
            runIndex = idx;
    }

    // Build args without the index parameter for QTest
    QStringList qtestArgs;
    for (int i = 1; i < argc; ++i) {
        bool isIdx = false;
        QString(argv[i]).toInt(&isIdx);
        if (!isIdx)
            qtestArgs << QString(argv[i]);
    }

    auto runTest = [&](QObject* test, const QStringList& args) -> int {
        QVector<QByteArray> ba;
        ba.append(QByteArray("test")); // argv[0] = program name
        QVector<char*> cargs;
        cargs.append(ba.last().data());
        for (const auto& a : args) {
            ba.append(a.toLocal8Bit());
            cargs.append(ba.last().data());
        }
        cargs.append(nullptr);
        return QTest::qExec(test, static_cast<int>(cargs.size() - 1), cargs.data());
    };

    struct TestCase { const char* name; std::function<QObject*()> create; };
    TestCase tests[] = {
        {"TestMusicProperties",    []() -> QObject* { return new TestMusicProperties(); }},
        {"TestStringSimilarity",   []() -> QObject* { return new TestStringSimilarity(); }},
        {"TestQueryItem",          []() -> QObject* { return new TestQueryItem(); }},
        {"TestLyricIgnoreRule",    []() -> QObject* { return new TestLyricIgnoreRule(); }},
        {"TestMSTFileManager",     []() -> QObject* { return new TestMSTFileManager(); }},
        {"TestMSTDataSource",      []() -> QObject* { return new TestMSTDataSource(); }},
        {"TestMSTSettingsManager", []() -> QObject* { return new TestMSTSettingsManager(); }},
        {"TestBlackBox",           []() -> QObject* { return new TestBlackBox(); }},
    };

    for (int i = 0; i < 8; ++i) {
        if (runIndex != 0 && runIndex != i + 1)
            continue;
        QObject* t = tests[i].create();
        int r = runTest(t, qtestArgs);
        if (r != 0) {
            qWarning() << "FAILED:" << tests[i].name << "(exit code" << r << ")";
            status |= r;
        }
        delete t;
    }

    return status;
}

