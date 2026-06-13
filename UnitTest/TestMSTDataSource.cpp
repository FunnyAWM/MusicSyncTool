/**
 * @file TestMSTDataSource.cpp
 * @brief Unit tests for MSTDataSource (Services layer)
 * @details Tests database connection management, CRUD operations, queries,
 *          pagination, sorting, favorites and rule-hit batch updates.
 *          Uses a QTemporaryDir so each test runs against a fresh SQLite DB.
 *          Cross-platform (Windows / Linux).
 */

#include "TestMSTDataSource.h"

// ======================== Fixture ========================

void TestMSTDataSource::init()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());

    // Normalise to forward slashes and ensure a trailing '/' so that
    // MSTDataSource::openDB() path parsing works on every platform.
    dbDirPath = QDir::fromNativeSeparators(tempDir->path()) + QStringLiteral("/");

    ds = new MSTDataSource();
    ++connectionCounter;
    ds->setConnectionName(QStringLiteral("test_ds_%1").arg(connectionCounter));
    ds->setPath(dbDirPath);
    QVERIFY(ds->openDB());
    ds->initTable();
}

void TestMSTDataSource::cleanup()
{
    if (ds) {
        ds->closeDB();
        delete ds;
        ds = nullptr;
    }
    // Remove the QSqlDatabase connection to avoid Qt SQL warnings on the
    // next test run that reuses the same counter value.
    QString connName = QStringLiteral("test_ds_%1").arg(connectionCounter);
    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase::removeDatabase(connName);
    }
    delete tempDir;
    tempDir = nullptr;
}

// ======================== Connection & open ========================

void TestMSTDataSource::testSetPath()
{
    const QString newPath = "/some/custom/path/";
    ds->setPath(newPath);
    QCOMPARE(ds->getPath(), newPath);
}

void TestMSTDataSource::testSetConnectionName()
{
    // setConnectionName is used during openDB; no getter exists, but we can
    // verify it doesn't crash and the DB still opens correctly.
    // The connection was already opened in init() with a name, so just
    // confirm the DB is still open.
    QVERIFY(ds->isOpen());
}

void TestMSTDataSource::testOpenDB_withPath()
{
    // Close the connection opened in init(), then reopen via openDB(path_).
    ds->closeDB();
    QString connName = QStringLiteral("test_ds_%1").arg(connectionCounter);
    QSqlDatabase::removeDatabase(connName);

    ++connectionCounter;
    ds->setConnectionName(QStringLiteral("test_ds_%1").arg(connectionCounter));

    QVERIFY(ds->openDB(dbDirPath));
    QVERIFY(ds->isOpen());
}

void TestMSTDataSource::testOpenDB_withSetPath()
{
    // Close and reopen using the no-argument openDB() (uses setPath value).
    ds->closeDB();
    QString connName = QStringLiteral("test_ds_%1").arg(connectionCounter);
    QSqlDatabase::removeDatabase(connName);

    ++connectionCounter;
    ds->setConnectionName(QStringLiteral("test_ds_%1").arg(connectionCounter));

    QVERIFY(ds->openDB());
    QVERIFY(ds->isOpen());
}

void TestMSTDataSource::testCloseDB()
{
    QVERIFY(ds->isOpen());
    ds->closeDB();
    QVERIFY(!ds->isOpen());
}

void TestMSTDataSource::testIsOpen()
{
    // DB was opened in init()
    QVERIFY(ds->isOpen());

    ds->closeDB();
    QVERIFY(!ds->isOpen());
}

// ======================== Page size ========================

void TestMSTDataSource::testDefaultPageSize()
{
    QCOMPARE(ds->getPageSize(), 200);
}

void TestMSTDataSource::testSetPageSize()
{
    ds->setPageSize(50);
    QCOMPARE(ds->getPageSize(), 50);

    ds->setPageSize(1);
    QCOMPARE(ds->getPageSize(), 1);
}

// ======================== CRUD ========================

void TestMSTDataSource::testInitTable()
{
    // The table was already created in init().  Insert a row to prove
    // the table exists and accepts the expected columns.
    insertRecord("InitTest", "Artist", "Album", "Pop", 2024, 1, "init.mp3");
    QCOMPARE(ds->getCount(), 1);
}

void TestMSTDataSource::testAddMusic_single_nonExistentFile()
{
    // addMusic reads tags via TagLib, so a non-existent file must fail gracefully.
    bool result = ds->addMusic("nonexistent_song.mp3");
    QVERIFY2(!result, "addMusic should return false for a non-existent file");
    QCOMPARE(ds->getCount(), 0);
}

