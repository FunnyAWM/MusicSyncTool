/**
 * @file TestMSTSettingsManager.cpp
 * @brief Unit tests for MSTSettingsManager (Services layer)
 * @details Tests settings persistence (create / save / load), log file I/O,
 *          log file name generation and log cleanup.
 *          Because MSTSettingsManager uses relative paths ("settings.json",
 *          "log/"), each test temporarily changes the working directory to a
 *          QTemporaryDir and restores it in cleanup().
 *          Cross-platform (Windows / Linux).
 */

#include "TestMSTSettingsManager.h"

// ======================== Fixture ========================

void TestMSTSettingsManager::init()
{
    originalWorkDir = QDir::currentPath();

    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());

    // MSTSettingsManager uses relative paths, so redirect CWD.
    QDir::setCurrent(tempDir->path());
}

void TestMSTSettingsManager::cleanup()
{
    QDir::setCurrent(originalWorkDir);
    delete tempDir;
    tempDir = nullptr;
}

// ======================== Settings I/O ========================

void TestMSTSettingsManager::testCreateDefaultSettings()
{
    MSTSettingsManager::createDefaultSettings();

    QFile file("settings.json");
    QVERIFY2(file.exists(), "settings.json should exist after createDefaultSettings");

    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QVERIFY(!doc.isNull());
    QJsonObject obj = doc.object();

    // Verify every default value written by createDefaultSettings().
    QCOMPARE(obj["ignoreLyric"].toBool(),    false);
    QCOMPARE(obj["sortBy"].toInt(),          static_cast<int>(PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE)));
    QCOMPARE(obj["orderBy"].toInt(),         static_cast<int>(PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC)));
    QCOMPARE(obj["language"].toString(),     QString(""));
    QCOMPARE(obj["favoriteTag"].toString(),  QString(""));
    QVERIFY(obj["rules"].toArray().isEmpty());
    QCOMPARE(obj["recursiveScan"].toBool(),  false);
}

void TestMSTSettingsManager::testSaveAndLoadSettings()
{
    SettingsData original;
    original.ignoreLyric   = true;
    original.sortBy        = PROPERTIES::toShort(PROPERTIES::SortByEnum::ARTIST);
    original.orderBy       = PROPERTIES::toShort(PROPERTIES::OrderByEnum::DESC);
    original.language      = "zh_CN";
    original.favoriteTag   = "FM";
    original.recursiveScan = true;
    original.rules         = {};   // no rules for this test

    QVERIFY(MSTSettingsManager::saveSettings(original));

    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));

    QCOMPARE(loaded.ignoreLyric,   original.ignoreLyric);
    QCOMPARE(loaded.sortBy,        original.sortBy);
    QCOMPARE(loaded.orderBy,       original.orderBy);
    QCOMPARE(loaded.language,      original.language);
    QCOMPARE(loaded.favoriteTag,   original.favoriteTag);
    QCOMPARE(loaded.recursiveScan, original.recursiveScan);
    QCOMPARE(loaded.rules.size(),  0);
}

void TestMSTSettingsManager::testLoadSettings_missingFile_createsDefault()
{
    // Ensure settings.json does not exist in the temp CWD.
    QVERIFY(!QFile::exists("settings.json"));

    SettingsData entity{};
    // loadSettings should fail (no file), create a default, and return false.
    bool result = MSTSettingsManager::loadSettings(entity);
    QVERIFY2(!result, "loadSettings should return false when settings.json is missing");

    // After the call, a default settings.json should have been created.
    QVERIFY2(QFile::exists("settings.json"),
             "createDefaultSettings should have been called as a fallback");
}

