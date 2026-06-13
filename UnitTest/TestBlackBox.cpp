/**
 * @file TestBlackBox.cpp
 * @brief Black-box integration tests for the MusicSyncTool Services layer.
 * @details Uses equivalence-class partitioning to test external behaviour
 *          without relying on internal implementation details.  Covers:
 *            - Pagination (boundary values)
 *            - Search (input partitioning)
 *            - Sort order
 *            - Settings persistence
 *            - File format validation
 *            - Favorite filter
 *            - Rule-hit filter
 *
 *          Cross-platform (Windows / Linux).  All file I/O uses
 *          QTemporaryDir; database records are inserted via SQL to avoid
 *          a TagLib dependency.
 */

#include "TestBlackBox.h"

// ======================== Fixture ========================

void TestBlackBox::init()
{
    originalWorkDir = QDir::currentPath();
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());

    dbDirPath = QDir::fromNativeSeparators(tempDir->path()) + QStringLiteral("/");

    ds = new MSTDataSource();
    ++connectionCounter;
    ds->setConnectionName(QStringLiteral("bb_ds_%1").arg(connectionCounter));
    ds->setPath(dbDirPath);
    QVERIFY(ds->openDB());
    ds->initTable();

    QDir::setCurrent(tempDir->path());
}

void TestBlackBox::cleanup()
{
    QDir::setCurrent(originalWorkDir);
    if (ds) { ds->closeDB(); delete ds; ds = nullptr; }
    QString cn = QStringLiteral("bb_ds_%1").arg(connectionCounter);
    if (QSqlDatabase::contains(cn)) QSqlDatabase::removeDatabase(cn);
    delete tempDir;
    tempDir = nullptr;
}

// ======================== EC: Pagination ========================

void TestBlackBox::testPagination_firstPage()
{
    ds->setPageSize(5);
    seedRecords();
    QList<QueryItem> p = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p.size(), 5);
    QCOMPARE(p[0].getTitle(), QString("Apple"));
    QCOMPARE(p[4].getTitle(), QString("Echo"));
}

void TestBlackBox::testPagination_middlePage()
{
    ds->setPageSize(5);
    seedRecords();
    QList<QueryItem> p = ds->getMusicToTable(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p.size(), 5);
    QCOMPARE(p[0].getTitle(), QString("Foxtrot"));
    QCOMPARE(p[4].getTitle(), QString("Juliet"));
}

void TestBlackBox::testPagination_lastPage()
{
    ds->setPageSize(5);
    seedRecords(); // 12 records, page 3 has 2
    QList<QueryItem> p = ds->getMusicToTable(3, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p.size(), 2);
    QCOMPARE(p[0].getTitle(), QString("Kilo"));
    QCOMPARE(p[1].getTitle(), QString("Lima"));
}

void TestBlackBox::testPagination_emptyDatabase()
{
    ds->setPageSize(5);
    QList<QueryItem> p = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p.size(), 0);
}

void TestBlackBox::testPagination_singleRecord()
{
    ds->setPageSize(5);
    insertRecord("Only", "Art", "Alb", "Pop", 2024, 1, "only.mp3");

    QList<QueryItem> p1 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p1.size(), 1);
    QCOMPARE(p1[0].getTitle(), QString("Only"));

    QList<QueryItem> p2 = ds->getMusicToTable(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p2.size(), 0);
}

void TestBlackBox::testPagination_exactlyOnePageSize()
{
    ds->setPageSize(5);
    for (int i = 1; i <= 5; ++i)
        insertRecord(QString("Song%1").arg(i, 2, 10, QChar('0')),
                     "Art", "Alb", "Pop", 2024, i,
                     QString("s%1.mp3").arg(i));

    QList<QueryItem> p1 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p1.size(), 5);

    QList<QueryItem> p2 = ds->getMusicToTable(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p2.size(), 0);
}

// ======================== EC: Search ========================

void TestBlackBox::testSearch_byExactTitle()
{
    seedRecords();
    QList<QueryItem> r = ds->searchMusic("Cherry");
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].getTitle(), QString("Cherry"));
    QCOMPARE(r[0].getArtist(), QString("ArtC"));
}

void TestBlackBox::testSearch_byExactArtist()
{
    seedRecords();
    // ArtA appears in tracks 1,4,7,10 = 4 records
    QList<QueryItem> r = ds->searchMusic("ArtA");
    QCOMPARE(r.size(), 4);
    for (const auto &item : r)
        QCOMPARE(item.getArtist(), QString("ArtA"));
}

void TestBlackBox::testSearch_byExactAlbum()
{
    seedRecords();
    // AlbX: tracks 1,3,5,7,9,11 = 6 records
    QList<QueryItem> r = ds->searchMusic("AlbX");
    QCOMPARE(r.size(), 6);
    for (const auto &item : r)
        QCOMPARE(item.getAlbum(), QString("AlbX"));
}