void TestMSTDataSource::testAddMusic_batch_nonExistentFiles()
{
    QStringList files = {"no_such_a.mp3", "no_such_b.flac"};
    QStringList errList = ds->addMusic(files);
    // Both files are fake, so both should appear in the error list.
    QCOMPARE(errList.size(), 2);
    QVERIFY(errList.contains("no_such_a.mp3"));
    QVERIFY(errList.contains("no_such_b.flac"));
    QCOMPARE(ds->getCount(), 0);
}

void TestMSTDataSource::testAddMusic_single_emptyString()
{
    bool result = ds->addMusic(QString());
    QVERIFY2(!result, "addMusic should return false for an empty file path");
}

void TestMSTDataSource::testDeleteMusic()
{
    insertRecord("Del_A", "ArtA", "AlbA", "Pop",  2024, 1, "del_a.mp3");
    insertRecord("Del_B", "ArtB", "AlbB", "Rock", 2024, 2, "del_b.mp3");
    QCOMPARE(ds->getCount(), 2);

    bool ok = ds->deleteMusic(QStringList{"del_a.mp3"});
    QVERIFY(ok);
    QCOMPARE(ds->getCount(), 1);

    ok = ds->deleteMusic(QStringList{"del_b.mp3"});
    QVERIFY(ok);
    QCOMPARE(ds->getCount(), 0);
}

void TestMSTDataSource::testDeleteMusic_emptyList()
{
    // An empty list is a no-op and should return true.
    QVERIFY(ds->deleteMusic(QStringList()));
}

void TestMSTDataSource::testGetCount()
{
    QCOMPARE(ds->getCount(), 0);

    insertRecord("Song1", "Art1", "Alb1", "Pop",  2024, 1, "s1.mp3");
    QCOMPARE(ds->getCount(), 1);

    insertRecord("Song2", "Art2", "Alb2", "Rock", 2024, 2, "s2.mp3");
    QCOMPARE(ds->getCount(), 2);

    insertRecord("Song3", "Art3", "Alb3", "Jazz", 2024, 3, "s3.mp3");
    QCOMPARE(ds->getCount(), 3);
}

// ======================== Query ========================

void TestMSTDataSource::testGetAll_allFields()
{
    insertRecord("TitleX", "ArtistX", "AlbumX", "Rock", 2023, 7, "fileX.mp3");

    QList<QueryItem> results = ds->getAll();   // default: QueryRows::ALL
    QCOMPARE(results.size(), 1);

    QCOMPARE(results[0].getTitle(),    QString("TitleX"));
    QCOMPARE(results[0].getArtist(),   QString("ArtistX"));
    QCOMPARE(results[0].getAlbum(),    QString("AlbumX"));
    QCOMPARE(results[0].getGenre(),    QString("Rock"));
    QCOMPARE(results[0].getYear(),     2023u);
    QCOMPARE(results[0].getTrack(),    7u);
    QCOMPARE(results[0].getFileName(), QString("fileX.mp3"));
}

void TestMSTDataSource::testGetAll_fileNameOnly()
{
    insertRecord("TitleY", "ArtistY", "AlbumY", "Jazz", 2022, 3, "fileY.flac");

    QList<QueryItem> results = ds->getAll({QueryRows::FILENAME});
    QCOMPARE(results.size(), 1);

    // When only FILENAME is requested, the SQL selects "fileName" as column 0.
    // The implementation maps column 0 to setTitle() inside the FILENAME case
    // of the rowMap loop.  Verify the fileName column value is retrievable.
    QCOMPARE(results[0].getFileName(), QString("fileY.flac"));
}

void TestMSTDataSource::testSearchMusic_byTitle()
{
    insertRecord("UniqueTitle", "SomeArtist", "SomeAlbum", "Pop", 2024, 1, "t1.mp3");
    insertRecord("OtherTitle",  "SomeArtist", "SomeAlbum", "Pop", 2024, 2, "t2.mp3");

    QList<QueryItem> results = ds->searchMusic("UniqueTitle");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].getTitle(),  QString("UniqueTitle"));
    QCOMPARE(results[0].getFileName(), QString("t1.mp3"));
}

void TestMSTDataSource::testSearchMusic_byArtist()
{
    insertRecord("TitleA", "UniqueArtist", "AlbumA", "Rock", 2024, 1, "a1.mp3");
    insertRecord("TitleB", "OtherArtist",  "AlbumB", "Rock", 2024, 2, "a2.mp3");

    // searchMusic matches exact equality on artist column.
    QList<QueryItem> results = ds->searchMusic("UniqueArtist");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0].getArtist(), QString("UniqueArtist"));
}

