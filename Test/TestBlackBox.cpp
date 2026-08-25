#include "TestBlackBox.h"

void TestBlackBox::init() {
    origCwd = QDir::currentPath();
    tmp = new QTemporaryDir(); QVERIFY(tmp->isValid());
    dbPath = QDir::fromNativeSeparators(tmp->path()) + "/";
    ds = new MSTDataSource(); ++conn;
    ds->setConnectionName(QString("bb_%1").arg(conn));
    ds->setPath(dbPath); QVERIFY(ds->openDB()); ds->initTable();
    QDir::setCurrent(tmp->path());
}
void TestBlackBox::cleanup() {
    QDir::setCurrent(origCwd);
    if (ds) { ds->closeDB(); delete ds; ds = nullptr; }
    QString cn = QString("bb_%1").arg(conn);
    if (QSqlDatabase::contains(cn)) QSqlDatabase::removeDatabase(cn);
    delete tmp; tmp = nullptr;
}

void TestBlackBox::pagination_firstPage() { ds->setPageSize(5); seed(); auto p=ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(p.size(),5); QCOMPARE(p[0].getTitle(),QString("Apple")); }
void TestBlackBox::pagination_middlePage() { ds->setPageSize(5); seed(); auto p=ds->getMusicToTable(2,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(p.size(),5); QCOMPARE(p[0].getTitle(),QString("Foxtrot")); }
void TestBlackBox::pagination_lastPage() { ds->setPageSize(5); seed(); auto p=ds->getMusicToTable(3,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(p.size(),2); QCOMPARE(p[0].getTitle(),QString("Kilo")); }
void TestBlackBox::pagination_emptyDatabase() { ds->setPageSize(5); QCOMPARE(ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),0); }
void TestBlackBox::pagination_singleRecord() { ds->setPageSize(5); insert("Only","A","B","C",2024,1,"only.mp3"); QCOMPARE(ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),1); }
void TestBlackBox::pagination_exactlyOnePageSize() { ds->setPageSize(5); for(int i=1;i<=5;++i) insert(QString("S%1").arg(i),"A","B","C",2024,i,QString("s%1.mp3").arg(i)); QCOMPARE(ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),5); QCOMPARE(ds->getMusicToTable(2,SortByEnum::TITLE,OrderByEnum::ASC).size(),0); }

void TestBlackBox::search_byExactTitle() { seed(); auto r=ds->searchMusic("Cherry"); QCOMPARE(r.size(),1); QCOMPARE(r[0].getTitle(),QString("Cherry")); }
void TestBlackBox::search_byExactArtist() { seed(); auto r=ds->searchMusic("ArtA"); QCOMPARE(r.size(),4); }
void TestBlackBox::search_byExactAlbum() { seed(); auto r=ds->searchMusic("AlbX"); QCOMPARE(r.size(),6); }
void TestBlackBox::search_noMatch() { seed(); QCOMPARE(ds->searchMusic("ZZZZZ").size(),0); }
void TestBlackBox::search_emptyString() { seed(); QCOMPARE(ds->searchMusic("").size(),0); }

void TestBlackBox::sort_titleAscending() { ds->setPageSize(12); seed(); auto a=ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(a.size(),12); for(int i=1;i<a.size();++i) QVERIFY(a[i].getTitle()>=a[i-1].getTitle()); }
void TestBlackBox::sort_titleDescending() { ds->setPageSize(12); seed(); auto a=ds->getMusicToTable(1,SortByEnum::TITLE,OrderByEnum::DESC); QCOMPARE(a.size(),12); for(int i=1;i<a.size();++i) QVERIFY(a[i].getTitle()<=a[i-1].getTitle()); }
void TestBlackBox::sort_artistAscending() { ds->setPageSize(12); seed(); auto a=ds->getMusicToTable(1,SortByEnum::ARTIST,OrderByEnum::ASC); for(int i=1;i<a.size();++i) QVERIFY(a[i].getArtist()>=a[i-1].getArtist()); }
void TestBlackBox::sort_artistDescending() { ds->setPageSize(12); seed(); auto a=ds->getMusicToTable(1,SortByEnum::ARTIST,OrderByEnum::DESC); for(int i=1;i<a.size();++i) QVERIFY(a[i].getArtist()<=a[i-1].getArtist()); }