void TestBlackBox::testSearch_noMatch()
{
    seedRecords();
    QList<QueryItem> r = ds->searchMusic("ZZZZZ_NoMatch");
    QCOMPARE(r.size(), 0);
}

void TestBlackBox::testSearch_emptyString()
{
    seedRecords();
    QList<QueryItem> r = ds->searchMusic("");
    QCOMPARE(r.size(), 0);
}

// ======================== EC: Sort order ========================

void TestBlackBox::testSort_titleAscending()
{
    ds->setPageSize(12);
    seedRecords();
    QList<QueryItem> all = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(all.size(), 12);
    for (int i = 1; i < all.size(); ++i)
        QVERIFY2(all[i].getTitle() >= all[i-1].getTitle(),
                 qPrintable(QString("'%1' should be >= '%2'")
                     .arg(all[i].getTitle(), all[i-1].getTitle())));
    QCOMPARE(all.first().getTitle(), QString("Apple"));
    QCOMPARE(all.last().getTitle(),  QString("Lima"));
}

void TestBlackBox::testSort_titleDescending()
{
    ds->setPageSize(12);
    seedRecords();
    QList<QueryItem> all = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::DESC);
    QCOMPARE(all.size(), 12);
    for (int i = 1; i < all.size(); ++i)
        QVERIFY2(all[i].getTitle() <= all[i-1].getTitle(),
                 qPrintable(QString("'%1' should be <= '%2'")
                     .arg(all[i].getTitle(), all[i-1].getTitle())));
    QCOMPARE(all.first().getTitle(), QString("Lima"));
    QCOMPARE(all.last().getTitle(),  QString("Apple"));
}

void TestBlackBox::testSort_artistAscending()
{
    ds->setPageSize(12);
    seedRecords();
    QList<QueryItem> all = ds->getMusicToTable(1, SortByEnum::ARTIST, OrderByEnum::ASC);
    QCOMPARE(all.size(), 12);
    for (int i = 1; i < all.size(); ++i)
        QVERIFY2(all[i].getArtist() >= all[i-1].getArtist(),
                 qPrintable(QString("Artist '%1' should be >= '%2'")
                     .arg(all[i].getArtist(), all[i-1].getArtist())));
    QCOMPARE(all.first().getArtist(), QString("ArtA"));
}

void TestBlackBox::testSort_artistDescending()
{
    ds->setPageSize(12);
    seedRecords();
    QList<QueryItem> all = ds->getMusicToTable(1, SortByEnum::ARTIST, OrderByEnum::DESC);
    QCOMPARE(all.size(), 12);
    for (int i = 1; i < all.size(); ++i)
        QVERIFY2(all[i].getArtist() <= all[i-1].getArtist(),
                 qPrintable(QString("Artist '%1' should be <= '%2'")
                     .arg(all[i].getArtist(), all[i-1].getArtist())));
    QCOMPARE(all.first().getArtist(), QString("ArtC"));
}

// ======================== EC: Settings persistence ========================

void TestBlackBox::testSettings_roundtrip()
{
    SettingsData original;
    original.ignoreLyric   = true;
    original.sortBy        = PROPERTIES::toShort(SortByEnum::ALBUM);
    original.orderBy       = PROPERTIES::toShort(OrderByEnum::DESC);
    original.language      = "en_US";
    original.favoriteTag   = "STAR";
    original.recursiveScan = true;
    original.rules         = {};

    QVERIFY(MSTSettingsManager::saveSettings(original));

    SettingsData loaded{};
    QVERIFY(MSTSettingsManager::loadSettings(loaded));
    QCOMPARE(loaded.ignoreLyric,   true);
    QCOMPARE(loaded.sortBy,        original.sortBy);
    QCOMPARE(loaded.orderBy,       original.orderBy);
    QCOMPARE(loaded.language,      QString("en_US"));
    QCOMPARE(loaded.favoriteTag,   QString("STAR"));
    QCOMPARE(loaded.recursiveScan, true);
}

void TestBlackBox::testSettings_missingFile()
{
    QVERIFY(!QFile::exists("settings.json"));

    SettingsData entity{};
    bool ok = MSTSettingsManager::loadSettings(entity);
    QVERIFY2(!ok, "loadSettings should return false for missing file");
    QVERIFY2(QFile::exists("settings.json"),
             "A default settings.json should be created as fallback");
}

// ======================== EC: File format validation ========================

void TestBlackBox::testFormatValidation_supported()
{
    // Verify every supported format returns true.
    const QStringList formats = {"mp3","flac","wav","aac","ogg",
                                 "wma","m4a","ape","aiff","opus"};
    for (const QString &fmt : formats) {
        QVERIFY2(MSTFileManager::isFormatSupported("test." + fmt),
                 qPrintable(QString("Expected '%1' to be supported").arg(fmt)));
    }
}