void TestMSTDataSource::testSearchMusic_noMatch()
{
    insertRecord("Song", "Band", "Record", "Pop", 2024, 1, "x.mp3");

    QList<QueryItem> results = ds->searchMusic("NonexistentQuery");
    QCOMPARE(results.size(), 0);
}

void TestMSTDataSource::testGetFileNameByMetadata()
{
    insertRecord("MDTitle", "MDArtist", "MDAlbum", "Pop", 2024, 1, "md_song.mp3");

    QueryItem searchItem("MDTitle", "MDArtist", "MDAlbum", "Pop", 2024, 1, "");
    QStringList fileNames = ds->getFileNameByMetadata({searchItem});

    QCOMPARE(fileNames.size(), 1);
    QCOMPARE(fileNames[0], QString("md_song.mp3"));
}

// ======================== Pagination ========================

void TestMSTDataSource::testGetMusicToTable_page1()
{
    ds->setPageSize(3);
    insertRecord("Banana", "ArtB", "Alb", "Pop", 2024, 1, "b.mp3");
    insertRecord("Apple",  "ArtA", "Alb", "Pop", 2024, 2, "a.mp3");
    insertRecord("Cherry", "ArtC", "Alb", "Pop", 2024, 3, "c.mp3");
    insertRecord("Delta",  "ArtD", "Alb", "Pop", 2024, 4, "d.mp3");
    insertRecord("Echo",   "ArtE", "Alb", "Pop", 2024, 5, "e.mp3");

    // Default sort: TITLE ASC
    QList<QueryItem> page1 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(page1.size(), 3);
    QCOMPARE(page1[0].getTitle(), QString("Apple"));
    QCOMPARE(page1[1].getTitle(), QString("Banana"));
    QCOMPARE(page1[2].getTitle(), QString("Cherry"));
}

void TestMSTDataSource::testGetMusicToTable_sortByTitle()
{
    ds->setPageSize(10);
    insertRecord("Cherry", "ArtC", "Alb", "Pop", 2024, 1, "c.mp3");
    insertRecord("Apple",  "ArtA", "Alb", "Pop", 2024, 2, "a.mp3");
    insertRecord("Banana", "ArtB", "Alb", "Pop", 2024, 3, "b.mp3");

    QList<QueryItem> results = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(results.size(), 3);
    QCOMPARE(results[0].getTitle(), QString("Apple"));
    QCOMPARE(results[1].getTitle(), QString("Banana"));
    QCOMPARE(results[2].getTitle(), QString("Cherry"));
}

void TestMSTDataSource::testGetMusicToTable_sortByArtist()
{
    ds->setPageSize(10);
    insertRecord("SongC", "Zack",   "Alb", "Pop", 2024, 1, "c.mp3");
    insertRecord("SongA", "Alpha",  "Alb", "Pop", 2024, 2, "a.mp3");
    insertRecord("SongB", "Middle", "Alb", "Pop", 2024, 3, "b.mp3");

    QList<QueryItem> results = ds->getMusicToTable(1, SortByEnum::ARTIST, OrderByEnum::ASC);
    QCOMPARE(results.size(), 3);
    QCOMPARE(results[0].getArtist(), QString("Alpha"));
    QCOMPARE(results[1].getArtist(), QString("Middle"));
    QCOMPARE(results[2].getArtist(), QString("Zack"));
}

void TestMSTDataSource::testGetMusicToTable_orderDesc()
{
    ds->setPageSize(10);
    insertRecord("Apple",  "ArtA", "Alb", "Pop", 2024, 1, "a.mp3");
    insertRecord("Banana", "ArtB", "Alb", "Pop", 2024, 2, "b.mp3");
    insertRecord("Cherry", "ArtC", "Alb", "Pop", 2024, 3, "c.mp3");

    QList<QueryItem> results = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::DESC);
    QCOMPARE(results.size(), 3);
    QCOMPARE(results[0].getTitle(), QString("Cherry"));
    QCOMPARE(results[1].getTitle(), QString("Banana"));
    QCOMPARE(results[2].getTitle(), QString("Apple"));
}

// ======================== Favorite & RuleHit ========================

