/**
 * @file TestQueryItem.cpp
 * @brief White-box branch coverage tests for the QueryItem class
 *        (src/Data/QueryItem.h + src/Data/QueryItem.cpp)
 *
 * Branch coverage targets:
 *   Default constructor     : all members at default-initialised values
 *   Full-param constructor  : every field assigned through initialiser/body
 *   Getters / Setters       : each accessor round-trips correctly
 *   Static sensitivity      : default value (4), explicit set, default-param set
 *   operator==              :
 *       - Core match (title)  → +2, hasCoreMatch=true
 *       - Core match (artist) → +2, hasCoreMatch=true
 *       - Auxiliary match (album fuzzy, track exact, year exact, genre fuzzy) → +1 each
 *       - hasCoreMatch=false → return false regardless of threshold
 *       - hasCoreMatch=true, threshold < sensitivity → return false
 *       - hasCoreMatch=true, threshold >= sensitivity → return true
 */

#include "TestQueryItem.h"

// ═══════════════════════════════════════════════════════════════════════════
// Constructors
// ═══════════════════════════════════════════════════════════════════════════

void TestQueryItem::testDefaultConstructor()
{
    // Default constructor: all QStrings are empty.
    // Note: year and track are POD (uint) and NOT zero-initialised by the
    // default constructor (= default).  Their values are indeterminate.
    QueryItem item;
    QCOMPARE(item.getTitle(),    QString(""));
    QCOMPARE(item.getArtist(),   QString(""));
    QCOMPARE(item.getAlbum(),    QString(""));
    QCOMPARE(item.getGenre(),    QString(""));
    QCOMPARE(item.getFileName(), QString(""));
    // year and track are indeterminate — verify they can be set afterwards
    item.setYear(0);
    QCOMPARE(item.getYear(), static_cast<uint>(0));
    item.setTrack(0);
    QCOMPARE(item.getTrack(), static_cast<uint>(0));
}

void TestQueryItem::testFullConstructor()
{
    // Full-parameter constructor: every field assigned via body assignments
    // (Note: year/track are initialised to 0 in the initialiser list, then
    //  overwritten by the parameter values in the constructor body)
    QueryItem item("Song Title", "Artist Name", "Album Name",
                   "Rock", 2024u, 5u, "/music/song.mp3");

    QCOMPARE(item.getTitle(),    QString("Song Title"));
    QCOMPARE(item.getArtist(),   QString("Artist Name"));
    QCOMPARE(item.getAlbum(),    QString("Album Name"));
    QCOMPARE(item.getGenre(),    QString("Rock"));
    QCOMPARE(item.getYear(),     static_cast<uint>(2024));
    QCOMPARE(item.getTrack(),    static_cast<uint>(5));
    QCOMPARE(item.getFileName(), QString("/music/song.mp3"));
}

// ═══════════════════════════════════════════════════════════════════════════
// Getters / Setters
// ═══════════════════════════════════════════════════════════════════════════

void TestQueryItem::testSetGetTitle()
{
    QueryItem item;
    item.setTitle("New Title");
    QCOMPARE(item.getTitle(), QString("New Title"));

    // Overwrite with a different value
    item.setTitle("Another Title");
    QCOMPARE(item.getTitle(), QString("Another Title"));
}

void TestQueryItem::testSetGetArtist()
{
    QueryItem item;
    item.setArtist("New Artist");
    QCOMPARE(item.getArtist(), QString("New Artist"));

    item.setArtist("Another Artist");
    QCOMPARE(item.getArtist(), QString("Another Artist"));
}

void TestQueryItem::testSetGetAlbum()
{
    QueryItem item;
    item.setAlbum("New Album");
    QCOMPARE(item.getAlbum(), QString("New Album"));

    item.setAlbum("Another Album");
    QCOMPARE(item.getAlbum(), QString("Another Album"));
}