void TestBlackBox::testFormatValidation_unsupported()
{
    QVERIFY(!MSTFileManager::isFormatSupported("file.txt"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.exe"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.pdf"));
    QVERIFY(!MSTFileManager::isFormatSupported("file.mp4"));
}

void TestBlackBox::testFormatValidation_noExtension()
{
    QVERIFY(!MSTFileManager::isFormatSupported("noextension"));
    QVERIFY(!MSTFileManager::isFormatSupported(""));
}

void TestBlackBox::testFormatValidation_uppercase()
{
    QVERIFY(MSTFileManager::isFormatSupported("song.MP3"));
    QVERIFY(MSTFileManager::isFormatSupported("song.FLAC"));
    QVERIFY(MSTFileManager::isFormatSupported("song.Wav"));
}

// ======================== EC: Favorite filter ========================

void TestBlackBox::testFavorite_allFavorites()
{
    ds->setPageSize(10);
    insertRecord("F1", "Art", "Alb", "Pop", 2024, 1, "f1.mp3", true);
    insertRecord("F2", "Art", "Alb", "Pop", 2024, 2, "f2.mp3", true);
    insertRecord("F3", "Art", "Alb", "Pop", 2024, 3, "f3.mp3", true);

    QList<QueryItem> favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 3);
    QCOMPARE(ds->getLastFavoriteCount(), 3);
}

void TestBlackBox::testFavorite_noFavorites()
{
    ds->setPageSize(10);
    seedRecords(); // has some favorites, so create fresh:

    // Delete all and insert records with no favorites.
    ds->prepareStatement("DELETE FROM musicInfo");
    ds->execQuery();

    insertRecord("X1", "Art", "Alb", "Pop", 2024, 1, "x1.mp3", false);
    insertRecord("X2", "Art", "Alb", "Pop", 2024, 2, "x2.mp3", false);

    QList<QueryItem> favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 0);
    QCOMPARE(ds->getLastFavoriteCount(), 0);
}

void TestBlackBox::testFavorite_someFavorites()
{
    ds->setPageSize(12);
    seedRecords(); // 4 out of 12 are favorites

    QList<QueryItem> favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 4);
    // Favorites are Apple, Banana, Cherry, Delta (already in alpha order).
    QCOMPARE(favs[0].getTitle(), QString("Apple"));
    QCOMPARE(favs[1].getTitle(), QString("Banana"));
    QCOMPARE(favs[2].getTitle(), QString("Cherry"));
    QCOMPARE(favs[3].getTitle(), QString("Delta"));
}

void TestBlackBox::testFavorite_lastFavoriteCount()
{
    ds->setPageSize(2);
    seedRecords(); // 4 favorites total

    // Request page 1; lastFavoriteCount should reflect the real total (4),
    // not just the page size.
    ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(ds->getLastFavoriteCount(), 4);
}

// ======================== EC: Rule hit ========================

void TestBlackBox::testRuleHit_includesRule()
{
    ds->setPageSize(12);
    seedRecords(); // tracks 7-10 are rule hits (Golf..Juliet)

    QList<QueryItem> hits = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(hits.size(), 4);
    QCOMPARE(hits[0].getTitle(), QString("Golf"));
    QCOMPARE(hits[1].getTitle(), QString("Hotel"));
    QCOMPARE(hits[2].getTitle(), QString("India"));
    QCOMPARE(hits[3].getTitle(), QString("Juliet"));
}

void TestBlackBox::testRuleHit_excludesRule()
{
    ds->setPageSize(12);
    // Insert records where ruleHit = false (i.e., the rule excludes them).
    insertRecord("ExA", "Art", "Alb", "Pop", 2024, 1, "exa.mp3", false, false);
    insertRecord("ExB", "Art", "Alb", "Pop", 2024, 2, "exb.mp3", false, false);

    QList<QueryItem> hits = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(hits.size(), 0);

    // Verify via the fileName overload that both return false.
    QVERIFY(!ds->getRuleHit("exa.mp3"));
    QVERIFY(!ds->getRuleHit("exb.mp3"));
}

void TestBlackBox::testRuleHit_noMatch()
{
    ds->setPageSize(12);
    seedRecords(); // has 4 rule hits

    // Now clear all and insert zero rule-hit records.
    ds->prepareStatement("DELETE FROM musicInfo");
    ds->execQuery();

    insertRecord("NR1", "Art", "Alb", "Pop", 2024, 1, "nr1.mp3", false, false);
    insertRecord("NR2", "Art", "Alb", "Pop", 2024, 2, "nr2.mp3", false, false);

    QList<QueryItem> hits = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(hits.size(), 0);

    // Also test the fileName overload with a non-existent name.
    QVERIFY(!ds->getRuleHit("nonexistent.mp3"));
}
