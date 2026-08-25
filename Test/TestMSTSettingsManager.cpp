#include "TestMSTSettingsManager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TestMSTSettingsManager::init() {
    origCwd = QDir::currentPath();
    tmp = new QTemporaryDir(); QVERIFY(tmp->isValid());
    QDir::setCurrent(tmp->path());
}
void TestMSTSettingsManager::cleanup() {
    QDir::setCurrent(origCwd);
    delete tmp; tmp = nullptr;
}

void TestMSTSettingsManager::createDefaultSettings() {
    MSTSettingsManager::createDefaultSettings();
    QFile f("settings.json"); QVERIFY(f.exists());
    QVERIFY(f.open(QIODevice::ReadOnly));
    auto doc = QJsonDocument::fromJson(f.readAll()); f.close();
    QVERIFY(!doc.isNull());
    auto o = doc.object();
    QCOMPARE(o["ignoreLyric"].toBool(), false);
    QCOMPARE(o["favoriteTag"].toString(), QString(""));
    QVERIFY(o["rules"].toArray().isEmpty());
    QCOMPARE(o["recursiveScan"].toBool(), false);
}

void TestMSTSettingsManager::saveAndLoad() {
    SettingsData orig; orig.ignoreLyric = true;
    orig.sortBy = PROPERTIES::toShort(PROPERTIES::SortByEnum::ARTIST);
    orig.orderBy = PROPERTIES::toShort(PROPERTIES::OrderByEnum::DESC);
    orig.language = "zh_CN"; orig.favoriteTag = "FM"; orig.recursiveScan = true;
    QVERIFY(MSTSettingsManager::saveSettings(orig));
    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));
    QCOMPARE(loaded.ignoreLyric, true);
    QCOMPARE(loaded.sortBy, orig.sortBy);
    QCOMPARE(loaded.language, QString("zh_CN"));
    QCOMPARE(loaded.favoriteTag, QString("FM"));
}

void TestMSTSettingsManager::loadMissingCreatesDefault() {
    QVERIFY(!QFile::exists("settings.json"));
    SettingsData e{};
    QVERIFY(!MSTSettingsManager::loadSettings(e));
    QVERIFY(QFile::exists("settings.json"));
}

void TestMSTSettingsManager::saveWithRules() {
    SettingsData orig; orig.ignoreLyric = false;
    orig.sortBy = PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE);
    orig.orderBy = PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC);
    orig.language = "en_US"; orig.favoriteTag = "LOVE"; orig.recursiveScan = false;
    LyricIgnoreRule r1(RuleType::INCLUDES, RuleField::ARTIST, "Various Artists"); r1.setRulesStr();
    LyricIgnoreRule r2(RuleType::EXCLUDES, RuleField::ALBUM, "Greatest Hits"); r2.setRulesStr();
    orig.rules = {r1, r2};
    QVERIFY(MSTSettingsManager::saveSettings(orig));
    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));
    QCOMPARE(loaded.rules.size(), 2);
    QCOMPARE(loaded.rules[0].getRuleName(), QString("Various Artists"));
    QCOMPARE(loaded.rules[1].getRuleName(), QString("Greatest Hits"));
}

void TestMSTSettingsManager::writeAndReadLog() {
    const QString lp = tmp->path() + "/scan.log";
    QDateTime exp(QDate(2024,6,15), QTime(14,30,0));
    MSTSettingsManager::writeLog(lp, exp);
    QVERIFY(QFile::exists(lp));
    QCOMPARE(MSTSettingsManager::getDateFromLog(lp), exp);
}

void TestMSTSettingsManager::getDateFromLog_missing() {
    QCOMPARE(MSTSettingsManager::getDateFromLog("/no/such.log"),
             QDateTime(QDate(1970,1,1), QTime(0,0,0)));
}

void TestMSTSettingsManager::buildLogFileName() {
    QString r = MSTSettingsManager::buildLogFileName("/home/user/music");
    QVERIFY(r.contains("lastScan"));
    QVERIFY(r.contains("/log/"));
    QVERIFY(r.endsWith(".log"));
}

void TestMSTSettingsManager::cleanLog() {
    QDir("log").mkpath(".");
    { QFile f("log/a.log"); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close(); }
    { QFile f("log/b.log"); QVERIFY(f.open(QIODevice::WriteOnly)); f.write("x"); f.close(); }
    QCOMPARE(QDir("log").entryList(QDir::Files).size(), 2);
    MSTSettingsManager::cleanLog();
    QCOMPARE(QDir("log").entryList(QDir::Files).size(), 0);
}