void TestMSTDataSource::testUpdateFavoriteBatch()
{
    insertRecord("FavA", "Art", "Alb", "Pop", 2024, 1, "fa.mp3");
    insertRecord("FavB", "Art", "Alb", "Pop", 2024, 2, "fb.mp3");
    insertRecord("FavC", "Art", "Alb", "Pop", 2024, 3, "fc.mp3");

    QList<FavoriteUpdate> updates = {
        {"fa.mp3", true},
        {"fb.mp3", true},
        {"fc.mp3", false},
    };
    ds->updateFavoriteBatch(updates);

    // Verify: getFavorite should return exactly the two marked as favorite.
    QList<QueryItem> favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 2);
    QCOMPARE(favs[0].getTitle(), QString("FavA"));
    QCOMPARE(favs[1].getTitle(), QString("FavB"));
}

void TestMSTDataSource::testGetFavorite()
{
    insertRecord("F1", "Art", "Alb", "Pop", 2024, 1, "f1.mp3", /*favorite=*/true);
    insertRecord("F2", "Art", "Alb", "Pop", 2024, 2, "f2.mp3", /*favorite=*/true);
    insertRecord("F3", "Art", "Alb", "Pop", 2024, 3, "f3.mp3", /*favorite=*/false);

    QList<QueryItem> favs = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(favs.size(), 2);
    QCOMPARE(favs[0].getTitle(), QString("F1"));
    QCOMPARE(favs[1].getTitle(), QString("F2"));
}

void TestMSTDataSource::testGetLastFavoriteCount()
{
    insertRecord("LC1", "Art", "Alb", "Pop", 2024, 1, "lc1.mp3", true);
    insertRecord("LC2", "Art", "Alb", "Pop", 2024, 2, "lc2.mp3", true);
    insertRecord("LC3", "Art", "Alb", "Pop", 2024, 3, "lc3.mp3", true);
    insertRecord("LC4", "Art", "Alb", "Pop", 2024, 4, "lc4.mp3", false);

    // lastFavoriteCount starts at 0 (set in class definition).
    // getFavorite updates it.
    ds->setPageSize(2);
    ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);

    // Total favorites = 3, even though page size is 2.
    QCOMPARE(ds->getLastFavoriteCount(), 3);
}

void TestMSTDataSource::testUpdateRuleHitBatch()
{
    insertRecord("RH1", "Art", "Alb", "Pop", 2024, 1, "rh1.mp3");
    insertRecord("RH2", "Art", "Alb", "Pop", 2024, 2, "rh2.mp3");
    insertRecord("RH3", "Art", "Alb", "Pop", 2024, 3, "rh3.mp3");

    QList<RuleHitUpdate> updates = {
        {"rh1.mp3", true},
        {"rh2.mp3", false},
        {"rh3.mp3", true},
    };
    ds->updateRuleHitBatch(updates);

    // Paginated query for rule-hit records.
    QList<QueryItem> hits = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(hits.size(), 2);
    QCOMPARE(hits[0].getTitle(), QString("RH1"));
    QCOMPARE(hits[1].getTitle(), QString("RH3"));
}

void TestMSTDataSource::testGetRuleHit_byFileName()
{
    insertRecord("HitSong",  "Art", "Alb", "Pop", 2024, 1, "hit.mp3",  false, true);
    insertRecord("MissSong", "Art", "Alb", "Pop", 2024, 2, "miss.mp3", false, false);

    QVERIFY(ds->getRuleHit("hit.mp3"));
    QVERIFY(!ds->getRuleHit("miss.mp3"));
}

void TestMSTDataSource::testGetRuleHit_notFound()
{
    insertRecord("Only", "Art", "Alb", "Pop", 2024, 1, "only.mp3", false, false);

    QVERIFY2(!ds->getRuleHit("does_not_exist.mp3"),
             "getRuleHit should return false for a non-existent fileName");
}

void TestMSTDataSource::testGetRuleHit_paginated()
{
    ds->setPageSize(2);
    insertRecord("R1", "Art", "Alb", "Pop", 2024, 1, "r1.mp3", false, true);
    insertRecord("R2", "Art", "Alb", "Pop", 2024, 2, "r2.mp3", false, true);
    insertRecord("R3", "Art", "Alb", "Pop", 2024, 3, "r3.mp3", false, true);

    QList<QueryItem> page1 = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(page1.size(), 2);
    QCOMPARE(page1[0].getTitle(), QString("R1"));
    QCOMPARE(page1[1].getTitle(), QString("R2"));

    QList<QueryItem> page2 = ds->getRuleHit(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(page2.size(), 1);
    QCOMPARE(page2[0].getTitle(), QString("R3"));
}
