#ifndef TESTMSTDATASOURCE_H
#define TESTMSTDATASOURCE_H

#include <QDir>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>
#include "../src/Data/QueryItem.h"
#include "../src/Services/MSTDataSource.h"

using namespace PROPERTIES;

class TestMSTDataSource : public QObject {
    Q_OBJECT
private:
    QTemporaryDir *tmp = nullptr;
    MSTDataSource *ds = nullptr;
    int conn = 0;
    QString dbPath;
    void init();
    void cleanup();
    void insert(const QString &t, const QString &a, const QString &al,
                const QString &g, int y, int tr, const QString &fn,
                bool fav = false, bool rh = false) {
        ds->prepareStatement("INSERT INTO musicInfo(title,artist,album,genre,year,track,fileName,favorite,ruleHit) VALUES(:t,:a,:al,:g,:y,:tr,:fn,:fav,:rh)");
        ds->bindValue(":t",t); ds->bindValue(":a",a); ds->bindValue(":al",al);
        ds->bindValue(":g",g); ds->bindValue(":y",y); ds->bindValue(":tr",tr);
        ds->bindValue(":fn",fn); ds->bindValue(":fav",fav?1:0); ds->bindValue(":rh",rh?1:0);
        ds->execQuery();
    }
private Q_SLOTS:
    void openClose();
    void pageSize();
    void initTable();
    void addMusic_nonexistent();
    void addMusic_empty();
    void deleteMusic();
    void deleteMusic_empty();
    void getCount();
    void getAll();
    void getByFileNames();
    void searchMusic();
    void searchMusic_noMatch();
    void getFileNameByMetadata();
    void getMusicToTable_sort();
    void getMusicToTable_page();
    void favorite();
    void ruleHit();
};

#endif