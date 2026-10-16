#include "TestMSTDataSource.h"

void TestMSTDataSource::init() {
    tmp = new QTemporaryDir(); QVERIFY(tmp->isValid());
    dbPath = QDir::fromNativeSeparators(tmp->path()) + "/";
    ds = new MSTDataSource(); ++conn;
    ds->setConnectionName(QString("tds_%1").arg(conn));
    ds->setPath(dbPath); QVERIFY(ds->openDB()); ds->initTable();
}
void TestMSTDataSource::cleanup() {
    if (ds) { ds->closeDB(); delete ds; ds = nullptr; }
    QString cn = QString("tds_%1").arg(conn);
    if (QSqlDatabase::contains(cn)) QSqlDatabase::removeDatabase(cn);
    delete tmp; tmp = nullptr;
}

void TestMSTDataSource::openClose() {
    QVERIFY(ds->isOpen()); ds->closeDB(); QVERIFY(!ds->isOpen());
    ds->setConnectionName(QString("tds_r_%1").arg(++conn));
    QVERIFY(ds->openDB()); QVERIFY(ds->isOpen());
}

void TestMSTDataSource::pageSize() {
    QCOMPARE(ds->getPageSize(), 200);
    ds->setPageSize(50); QCOMPARE(ds->getPageSize(), 50);
}

void TestMSTDataSource::initTable() {
    insert("A","B","C","D",2024,1,"a.mp3");
    QCOMPARE(ds->getCount(), 1);
}

void TestMSTDataSource::addMusic_nonexistent() {
    QVERIFY(!ds->addMusic("nonexistent.mp3"));
    QStringList fl = ds->addMusic({"a.mp3","b.flac"});
    QCOMPARE(fl.size(), 2);
    QCOMPARE(ds->getCount(), 0);
}

void TestMSTDataSource::addMusic_empty() {
    QVERIFY(!ds->addMusic(QString()));
}

void TestMSTDataSource::deleteMusic() {
    insert("A","B","C","D",2024,1,"da.mp3"); insert("B","B","C","D",2024,2,"db.mp3");
    QCOMPARE(ds->getCount(), 2);
    QVERIFY(ds->deleteMusic({"da.mp3"}));
    QCOMPARE(ds->getCount(), 1);
}

void TestMSTDataSource::deleteMusic_empty() {
    QVERIFY(ds->deleteMusic(QStringList()));
}

void TestMSTDataSource::getCount() {
    QCOMPARE(ds->getCount(), 0);
    insert("A","B","C","D",2024,1,"a.mp3");
    QCOMPARE(ds->getCount(), 1);
}

void TestMSTDataSource::getAll() {
    insert("T","A","B","G",2023,7,"f.mp3");
    auto r = ds->getAll();
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].getTitle(), QString("T"));
    QCOMPARE(r[0].getFileName(), QString("f.mp3"));
}

void TestMSTDataSource::getByFileNames() {
    insert("X","Y","Z","W",2024,1,"x.mp3"); insert("X2","Y2","Z2","W2",2024,2,"y.mp3");
    auto r = ds->getByFileNames({"x.mp3"});
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].getTitle(), QString("X"));
}

void TestMSTDataSource::searchMusic() {
    insert("U","V","W","X",2024,1,"u.mp3");
    auto r = ds->searchMusic("U");
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].getFileName(), QString("u.mp3"));
}

void TestMSTDataSource::searchMusic_noMatch() {
    insert("A","B","C","D",2024,1,"a.mp3");
    QCOMPARE(ds->searchMusic("ZZZ").size(), 0);
}

void TestMSTDataSource::getFileNameByMetadata() {
    insert("M","N","O","P",2024,1,"m.mp3");
    QueryItem qi("M","N","O","P",2024,1,"");
    auto r = ds->getFileNameByMetadata({qi});
    QCOMPARE(r.size(), 1); QCOMPARE(r[0], QString("m.mp3"));
}

void TestMSTDataSource::getMusicToTable_sort() {
    ds->setPageSize(10);
    insert("C","Z","X","G",2024,1,"c.mp3"); insert("A","A","X","G",2024,2,"a.mp3"); insert("B","M","X","G",2024,3,"b.mp3");
    auto r = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(r.size(), 3);
    QCOMPARE(r[0].getTitle(), QString("A")); QCOMPARE(r[2].getTitle(), QString("C"));
    auto r2 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::DESC);
    QCOMPARE(r2[0].getTitle(), QString("C"));
    auto r3 = ds->getMusicToTable(1, SortByEnum::ARTIST, OrderByEnum::ASC);
    QCOMPARE(r3[0].getArtist(), QString("A"));
    QCOMPARE(r3[2].getArtist(), QString("Z"));
}