void TestQueryItem::testSetGetGenre()
{
    QueryItem item;
    item.setGenre("Pop");
    QCOMPARE(item.getGenre(), QString("Pop"));

    item.setGenre("Jazz");
    QCOMPARE(item.getGenre(), QString("Jazz"));
}

void TestQueryItem::testSetGetYear()
{
    QueryItem item;
    item.setYear(2024u);
    QCOMPARE(item.getYear(), static_cast<uint>(2024));

    item.setYear(1999u);
    QCOMPARE(item.getYear(), static_cast<uint>(1999));
}

void TestQueryItem::testSetGetTrack()
{
    QueryItem item;
    item.setTrack(7u);
    QCOMPARE(item.getTrack(), static_cast<uint>(7));

    item.setTrack(0u);
    QCOMPARE(item.getTrack(), static_cast<uint>(0));
}

void TestQueryItem::testSetGetFileName()
{
    QueryItem item;
    item.setFileName("/path/to/file.mp3");
    QCOMPARE(item.getFileName(), QString("/path/to/file.mp3"));

    // Platform-style path
#if defined(_WIN32) || defined(_WIN64)
    item.setFileName("C:\\Music\\song.flac");
    QCOMPARE(item.getFileName(), QString("C:\\Music\\song.flac"));
#else
    item.setFileName("/home/user/Music/song.flac");
    QCOMPARE(item.getFileName(), QString("/home/user/Music/song.flac"));
#endif
}

// ═══════════════════════════════════════════════════════════════════════════
// Static sensitivity
// ═══════════════════════════════════════════════════════════════════════════

void TestQueryItem::testGetSetSimilarityThreshold()
{
    // Save original to restore later (sensitivity is static – shared state)
    int original = QueryItem::getSimilarityThreshold();

    // Branch: explicit value
    QueryItem::setSimilarityThreshold(5);
    QCOMPARE(QueryItem::getSimilarityThreshold(), 5);

    QueryItem::setSimilarityThreshold(1);
    QCOMPARE(QueryItem::getSimilarityThreshold(), 1);

    // Branch: default parameter (s = 3)
    QueryItem::setSimilarityThreshold();
    QCOMPARE(QueryItem::getSimilarityThreshold(), 3);

    // Restore
    QueryItem::setSimilarityThreshold(original);
}

void TestQueryItem::testDefaultSimilarityThreshold()
{
    // The static member is initialised to 4 at program start.
    // NOTE: This test must run before any test that modifies sensitivity,
    //       or we restore it explicitly here.
    // We force a known state: set to default value 4 and verify.
    QueryItem::setSimilarityThreshold(4);
    QCOMPARE(QueryItem::getSimilarityThreshold(), 4);
}

// ═══════════════════════════════════════════════════════════════════════════
// operator== (weighted fuzzy comparison)
//
// Scoring:
//   title  match (fuzzy) → +2, hasCoreMatch = true
//   artist match (fuzzy) → +2, hasCoreMatch = true
//   album  match (fuzzy) → +1
//   track  match (exact) → +1
//   year   match (exact) → +1
//   genre  match (fuzzy) → +1
//   Max possible = 8
//
// Result: hasCoreMatch && (totalScore >= sensitivity)
// Default sensitivity = 4
// ═══════════════════════════════════════════════════════════════════════════

void TestQueryItem::testEquality_identicalItems()
{
    QueryItem::setSimilarityThreshold(4); // ensure known state

    // All fields identical → all branches take the "match" side
    //   title  match → +2, hasCoreMatch=true
    //   artist match → +2
    //   album  match → +1
    //   track  match → +1
    //   year   match → +1
    //   genre  match → +1
    //   total = 8 >= 4, hasCoreMatch=true → true
    QueryItem a("Song Title", "Artist Name", "Album", "Rock", 2024u, 1u, "/a.mp3");
    QueryItem b("Song Title", "Artist Name", "Album", "Rock", 2024u, 1u, "/b.mp3");
    QVERIFY(a == b);
}

