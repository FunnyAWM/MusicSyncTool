#include "TestIntegration.h"

void TestIntegration::init() {
    origCwd = QDir::currentPath();
    tmp = new QTemporaryDir(); QVERIFY(tmp->isValid());
    dbPath = QDir::fromNativeSeparators(tmp->path()) + "/";
    ds = new MSTDataSource(); ++conn;
    ds->setConnectionName(QString("int_%1").arg(conn));
    ds->setPath(dbPath); QVERIFY(ds->openDB()); ds->initTable();
    QDir::setCurrent(tmp->path());
}
void TestIntegration::cleanup() {
    QDir::setCurrent(origCwd);
    if (ds) { ds->closeDB(); delete ds; ds = nullptr; }
    QString cn = QString("int_%1").arg(conn);
    if (QSqlDatabase::contains(cn)) QSqlDatabase::removeDatabase(cn);
    delete tmp; tmp = nullptr;
}

void TestIntegration::scanThenQuery() {
    insert("Song1","Artist1","Album1","Rock",2024,1,"song1.mp3");
    insert("Song2","Artist2","Album2","Jazz",2024,2,"song2.mp3");
    QCOMPARE(ds->getCount(), 2);
    auto all = ds->getAll();
    QCOMPARE(all.size(), 2);
    QCOMPARE(all[0].getTitle(), QString("Song1"));
    QCOMPARE(all[1].getTitle(), QString("Song2"));
}

void TestIntegration::paginationBoundaries() {
    ds->setPageSize(5); seed(); // 12 records
    auto p1 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p1.size(), 5); QCOMPARE(p1[0].getTitle(), QString("Apple"));
    auto p2 = ds->getMusicToTable(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p2.size(), 5); QCOMPARE(p2[0].getTitle(), QString("Foxtrot"));
    auto p3 = ds->getMusicToTable(3, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p3.size(), 2); QCOMPARE(p3[0].getTitle(), QString("Kilo"));
    auto p4 = ds->getMusicToTable(4, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p4.size(), 0);
}

void TestIntegration::searchAcrossColumns() {
    seed();
    QCOMPARE(ds->searchMusic("Cherry").size(), 1);
    QCOMPARE(ds->searchMusic("ArtA").size(), 4);
    QCOMPARE(ds->searchMusic("AlbX").size(), 6);
    QCOMPARE(ds->searchMusic("ZZZZ").size(), 0);
}

void TestIntegration::sortConsistency() {
    ds->setPageSize(12); seed();
    auto asc = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    for (int i=1;i<asc.size();++i)
        QVERIFY2(asc[i].getTitle() >= asc[i-1].getTitle(), "ASC order violation");
    auto desc = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::DESC);
    for (int i=1;i<desc.size();++i)
        QVERIFY2(desc[i].getTitle() <= desc[i-1].getTitle(), "DESC order violation");
    QCOMPARE(asc.first().getTitle(), desc.last().getTitle());
    QCOMPARE(asc.last().getTitle(), desc.first().getTitle());
}

void TestIntegration::settingsRoundtrip() {
    SettingsData orig; orig.ignoreLyric = true;
    orig.sortBy = toShort(SortByEnum::ALBUM);
    orig.orderBy = toShort(OrderByEnum::DESC);
    orig.language = "en_US"; orig.favoriteTag = "STAR"; orig.recursiveScan = true;
    LyricIgnoreRule r(RuleType::INCLUDES, RuleField::TITLE, "test"); r.setRulesStr();
    orig.rules = {r};
    QVERIFY(MSTSettingsManager::saveSettings(orig));
    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));
    QCOMPARE(loaded.sortBy, orig.sortBy);
    QCOMPARE(loaded.rules.size(), 1);
    QCOMPARE(loaded.rules[0].getRuleName(), QString("test"));
}

void TestIntegration::formatValidation() {
    for (const auto& f : {"mp3","flac","wav","aac","ogg","wma","m4a","ape","aiff","opus"})
        QVERIFY(MSTFileManager::isFormatSupported(QString("x.") + f));
    QVERIFY(!MSTFileManager::isFormatSupported("x.txt"));
    QVERIFY(MSTFileManager::isFormatSupported("X.MP3"));
}

void TestIntegration::favoriteFilter() {
    ds->setPageSize(12); seed();
    auto favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 4);
    QCOMPARE(ds->getLastFavoriteCount(), 4);
    QCOMPARE(favs[0].getTitle(), QString("Apple"));
    QCOMPARE(favs[3].getTitle(), QString("Delta"));
}

void TestIntegration::ruleHitFilter() {
    ds->setPageSize(12); seed();
    auto hits = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(hits.size(), 4);
    QCOMPARE(hits[0].getTitle(), QString("Golf"));
    QCOMPARE(hits[3].getTitle(), QString("Juliet"));
}

void TestIntegration::emptyDatabase() {
    ds->setPageSize(5);
    QCOMPARE(ds->getCount(), 0);
    QCOMPARE(ds->getAll().size(), 0);
    QCOMPARE(ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC).size(), 0);
    QCOMPARE(ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC).size(), 0);
    QCOMPARE(ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC).size(), 0);
    QCOMPARE(ds->searchMusic("anything").size(), 0);
}