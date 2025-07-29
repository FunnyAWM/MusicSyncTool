#include "TestQueryItem.h"

void TestQueryItem::init()
{
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestQueryItem::cleanup()
{
    delete tempDir;
    tempDir = nullptr;
}

void TestQueryItem::testDefaultConstructor()
{
    QueryItem item;
    
    // 默认构造的对象应该有空的字符串值和0的数值
    QVERIFY(item.getTitle().isEmpty());
    QVERIFY(item.getArtist().isEmpty());
    QVERIFY(item.getAlbum().isEmpty());
    QVERIFY(item.getGenre().isEmpty());
    QVERIFY(item.getFileName().isEmpty());
    QCOMPARE(item.getYear(), 0u);
    QCOMPARE(item.getTrack(), 0u);
}

void TestQueryItem::testParameterConstructor()
{
    QString title = "Test Song";
    QString artist = "Test Artist";
    QString album = "Test Album";
    QString genre = "Test Genre";
    uint year = 2023;
    uint track = 5;
    QString fileName = "test.mp3";
    
    QueryItem item(title, artist, album, genre, year, track, fileName);
    
    QCOMPARE(item.getTitle(), title);
    QCOMPARE(item.getArtist(), artist);
    QCOMPARE(item.getAlbum(), album);
    QCOMPARE(item.getGenre(), genre);
    QCOMPARE(item.getYear(), year);
    QCOMPARE(item.getTrack(), track);
    QCOMPARE(item.getFileName(), fileName);
}

void TestQueryItem::testGetters()
{
    QueryItem item("Title", "Artist", "Album", "Genre", 2020, 3, "file.mp3");
    
    QCOMPARE(item.getTitle(), QString("Title"));
    QCOMPARE(item.getArtist(), QString("Artist"));
    QCOMPARE(item.getAlbum(), QString("Album"));
    QCOMPARE(item.getGenre(), QString("Genre"));
    QCOMPARE(item.getYear(), 2020u);
    QCOMPARE(item.getTrack(), 3u);
    QCOMPARE(item.getFileName(), QString("file.mp3"));
}

void TestQueryItem::testSetters()
{
    QueryItem item;
    
    // 测试设置字符串属性
    item.setTitle("New Title");
    QCOMPARE(item.getTitle(), QString("New Title"));
    
    item.setArtist("New Artist");
    QCOMPARE(item.getArtist(), QString("New Artist"));
    
    item.setAlbum("New Album");
    QCOMPARE(item.getAlbum(), QString("New Album"));
    
    item.setGenre("New Genre");
    QCOMPARE(item.getGenre(), QString("New Genre"));
    
    item.setFileName("new_file.mp3");
    QCOMPARE(item.getFileName(), QString("new_file.mp3"));
    
    // 测试设置数值属性
    item.setYear(2024);
    QCOMPARE(item.getYear(), 2024u);
    
    item.setTrack(7);
    QCOMPARE(item.getTrack(), 7u);
}

void TestQueryItem::testSensitivity()
{
    // 测试默认相似度阈值
    int defaultSensitivity = QueryItem::getSensitivity();
    QCOMPARE(defaultSensitivity, 3);
    
    // 测试设置新的相似度阈值
    QueryItem::setSensitivity(5);
    QCOMPARE(QueryItem::getSensitivity(), 5);
    
    // 测试重置为默认值
    QueryItem::setSensitivity();
    QCOMPARE(QueryItem::getSensitivity(), 3);
    
    // 测试边界值
    QueryItem::setSensitivity(0);
    QCOMPARE(QueryItem::getSensitivity(), 0);
    
    QueryItem::setSensitivity(10);
    QCOMPARE(QueryItem::getSensitivity(), 10);
    
    // 恢复默认值以免影响其他测试
    QueryItem::setSensitivity(3);
}

void TestQueryItem::testEqualityOperator()
{
    QueryItem item1("Title", "Artist", "Album", "Genre", 2020, 1, "file1.mp3");
    QueryItem item2("Title", "Artist", "Album", "Genre", 2020, 1, "file1.mp3");
    QueryItem item3("Different", "Artist", "Album", "Genre", 2020, 1, "file1.mp3");
    
    // 相同内容的对象应该相等
    QVERIFY(item1 == item2);
    
    // 不同内容的对象应该不相等
    QVERIFY(!(item1 == item3));
    QVERIFY(item1 != item3);
}

void TestQueryItem::testSimilarityComparison()
{
    QueryItem item1("Test Song", "Test Artist", "Test Album", "Pop", 2020, 1, "test1.mp3");
    QueryItem item2("Test Song", "Test Artist", "Test Album", "Rock", 2021, 2, "test2.mp3");
    QueryItem item3("Different Song", "Different Artist", "Different Album", "Jazz", 2022, 3, "test3.mp3");
    
    // 设置相似度阈值
    QueryItem::setSensitivity(3);
    
    // 相似的歌曲（标题、艺术家、专辑相同）
    QVERIFY(item1.isSimilar(item2));
    
    // 完全不同的歌曲
    QVERIFY(!item1.isSimilar(item3));
    QVERIFY(!item2.isSimilar(item3));
    
    // 测试与自身的相似性
    QVERIFY(item1.isSimilar(item1));
}

void TestQueryItem::testEdgeCases()
{
    // 测试空字符串
    QueryItem emptyItem("", "", "", "", 0, 0, "");
    QVERIFY(emptyItem.getTitle().isEmpty());
    QVERIFY(emptyItem.getArtist().isEmpty());
    QVERIFY(emptyItem.getAlbum().isEmpty());
    QVERIFY(emptyItem.getGenre().isEmpty());
    QVERIFY(emptyItem.getFileName().isEmpty());
    
    // 测试空字符串的相似性比较
    QueryItem anotherEmpty("", "", "", "", 0, 0, "");
    QVERIFY(emptyItem.isSimilar(anotherEmpty));
    
    // 测试一个空一个非空的情况
    QueryItem nonEmpty("Title", "Artist", "Album", "Genre", 2020, 1, "file.mp3");
    QVERIFY(!emptyItem.isSimilar(nonEmpty));
    
    // 测试特殊字符
    QueryItem specialChars("Title!@#$%", "Artist&*()", "Album[]{}|", "Genre;:\"'", 2020, 1, "file?.mp3");
    QVERIFY(specialChars.getTitle().contains("!@#$%"));
    QVERIFY(specialChars.getArtist().contains("&*()"));
}

void TestQueryItem::testChineseCharacters()
{
    // 测试中文字符
    QueryItem chineseItem("测试歌曲", "测试歌手", "测试专辑", "流行", 2020, 1, "test.mp3");
    
    QCOMPARE(chineseItem.getTitle(), QString("测试歌曲"));
    QCOMPARE(chineseItem.getArtist(), QString("测试歌手"));
    QCOMPARE(chineseItem.getAlbum(), QString("测试专辑"));
    QCOMPARE(chineseItem.getGenre(), QString("流行"));
    
    // 测试中文的相似性比较
    QueryItem anotherChinese("测试歌曲", "测试歌手", "测试专辑", "摇滚", 2021, 2, "test2.mp3");
    QVERIFY(chineseItem.isSimilar(anotherChinese));
    
    // 测试中英混合
    QueryItem mixedItem("Test测试", "Artist歌手", "Album专辑", "Pop流行", 2020, 1, "mixed.mp3");
    QVERIFY(mixedItem.getTitle().contains("Test"));
    QVERIFY(mixedItem.getTitle().contains("测试"));
}

void TestQueryItem::testNumericBoundaries()
{
    QueryItem item;
    
    // 测试年份边界值
    item.setYear(0);
    QCOMPARE(item.getYear(), 0u);
    
    item.setYear(UINT_MAX);
    QCOMPARE(item.getYear(), UINT_MAX);
    
    item.setYear(2024);
    QCOMPARE(item.getYear(), 2024u);
    
    // 测试音轨编号边界值
    item.setTrack(0);
    QCOMPARE(item.getTrack(), 0u);
    
    item.setTrack(UINT_MAX);
    QCOMPARE(item.getTrack(), UINT_MAX);
    
    item.setTrack(99);
    QCOMPARE(item.getTrack(), 99u);
    
    // 测试年份为0的合理性（可能表示未知年份）
    QueryItem unknownYear("Title", "Artist", "Album", "Genre", 0, 1, "file.mp3");
    QCOMPARE(unknownYear.getYear(), 0u);
}

#include "TestQueryItem.moc"