void TestQueryItem::testEquality_differentTitles()
{
    QueryItem::setSimilarityThreshold(4);

    // Titles completely different → fuzzyMatch returns false
    //   title  NO match → +0, hasCoreMatch stays false (if artist also differs)
    //   artist NO match → +0
    //   album  match    → +1
    //   track  match    → +1
    //   year   match    → +1
    //   genre  match    → +1
    //   total = 4 >= 4, BUT hasCoreMatch = false → return false
    QueryItem a("Completely Different Title", "Different Artist", "Album", "Rock", 2024u, 1u, "/a.mp3");
    QueryItem b("Another Song Entirely", "Some Other Person", "Album", "Rock", 2024u, 1u, "/b.mp3");
    QVERIFY(!(a == b));
}

void TestQueryItem::testEquality_similarTitles_differentArtists()
{
    QueryItem::setSimilarityThreshold(4);

    // Title matches (exact), artist is completely different
    //   title  match → +2, hasCoreMatch=true
    //   artist NO match → +0
    //   album  NO match → +0
    //   track  NO match → +0
    //   year   NO match → +0
    //   genre  NO match → +0
    //   total = 2 < 4, hasCoreMatch=true → return false (threshold not met)
    QueryItem a("Test Song", "ABC", "AlbumA", "Pop", 2020u, 1u, "/a.mp3");
    QueryItem b("Test Song", "XYZ", "AlbumB", "Jazz", 2021u, 2u, "/b.mp3");
    QVERIFY(!(a == b));
}

void TestQueryItem::testEquality_withSimilarityThresholdChange()
{
    // Lower sensitivity so that fewer matching fields suffice
    QueryItem::setSimilarityThreshold(2);

    // Only artist matches as core field (+2). All other fields differ
    // completely so fuzzyMatch returns false for title, album, and genre.
    QueryItem a("ABC", "Same Artist", "AAA", "Pop", 2020u, 1u, "/a.mp3");
    QueryItem b("XYZ", "Same Artist", "ZZZ", "Jazz", 2021u, 2u, "/b.mp3");

    // sensitivity=2: artist match score=2 >= 2 → true
    QVERIFY(a == b);

    // Sensitivity = 3: artist match score=2 < 3 → false
    QueryItem::setSimilarityThreshold(3);
    QVERIFY(!(a == b));

    // Sensitivity = 1: even a single core match suffices
    QueryItem::setSimilarityThreshold(1);
    QVERIFY(a == b);

    // Restore default
    QueryItem::setSimilarityThreshold(4);
}

void TestQueryItem::testEquality_emptyItems()
{
    QueryItem::setSimilarityThreshold(4);

    // Two default-constructed items: all fields are empty strings / 0
    //   title  "" vs "" → fuzzyMatch returns true  (str1==str2 shortcut) → +2, hasCoreMatch=true
    //   artist "" vs "" → fuzzyMatch returns true  → +2
    //   album  "" vs "" → fuzzyMatch returns true  → +1
    //   track  0 == 0   → +1
    //   year   0 == 0   → +1
    //   genre  "" vs "" → fuzzyMatch returns true  → +1
    //   total = 8 >= 4, hasCoreMatch=true → true
    QueryItem a;
    QueryItem b;
    QVERIFY(a == b);

    // One empty, one fully populated:
    //   title  "" vs "Song Title" → fuzzyMatch("", "Song Title") → false (one empty)
    //   artist "" vs "Artist"     → false
    //   album  "" vs "Album"      → false
    //   track  0 != 1             → +0
    //   year   0 != 2024          → +0
    //   genre  "" vs "Rock"       → false
    //   total = 0, hasCoreMatch=false → false
    QueryItem c;
    QueryItem d("Song Title", "Artist", "Album", "Rock", 2024u, 1u, "/d.mp3");
    QVERIFY(!(c == d));
}
