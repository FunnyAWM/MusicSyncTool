#include "TestQueryItem.h"

void TestQueryItem::defaultConstructor() {
    QueryItem item;
    QCOMPARE(item.getTitle(), QString(""));
    QCOMPARE(item.getArtist(), QString(""));
    QCOMPARE(item.getAlbum(), QString(""));
    QCOMPARE(item.getGenre(), QString(""));
    QCOMPARE(item.getFileName(), QString(""));
}

void TestQueryItem::fullConstructor() {
    QueryItem item("Title", "Artist", "Album", "Rock", 2024u, 5u, "/m/s.mp3");
    QCOMPARE(item.getTitle(), QString("Title"));
    QCOMPARE(item.getArtist(), QString("Artist"));
    QCOMPARE(item.getAlbum(), QString("Album"));
    QCOMPARE(item.getGenre(), QString("Rock"));
    QCOMPARE(item.getYear(), 2024u);
    QCOMPARE(item.getTrack(), 5u);
    QCOMPARE(item.getFileName(), QString("/m/s.mp3"));
}

void TestQueryItem::settersAndGetters() {
    QueryItem item;
    item.setTitle("T"); item.setArtist("A"); item.setAlbum("B");
    item.setGenre("G"); item.setYear(2000u); item.setTrack(3u);
    item.setFileName("f.mp3");
    QCOMPARE(item.getTitle(), QString("T"));
    QCOMPARE(item.getArtist(), QString("A"));
    QCOMPARE(item.getAlbum(), QString("B"));
    QCOMPARE(item.getGenre(), QString("G"));
    QCOMPARE(item.getYear(), 2000u);
    QCOMPARE(item.getTrack(), 3u);
    QCOMPARE(item.getFileName(), QString("f.mp3"));
}

void TestQueryItem::similarityThreshold_default() {
    QueryItem::setSimilarityThreshold(5);
    QCOMPARE(QueryItem::getSimilarityThreshold(), 5);
}

void TestQueryItem::similarityThreshold_set() {
    int orig = QueryItem::getSimilarityThreshold();
    QueryItem::setSimilarityThreshold(2);
    QCOMPARE(QueryItem::getSimilarityThreshold(), 2);
    QueryItem::setSimilarityThreshold();
    QCOMPARE(QueryItem::getSimilarityThreshold(), 5);
    QueryItem::setSimilarityThreshold(orig);
}

void TestQueryItem::equality_identical() {
    QueryItem::setSimilarityThreshold(5);
    QueryItem a("T", "A", "B", "G", 2024u, 1u, "a.mp3");
    QueryItem b("T", "A", "B", "G", 2024u, 1u, "b.mp3");
    QVERIFY(a == b);
}

void TestQueryItem::equality_different() {
    QueryItem::setSimilarityThreshold(5);
    QueryItem a("ABC", "X", "B", "G", 2024u, 1u, "a.mp3");
    QueryItem b("XYZ", "Y", "B", "G", 2024u, 1u, "b.mp3");
    QVERIFY(!(a == b));
}

void TestQueryItem::equality_thresholdBoundary() {
    QueryItem a("Same", "Same Artist", "AlbA", "Pop", 2020u, 1u, "a.mp3");
    QueryItem b("Diff", "Same Artist", "AlbB", "Jazz", 2021u, 2u, "b.mp3");
    QueryItem::setSimilarityThreshold(2);
    QVERIFY(a == b);
    QueryItem::setSimilarityThreshold(3);
    QVERIFY(!(a == b));
    QueryItem::setSimilarityThreshold(4);
}

void TestQueryItem::equality_noCoreMatch() {
    QueryItem::setSimilarityThreshold(5);
    QueryItem a("ABC", "X", "Same", "G", 2024u, 1u, "a.mp3");
    QueryItem b("XYZ", "Y", "Same", "G", 2024u, 1u, "b.mp3");
    QVERIFY(!(a == b));
}

void TestQueryItem::equality_empty() {
    QueryItem::setSimilarityThreshold(5);
    QueryItem a, b;
    QVERIFY(!(a == b));
    QueryItem c("T", "A", "B", "G", 2024u, 1u, "c.mp3");
    QVERIFY(!(a == c));
}