void TestMSTDataSource::getMusicToTable_page() {
    ds->setPageSize(2);
    for (int i = 0; i < 5; ++i) insert(QString("S%1").arg(i),"A","B","G",2024,i+1,QString("s%1.mp3").arg(i));
    auto p1 = ds->getMusicToTable(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p1.size(), 2);
    auto p2 = ds->getMusicToTable(2, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p2.size(), 2);
    auto p3 = ds->getMusicToTable(3, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(p3.size(), 1);
}

void TestMSTDataSource::favorite() {
    insert("F1","A","B","G",2024,1,"f1.mp3",true);
    insert("F2","A","B","G",2024,2,"f2.mp3",true);
    insert("F3","A","B","G",2024,3,"f3.mp3",false);
    auto r = ds->getFavorite(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(r.size(), 2);
    QCOMPARE(ds->getLastFavoriteCount(), 2);
}

void TestMSTDataSource::ruleHit() {
    insert("R1","A","B","G",2024,1,"r1.mp3",false,true);
    insert("R2","A","B","G",2024,2,"r2.mp3",false,false);
    QVERIFY(ds->getRuleHit("r1.mp3"));
    QVERIFY(!ds->getRuleHit("r2.mp3"));
    QVERIFY(!ds->getRuleHit("nonexistent.mp3"));
    auto r = ds->getRuleHit(1, SortByEnum::TITLE, OrderByEnum::ASC);
    QCOMPARE(r.size(), 1);
}

void TestMSTDataSource::schemaMigration_legacyTable() {
    // 另开连接模拟旧库：只有 fileName 一列且 user_version 为 0
    const QString legacyConn = QString("tds_legacy_%1").arg(conn);
    {
        MSTDataSource legacy;
        legacy.setConnectionName(legacyConn);
        legacy.setPath(dbPath);
        QVERIFY(legacy.openDB());
        legacy.prepareStatement("CREATE TABLE IF NOT EXISTS musicInfo (fileName TEXT)");
        QVERIFY(legacy.execQuery());
        legacy.prepareStatement("INSERT INTO musicInfo(fileName) VALUES('legacy.mp3')");
        QVERIFY(legacy.execQuery());

        legacy.initTable();
        // 迁移补列后，按完整列写入必须成功
        legacy.prepareStatement(
            "INSERT INTO musicInfo(title,artist,album,genre,year,track,favorite,ruleHit,fileName) "
            "VALUES('T','A','Al','G',2024,1,0,0,'migrated.mp3')");
        QVERIFY(legacy.execQuery());
        QCOMPARE(legacy.getCount(), 2);

        // 幂等：重复调用不改变已有数据
        legacy.initTable();
        QCOMPARE(legacy.getCount(), 2);
        legacy.closeDB();
    }
    QSqlDatabase::removeDatabase(legacyConn);
}

void TestMSTDataSource::searchMusic_escapesWildcards() {
    insert("100%","A","B","G",2024,1,"p1.mp3");
    insert("100x","A","B","G",2024,2,"p2.mp3");
    insert("a_c","A","B","G",2024,3,"p3.mp3");
    insert("abc","A","B","G",2024,4,"p4.mp3");

    // '%' 按字面匹配：只命中标题真的含 % 的那条，不会命中 "100x"
    const auto percent = ds->searchMusic("100%");
    QCOMPARE(percent.size(), 1);
    QCOMPARE(percent[0].getFileName(), QString("p1.mp3"));
    QCOMPARE(ds->getSearchCount("100%"), 1);

    // '_' 同理，不再匹配任意单字符
    const auto underscore = ds->searchMusic("a_c");
    QCOMPARE(underscore.size(), 1);
    QCOMPARE(underscore[0].getFileName(), QString("p3.mp3"));

    // 不带通配符时仍是子串匹配
    QCOMPARE(ds->searchMusic("100").size(), 2);
}

void TestMSTDataSource::getByFileNames_chunked() {
    insert("T0","A","B","G",2024,1,"f0.mp3");
    insert("T600","A","B","G",2024,2,"f600.mp3");
    insert("T1200","A","B","G",2024,3,"f1200.mp3");

    QStringList names;
    names.reserve(1201);
    for (int i = 0; i <= 1200; ++i) names << QString("f%1.mp3").arg(i);

    // 1201 个名字跨 3 个批次（每批 500），命中不得被批次边界截断或重复
    const auto r = ds->getByFileNames(names);
    QCOMPARE(r.size(), 3);
}