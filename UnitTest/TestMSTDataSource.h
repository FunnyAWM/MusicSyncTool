#ifndef TESTMSTDATASOURCE_H
#define TESTMSTDATASOURCE_H

#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Data/QueryItem.h"
#include "../src/Services/MSTDataSource.h"

using namespace PROPERTIES;

class TestMSTDataSource : public QObject {
    Q_OBJECT

private:
    QTemporaryDir *tempDir  = nullptr;
    MSTDataSource *ds       = nullptr;
    int connectionCounter    = 0;
    QString dbDirPath;

    void insertRecord(const QString &title, const QString &artist,
                      const QString &album,  const QString &genre,
                      int year, int track,   const QString &fileName,
                      bool favorite = false, bool ruleHit = false)
    {
        ds->prepareStatement(
            "INSERT INTO musicInfo (title, artist, album, genre, year, track, "
            "fileName, favorite, ruleHit) "
            "VALUES (:title, :artist, :album, :genre, :year, :track, "
            ":fileName, :favorite, :ruleHit)");
        ds->bindValue(":title",    title);
        ds->bindValue(":artist",   artist);
        ds->bindValue(":album",    album);
        ds->bindValue(":genre",    genre);
        ds->bindValue(":year",     year);
        ds->bindValue(":track",    track);
        ds->bindValue(":fileName", fileName);
        ds->bindValue(":favorite", favorite ? 1 : 0);
        ds->bindValue(":ruleHit",  ruleHit  ? 1 : 0);
        ds->execQuery();
    }

private Q_SLOTS:
    void init();
    void cleanup();
    void testSetPath();
    void testSetConnectionName();
    void testOpenDB_withPath();
    void testOpenDB_withSetPath();
    void testCloseDB();
    void testIsOpen();
    void testDefaultPageSize();
    void testSetPageSize();
    void testInitTable();
    void testAddMusic_single_nonExistentFile();
    void testAddMusic_batch_nonExistentFiles();
    void testAddMusic_single_emptyString();
    void testDeleteMusic();
    void testDeleteMusic_emptyList();
    void testGetCount();
    void testGetAll_allFields();
    void testGetAll_fileNameOnly();
    void testSearchMusic_byTitle();
    void testSearchMusic_byArtist();
    void testSearchMusic_noMatch();
    void testGetFileNameByMetadata();
    void testGetMusicToTable_page1();
    void testGetMusicToTable_sortByTitle();
    void testGetMusicToTable_sortByArtist();
    void testGetMusicToTable_orderDesc();
    void testUpdateFavoriteBatch();
    void testGetFavorite();
    void testGetLastFavoriteCount();
    void testUpdateRuleHitBatch();
    void testGetRuleHit_byFileName();
    void testGetRuleHit_notFound();
    void testGetRuleHit_paginated();
};

#endif // TESTMSTDATASOURCE_H
