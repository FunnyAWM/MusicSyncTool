#ifndef TESTBLACKBOX_H
#define TESTBLACKBOX_H

#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Core/SettingEntity.h"
#include "../src/Data/QueryItem.h"
#include "../src/Services/MSTDataSource.h"
#include "../src/Services/MSTFileManager.h"
#include "../src/Services/MSTSettingsManager.h"

using namespace PROPERTIES;

class TestBlackBox : public QObject {
    Q_OBJECT
private:
    QTemporaryDir *tmp = nullptr;
    MSTDataSource *ds = nullptr;
    int conn = 0;
    QString dbPath, origCwd;
    void init();
    void cleanup();
    void insert(const QString &t, const QString &a, const QString &al,
                const QString &g, int y, int tr, const QString &fn,
                bool fav = false, bool rh = false) {
        ds->prepareStatement("INSERT INTO musicInfo VALUES(:t,:a,:al,:g,:y,:tr,:fav,:rh,:fn)");
        ds->bindValue(":t",t); ds->bindValue(":a",a); ds->bindValue(":al",al);
        ds->bindValue(":g",g); ds->bindValue(":y",y); ds->bindValue(":tr",tr);
        ds->bindValue(":fn",fn); ds->bindValue(":fav",fav?1:0); ds->bindValue(":rh",rh?1:0);
        ds->execQuery();
    }
    void seed() {
        struct R { const char *t,*a,*al; bool f,r; };
        const R s[] = {
            {"Apple","ArtA","AlbX",true,false}, {"Banana","ArtB","AlbY",true,false},
            {"Cherry","ArtC","AlbX",true,false}, {"Delta","ArtA","AlbY",true,false},
            {"Echo","ArtB","AlbX",false,false}, {"Foxtrot","ArtC","AlbY",false,false},
            {"Golf","ArtA","AlbX",false,true}, {"Hotel","ArtB","AlbY",false,true},
            {"India","ArtC","AlbX",false,true}, {"Juliet","ArtA","AlbY",false,true},
            {"Kilo","ArtB","AlbX",false,false}, {"Lima","ArtC","AlbY",false,false},
        };
        for (int i=0;i<12;++i)
            insert(s[i].t,s[i].a,s[i].al,"Pop",2024,i+1,QString("%1.mp3").arg(s[i].t).toLower(),s[i].f,s[i].r);
    }
private Q_SLOTS:
    void pagination_firstPage();
    void pagination_middlePage();
    void pagination_lastPage();
    void pagination_emptyDatabase();
    void pagination_singleRecord();
    void pagination_exactlyOnePageSize();
    void search_byExactTitle();
    void search_byExactArtist();
    void search_byExactAlbum();
    void search_noMatch();
    void search_emptyString();
    void sort_titleAscending();
    void sort_titleDescending();
    void sort_artistAscending();
    void sort_artistDescending();
    void settings_roundtrip();
    void settings_missingFile();
    void formatValidation_supported();
    void formatValidation_unsupported();
    void formatValidation_noExtension();
    void formatValidation_uppercase();
    void favorite_allFavorites();
    void favorite_noFavorites();
    void favorite_someFavorites();
    void favorite_lastFavoriteCount();
    void ruleHit_includesRule();
    void ruleHit_excludesRule();
    void ruleHit_noMatch();
};

#endif