void TestBlackBox::settings_roundtrip() { SettingsData o; o.ignoreLyric=true; o.sortBy=toShort(SortByEnum::ALBUM); o.orderBy=toShort(OrderByEnum::DESC); o.language="en_US"; o.favoriteTag="STAR"; o.recursiveScan=true; QVERIFY(MSTSettingsManager::saveSettings(o)); SettingsData l{}; QVERIFY(MSTSettingsManager::loadSettings(l)); QCOMPARE(l.sortBy,o.sortBy); }
void TestBlackBox::settings_missingFile() { QVERIFY(!QFile::exists("settings.json")); SettingsData e{}; QVERIFY(!MSTSettingsManager::loadSettings(e)); QVERIFY(QFile::exists("settings.json")); }

void TestBlackBox::formatValidation_supported() { for(auto& f:{"mp3","flac","wav","aac","ogg","wma","m4a","ape","aiff","opus"}) QVERIFY(MSTFileManager::isFormatSupported(QString("x.")+f)); }
void TestBlackBox::formatValidation_unsupported() { QVERIFY(!MSTFileManager::isFormatSupported("f.txt")); QVERIFY(!MSTFileManager::isFormatSupported("f.exe")); }
void TestBlackBox::formatValidation_noExtension() { QVERIFY(!MSTFileManager::isFormatSupported("noext")); QVERIFY(!MSTFileManager::isFormatSupported("")); }
void TestBlackBox::formatValidation_uppercase() { QVERIFY(MSTFileManager::isFormatSupported("X.MP3")); QVERIFY(MSTFileManager::isFormatSupported("X.FLAC")); }

void TestBlackBox::favorite_allFavorites() { ds->setPageSize(10); insert("F1","A","B","C",2024,1,"f1.mp3",true); insert("F2","A","B","C",2024,2,"f2.mp3",true); insert("F3","A","B","C",2024,3,"f3.mp3",true); QCOMPARE(ds->getFavorite(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),3); QCOMPARE(ds->getLastFavoriteCount(),3); }
void TestBlackBox::favorite_noFavorites() { ds->setPageSize(10); insert("X1","A","B","C",2024,1,"x1.mp3",false); insert("X2","A","B","C",2024,2,"x2.mp3",false); QCOMPARE(ds->getFavorite(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),0); QCOMPARE(ds->getLastFavoriteCount(),0); }
void TestBlackBox::favorite_someFavorites() { ds->setPageSize(12); seed(); auto f=ds->getFavorite(1,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(f.size(),4); QCOMPARE(f[0].getTitle(),QString("Apple")); QCOMPARE(f[3].getTitle(),QString("Delta")); }
void TestBlackBox::favorite_lastFavoriteCount() { ds->setPageSize(2); seed(); ds->getFavorite(1,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(ds->getLastFavoriteCount(),4); }

void TestBlackBox::ruleHit_includesRule() { ds->setPageSize(12); seed(); auto h=ds->getRuleHit(1,SortByEnum::TITLE,OrderByEnum::ASC); QCOMPARE(h.size(),4); QCOMPARE(h[0].getTitle(),QString("Golf")); QCOMPARE(h[3].getTitle(),QString("Juliet")); }
void TestBlackBox::ruleHit_excludesRule() { ds->setPageSize(12); insert("ExA","A","B","C",2024,1,"exa.mp3",false,false); insert("ExB","A","B","C",2024,2,"exb.mp3",false,false); QCOMPARE(ds->getRuleHit(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),0); QVERIFY(!ds->getRuleHit("exa.mp3")); QVERIFY(!ds->getRuleHit("exb.mp3")); }
void TestBlackBox::ruleHit_noMatch() { ds->setPageSize(12); seed(); ds->prepareStatement("DELETE FROM musicInfo"); ds->execQuery(); insert("NR1","A","B","C",2024,1,"nr1.mp3",false,false); insert("NR2","A","B","C",2024,2,"nr2.mp3",false,false); QCOMPARE(ds->getRuleHit(1,SortByEnum::TITLE,OrderByEnum::ASC).size(),0); QVERIFY(!ds->getRuleHit("nonexistent.mp3")); }