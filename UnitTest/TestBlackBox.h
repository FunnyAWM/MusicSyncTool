#ifndef TESTBLACKBOX_H
#define TESTBLACKBOX_H

#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Core/MusicProperties.h"
#include "../src/Core/SettingEntity.h"
#include "../src/Data/LyricIgnoreRule.h"
#include "../src/Data/QueryItem.h"
#include "../src/Services/MSTDataSource.h"
#include "../src/Services/MSTFileManager.h"
#include "../src/Services/MSTSettingsManager.h"

using namespace PROPERTIES;

class TestBlackBox : public QObject {
    Q_OBJECT

private:
    QTemporaryDir *tempDir = nullptr;
    MSTDataSource *ds      = nullptr;
    int connectionCounter   = 0;
    QString dbDirPath;
    QString originalWorkDir;

    void insertRecord(const QString &title,  const QString &artist,
                      const QString &album,  const QString &genre,
                      int year, int track,   const QString &fileName,
                      bool favorite = false, bool ruleHit  = false)
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

    void seedRecords()
    {
        struct S { const char *t; const char *a; const char *al; bool f; bool r; };
        static const S s[] = {
            {"Apple","ArtA","AlbX",true,false},  {"Banana","ArtB","AlbY",true,false},
            {"Cherry","ArtC","AlbX",true,false}, {"Delta","ArtA","AlbY",true,false},
            {"Echo","ArtB","AlbX",false,false},  {"Foxtrot","ArtC","AlbY",false,false},
            {"Golf","ArtA","AlbX",false,true},   {"Hotel","ArtB","AlbY",false,true},
            {"India","ArtC","AlbX",false,true},  {"Juliet","ArtA","AlbY",false,true},
            {"Kilo","ArtB","AlbX",false,false},  {"Lima","ArtC","AlbY",false,false},
        };
        for (int i = 0; i < 12; ++i)
            insertRecord(s[i].t, s[i].a, s[i].al, "Pop", 2024, i+1,
                         QString("%1.mp3").arg(s[i].t).toLower(), s[i].f, s[i].r);
    }

private Q_SLOTS:
    void init();
    void cleanup();
    void testPagination_firstPage();
    void testPagination_middlePage();
    void testPagination_lastPage();
    void testPagination_emptyDatabase();
    void testPagination_singleRecord();
    void testPagination_exactlyOnePageSize();
    void testSearch_byExactTitle();
    void testSearch_byExactArtist();
    void testSearch_byExactAlbum();
    void testSearch_noMatch();
    void testSearch_emptyString();
    void testSort_titleAscending();
    void testSort_titleDescending();
    void testSort_artistAscending();
    void testSort_artistDescending();
    void testSettings_roundtrip();
    void testSettings_missingFile();
    void testFormatValidation_supported();
    void testFormatValidation_unsupported();
    void testFormatValidation_noExtension();
    void testFormatValidation_uppercase();
    void testFavorite_allFavorites();
    void testFavorite_noFavorites();
    void testFavorite_someFavorites();
    void testFavorite_lastFavoriteCount();
    void testRuleHit_includesRule();
    void testRuleHit_excludesRule();
    void testRuleHit_noMatch();
};

#endif // TESTBLACKBOX_H