void TestMSTSettingsManager::testSaveSettings_withRules()
{
    SettingsData original;
    original.ignoreLyric   = false;
    original.sortBy        = PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE);
    original.orderBy       = PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC);
    original.language      = "en_US";
    original.favoriteTag   = "LOVE";
    original.recursiveScan = false;

    LyricIgnoreRule rule1(RuleType::INCLUDES, RuleField::ARTIST, "Various Artists");
    rule1.setRulesStr();

    LyricIgnoreRule rule2(RuleType::EXCLUDES, RuleField::ALBUM, "Greatest Hits");
    rule2.setRulesStr();

    original.rules = {rule1, rule2};

    QVERIFY(MSTSettingsManager::saveSettings(original));

    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));

    QCOMPARE(loaded.rules.size(), 2);

    QCOMPARE(loaded.rules[0].getRuleType(),  RuleType::INCLUDES);
    QCOMPARE(loaded.rules[0].getRuleField(), RuleField::ARTIST);
    QCOMPARE(loaded.rules[0].getRuleName(),  QString("Various Artists"));

    QCOMPARE(loaded.rules[1].getRuleType(),  RuleType::EXCLUDES);
    QCOMPARE(loaded.rules[1].getRuleField(), RuleField::ALBUM);
    QCOMPARE(loaded.rules[1].getRuleName(),  QString("Greatest Hits"));
}

void TestMSTSettingsManager::testSaveSettings_emptyFavoriteTag()
{
    SettingsData original;
    original.ignoreLyric   = false;
    original.sortBy        = PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE);
    original.orderBy       = PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC);
    original.language      = "";
    original.favoriteTag   = "";     // deliberately empty
    original.recursiveScan = false;
    original.rules         = {};

    QVERIFY(MSTSettingsManager::saveSettings(original));

    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));

    QCOMPARE(loaded.favoriteTag, QString(""));
}

// ======================== Log management ========================

void TestMSTSettingsManager::testWriteAndReadLog()
{
    const QString logPath = tempDir->path() + "/test_scan.log";
    QDateTime expected(QDate(2024, 6, 15), QTime(14, 30, 0));

    MSTSettingsManager::writeLog(logPath, expected);
    QVERIFY2(QFile::exists(logPath), "Log file should exist after writeLog");

    QDateTime actual = MSTSettingsManager::getDateFromLog(logPath);
    QCOMPARE(actual, expected);
}

void TestMSTSettingsManager::testGetDateFromLog_missingFile()
{
    // A non-existent log file should return the Unix epoch (1970-01-01).
    QDateTime result = MSTSettingsManager::getDateFromLog("/no/such/file.log");
    QDateTime epoch(QDate(1970, 1, 1), QTime(0, 0, 0));
    QCOMPARE(result, epoch);
}

void TestMSTSettingsManager::testBuildLogFileName_normalPath()
{
    QString result = MSTSettingsManager::buildLogFileName("/home/user/music");

    // The result must live under <applicationDirPath>/log/ and contain "lastScan".
    QVERIFY(result.contains("lastScan"));
    QVERIFY(result.contains("/log/"));
    // Path segments should appear in the generated name.
    QVERIFY(result.contains("home"));
    QVERIFY(result.contains("user"));
    QVERIFY(result.contains("music"));
    // Must end with .log
    QVERIFY(result.endsWith(".log"));
}

void TestMSTSettingsManager::testBuildLogFileName_pathWithColon()
{
#if defined(_WIN32) || defined(_WIN64)
    // On Windows the drive letter contains a colon (e.g. "C:").
    // The implementation strips colons from the first path segment.
    QString result = MSTSettingsManager::buildLogFileName("C:/Music/Rock");
    QVERIFY2(!result.contains("C:"),
             "Drive-letter colon should be stripped from log filename");
    QVERIFY(result.contains("lastScan"));
    QVERIFY(result.endsWith(".log"));
#else
    // On Linux colons are valid in file names but unusual; just verify format.
    QString result = MSTSettingsManager::buildLogFileName("/data/my:music");
    QVERIFY(result.contains("lastScan"));
    QVERIFY(result.endsWith(".log"));
#endif
}

void TestMSTSettingsManager::testCleanLog()
{
    // Create a "log" subdirectory in the temp CWD with some dummy files.
    QDir logDir("log");
    QVERIFY(logDir.mkpath("."));

    QFile f1("log/scan1.log");
    QVERIFY(f1.open(QIODevice::WriteOnly));
    f1.write("dummy");
    f1.close();

    QFile f2("log/scan2.log");
    QVERIFY(f2.open(QIODevice::WriteOnly));
    f2.write("dummy");
    f2.close();

    QCOMPARE(QDir("log").entryList(QDir::Files).size(), 2);

    MSTSettingsManager::cleanLog();

    // All files should be deleted; directory itself may still exist.
    QCOMPARE(QDir("log").entryList(QDir::Files).size(), 0);
}
